/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
 * - use g_io_channel_get_flags() (instead of eof?) */



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
	char * title;
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
	int eof;			/* end of file 		*/
	int fds[2];			/* for the pipe		*/
	pid_t pid;			/* child's pid		*/
	size_t cnt;			/* bytes written	*/
	char buf[65536];
	size_t buf_cnt;
	GIOChannel * in_channel;
	guint in_id;
	GIOChannel * out_channel;
	guint out_id;

	/* widgets */
	GtkWidget * speed;
	GtkWidget * progress;
	int pulse;			/* tells when to pulse	*/
} Progress;

/* functions */
static int _progress_error(char const * message, int ret);
static int _progress_gerror(char const * message, GError * error, int ret);
static int _progress_exec(Progress * progress, char * argv[]);

/* callbacks */
static gboolean _progress_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _progress_cancel(GtkWidget * widget, gpointer data);
static gboolean _progress_channel(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _progress_idle_in(gpointer data);
static gboolean _progress_idle_out(gpointer data);
static gboolean _progress_timeout(gpointer data);

static int _progress(Prefs * prefs, char * argv[])
{
	Progress p;
	struct stat st;
	GtkWidget * window = NULL;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	GtkWidget * widget;
	PangoFontDescription * bold;
	char * q;
  
	memset(&p, 0, sizeof(p));
	p.prefs = prefs;
	if(pipe(p.fds) != 0)
		return _progress_error("pipe", 1);
	if((p.pid = fork()) == -1)
	{
		close(p.fds[0]);
		close(p.fds[1]);
		return _progress_error("fork", 1);
	}
	if(p.pid != 0)
		return _progress_exec(&p, argv);
	close(p.fds[0]);
	if(gettimeofday(&p.tv, NULL) != 0)
		return _progress_error("gettimeofday", 1);
	if(prefs->filename == NULL)
		prefs->filename = "Standard input";
	else if((p.fd = open(prefs->filename, O_RDONLY)) < 0)
		return _progress_error(prefs->filename, 1);
	else if(fstat(p.fd, &st) == 0)
		prefs->length = st.st_size;
	p.in_channel = g_io_channel_unix_new(p.fd);
	g_io_channel_set_encoding(p.in_channel, NULL, NULL);
	p.in_id = 0;
	g_idle_add(_progress_idle_in, &p);
	p.out_channel = g_io_channel_unix_new(p.fds[1]);
	g_io_channel_set_encoding(p.out_channel, NULL, NULL);
	p.out_id = 0;
	/* graphical interface */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), prefs->title != NULL
			? prefs->title : "Progress");
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
	if((q = g_filename_to_utf8(prefs->filename, -1, NULL, NULL, NULL))
			== NULL)
		q = prefs->filename;
	widget = gtk_label_new(q);
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

static int _error_do(char const * message, char const * error, int ret);

static int _progress_error(char const * message, int ret)
{
	return _error_do(message, strerror(errno), ret);
}

static int _progress_gerror(char const * message, GError * error, int ret)
{
	return _error_do(message, error->message, ret);
}

