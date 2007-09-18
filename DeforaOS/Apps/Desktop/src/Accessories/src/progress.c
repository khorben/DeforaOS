/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Accessories */
/* Accessories is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation.
 *
 * Accessories is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Accessories; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */
/* TODO:
 * - implement -z */



#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>


/* types */
typedef struct _Prefs
{
	int flags;
	char * filename;
	size_t length;
} Prefs;
#define PREFS_z 0x1


/* progress */
/* types */
typedef struct _Progress
{
	Prefs * prefs;			/* preferences		*/

	struct timeval tv;		/* start time		*/
	int fd;				/* read descriptor	*/
	int fds[2];			/* for the pipe		*/
	pid_t pid;			/* child's pid		*/
	size_t cnt;			/* bytes written	*/
	char buf[BUFSIZ];
	size_t buf_cnt;

	/* widgets */
	GtkWidget * speed;
	GtkWidget * progress;
	int pulse;			/* tells when to pulse	*/
} Progress;

/* functions */
static int _progress_error(char const * message, int ret);
static int _progress_exec(Progress * progress, char * argv[]);

/* callbacks */
static gboolean _progress_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _progress_cancel(GtkWidget * widget, gpointer data);
static gboolean _progress_out(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _progress_timeout(gpointer data);

static int _progress(Prefs * prefs, char * argv[])
{
	Progress p;
	struct stat st;
	GIOChannel * channel;
	GtkWidget * window = NULL;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	GtkWidget * widget;
	PangoFontDescription * bold;
  
	p.prefs = prefs;
	p.fd = 0;
	p.cnt = 0;
	if(gettimeofday(&p.tv, NULL) != 0)
		return _progress_error("gettimeofday", 1);
	if(prefs->filename == NULL)
		prefs->filename = "Standard input";
	else if((p.fd = open(prefs->filename, O_RDONLY)) < 0)
		return _progress_error(prefs->filename, 1);
	else if(fstat(p.fd, &st) == 0)
		prefs->length = st.st_size;
	if(pipe(p.fds) != 0)
	{
		close(p.fd);
		return _progress_error("pipe", 1);
	}
	channel = g_io_channel_unix_new(p.fds[1]);
	g_io_add_watch(channel, G_IO_OUT, _progress_out, &p);
	if((p.pid = fork()) == -1)
	{
		close(p.fd);
		close(p.fds[0]);
		close(p.fds[1]);
		return _progress_error("fork", 1);
	}
	if(p.pid != 0)
		return _progress_exec(&p, argv);
	close(p.fds[0]);
	/* graphical interface */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Progress");
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_progress_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	left = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	right = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_label_new("File: ");
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(prefs->filename);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(right, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new("Speed: ");
	gtk_widget_modify_font(widget, bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	p.speed = gtk_label_new("0.0 kB/s");
	g_timeout_add(250, _progress_timeout, &p);
	gtk_misc_set_alignment(GTK_MISC(p.speed), 0, 0);
	gtk_size_group_add_widget(right, p.speed);
	gtk_box_pack_start(GTK_BOX(hbox), p.speed, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	p.progress = gtk_progress_bar_new();
	p.pulse = 0;
	gtk_box_pack_start(GTK_BOX(vbox), p.progress, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_progress_cancel), NULL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_widget_show_all(window);
	gtk_main();
	close(p.fd);
	close(p.fds[1]);
	return 0;
}

static int _progress_error(char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s",
			message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	return ret;
}


static int _progress_exec(Progress * progress, char * argv[])
{
	close(progress->fds[1]);
	if(dup2(progress->fds[0], 0) == -1)
		exit(1); /* FIXME warn user */
	execvp(argv[0], argv);
	/* FIXME warn user */
	exit(0);
}


/* callbacks */
static gboolean _progress_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
	return FALSE;
}

static void _progress_cancel(GtkWidget * widget, gpointer data)
{
	gtk_main_quit();
}


/* progress_out */
static void _out_rate(Progress * p);

static gboolean _progress_out(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Progress * p = data;
	ssize_t len;
	gsize written;

	/* FIXME use g_io_channel_read too? */
	if(condition != G_IO_OUT
			|| (len = read(p->fd, p->buf, sizeof(p->buf))) < 0)
	{
		gtk_main_quit();
		_progress_error(p->prefs->filename, 0);
		return FALSE;
	}
	if(g_io_channel_write(source, p->buf, len, &written) != G_IO_ERROR_NONE
			|| written != (gsize)len)
	{
		/* FIXME it may just be that everything was not written
		 * => put buffer and position in Prefs/Progress */
		gtk_main_quit();
		_progress_error(p->prefs->filename, 0);
		return FALSE;
	}
	p->cnt += len;
	_out_rate(p);
	if(len == 0)
	{
		gtk_main_quit();
		return FALSE;
	}
	return TRUE;
}

static void _out_rate(Progress * p)
{
	gdouble fraction;
	char buf[16];

	if(p->prefs->length == 0 || p->cnt == 0)
	{
		p->pulse = 1;
		return;
	}
	fraction = p->cnt;
	fraction /= p->prefs->length;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(p->progress),
			fraction);
	snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(p->progress), buf);
}


/* progress_timeout */
static gboolean _progress_timeout(gpointer data)
{
	Progress * progress = data;
	struct timeval tv;
	double rate;
	char buf[16];

	if(progress->pulse == 1)
	{
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress->progress));
		progress->pulse = 0;
	}
	if(progress->cnt == 0)
	{
		gtk_label_set_text(GTK_LABEL(progress->speed), "0.0 kB/s");
		return TRUE;
	}
	if(gettimeofday(&tv, NULL) != 0)
		return _progress_error("gettimeofday", FALSE);
	if((tv.tv_sec = tv.tv_sec - progress->tv.tv_sec) < 0)
		tv.tv_sec = 0;
	if((tv.tv_usec = tv.tv_usec - progress->tv.tv_usec) < 0)
	{
		tv.tv_sec--;
		tv.tv_usec = progress->tv.tv_usec - tv.tv_usec;
	}
	rate = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	rate = progress->cnt / rate;
	snprintf(buf, sizeof(buf), "%.1f kB/s", rate);
	gtk_label_set_text(GTK_LABEL(progress->speed), buf);
	return TRUE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: progress [-z][-f file][-l length][-p prefix]"
			" cmd [args...]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "f:l:z")) != -1)
		switch(o)
		{
			case 'f':
				prefs.filename = optarg;
				break;
			case 'l':
				prefs.length = strtol(optarg, &p, 0);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'z':
				prefs.flags |= PREFS_z; /* FIXME implement */
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	return _progress(&prefs, &argv[optind]) == 0 ? 0 : 2;
}