static int _error_do(char const * message, char const * error, int ret)
{
	GtkWidget * dialog;

	if(ret < 0)
	{
		fputs("progress: ", stderr);
		perror(message);
		return -ret;
	}
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s: %s",
			message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static gboolean _progress_idle_in(gpointer data)
{
	Progress * p = data;

	if(p->in_id == 0)
		p->in_id = g_io_add_watch(p->in_channel, G_IO_IN,
				_progress_channel, p);
	return FALSE;
}

static gboolean _progress_idle_out(gpointer data)
{
	Progress * p = data;

	if(p->out_id == 0)
		p->out_id = g_io_add_watch(p->out_channel, G_IO_OUT,
				_progress_channel, p);
	return FALSE;
}


/* progress_exec */
static int _exec_gunzip(Progress * progress, char * argv[]);

static int _progress_exec(Progress * progress, char * argv[])
{
	close(progress->fds[1]);
	if(dup2(progress->fds[0], 0) == -1)
	{
		perror("dup2");
		exit(1); /* FIXME warn user */
	}
	if(progress->prefs->flags & PREFS_z)
		_exec_gunzip(progress, argv);
	else
		execvp(argv[0], argv);
	exit(_progress_error(argv[0], -1));
	return 1;
}

static int _exec_gunzip(Progress * progress, char * argv[])
{
	static Progress tmp;

	tmp.prefs = progress->prefs;
	tmp.prefs->flags -= PREFS_z;
	if(pipe(tmp.fds) != 0)
		exit(_progress_error("pipe", -1));
	if((tmp.pid = fork()) == -1)
		exit(_progress_error("fork", -1));
	if(tmp.pid == 0)
		return _progress_exec(&tmp, argv);
	close(tmp.fds[0]);
	if(dup2(tmp.fds[1], 1) == -1)
		exit(_progress_error("dup2", -1));
	execlp("gunzip", "gunzip", NULL);
	execlp("gzip", "gzip", "-d", NULL);
	exit(_progress_error("gunzip", -1));
	return 1;
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
static gboolean _channel_in(Progress * p, GIOChannel * source);
static gboolean _channel_out(Progress * p, GIOChannel * source);
static void _out_rate(Progress * p);

static gboolean _progress_channel(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Progress * p = data;

	if(condition == G_IO_IN)
		return _channel_in(p, source);
	if(condition == G_IO_OUT)
		return _channel_out(p, source);
	_progress_error(p->prefs->filename, 0);
	gtk_main_quit();
	return FALSE;
}

static gboolean _channel_in(Progress * p, GIOChannel * source)
{
	gsize read;

	p->in_id = 0;
	/* I would use g_io_channel_read_chars but it doesn't work */
	if(g_io_channel_read(source, &p->buf[p->buf_cnt],
				sizeof(p->buf) - p->buf_cnt, &read)
			!= G_IO_ERROR_NONE)
	{
		_progress_error(p->prefs->filename, 0);
		g_io_channel_unref(source);
		gtk_main_quit();
		return FALSE;
	}
	if(read == 0)
	{
		p->eof = 1; /* reached end of input file */
		return FALSE;
	}
	if(p->buf_cnt == 0)
		g_idle_add(_progress_idle_out, p); /* begin to write */
	p->buf_cnt += read;
	if(p->buf_cnt != sizeof(p->buf))
		_progress_idle_in(p); /* continue to read */
	return FALSE;
}

static gboolean _channel_out(Progress * p, GIOChannel * source)
{
	gsize written;
	GError * error = NULL;

	p->out_id = 0;
	/* write data */
	if(g_io_channel_write_chars(source, p->buf, p->buf_cnt, &written,
				&error) == G_IO_STATUS_ERROR)
	{
		_progress_gerror(p->prefs->filename, error, 0);
		gtk_main_quit();
		return FALSE;
	}
	g_idle_add(_progress_idle_in, p);
	p->buf_cnt -= written;
	memmove(p->buf, &p->buf[written], p->buf_cnt);
	p->cnt += written;
	_out_rate(p);
	/* continue to write */
	if(p->buf_cnt > 0)
		g_idle_add(_progress_idle_out, p);
	else if(p->eof == 1) /* reached end of output */
		gtk_main_quit();
	return FALSE;
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
	char const * unit = "kB";

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
		tv.tv_usec += 1000000;
	}
	rate = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	if((rate = progress->cnt / rate) > 1024)
	{
		rate /= 1024;
		unit = "MB";
	}
	snprintf(buf, sizeof(buf), "%.1f %s/s", rate, unit);
	gtk_label_set_text(GTK_LABEL(progress->speed), buf);
	return TRUE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: progress [-z][-f file][-l length][-p prefix][-t title]"
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
	while((o = getopt(argc, argv, "f:l:t:z")) != -1)
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
			case 't':
				prefs.title = optarg;
				break;
			case 'z':
				prefs.flags |= PREFS_z;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	return _progress(&prefs, &argv[optind]) == 0 ? 0 : 2;
}
