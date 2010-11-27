/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "../config.h"
#define _(string) gettext(string)


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* Copy */
/* types */
typedef int Prefs;
#define PREFS_f 0x01
#define PREFS_i 0x02
#define PREFS_p 0x04
#define PREFS_R 0x08
#define PREFS_H 0x10
#define PREFS_L 0x20
#define PREFS_P 0x40

typedef struct _Copy
{
	Prefs * prefs;
	unsigned int filec;
	char ** filev;
	unsigned int cur;

	struct timeval tv;
	size_t size;
	size_t cnt;
	char buf[65536];
	size_t buf_cnt;
	int eof;
	GIOChannel * in_channel;
	guint in_id;
	GIOChannel * out_channel;
	guint out_id;

	/* widgets */
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
	GtkWidget * flabel;
	GtkWidget * fspeed;
	GtkWidget * fprogress;
	int fpulse;			/* tells when to pulse */
} Copy;

/* functions */
static void _copy_refresh(Copy * copy);

/* callbacks */
static void _copy_on_closex(void);
static gboolean _copy_idle_first(gpointer data);

static int _copy(Prefs * prefs, unsigned int filec, char * filev[])
{
	static Copy copy;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * left;
	GtkSizeGroup * right;
	GtkWidget * widget;
	PangoFontDescription * bold;

	if(filec < 2 || filev == NULL)
		return 1; /* FIXME report error */
	copy.prefs = prefs;
	copy.filec = filec;
	copy.filev = filev;
	copy.cur = 0;
	/* graphical interface */
	copy.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(copy.window), _("Copy file(s)"));
	gtk_window_set_resizable(GTK_WINDOW(copy.window), FALSE);
	g_signal_connect(G_OBJECT(copy.window), "delete-event", G_CALLBACK(
			_copy_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	left = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	right = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* current argument */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Copying: "));
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	copy.label = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(copy.label), 0, 0);
	gtk_size_group_add_widget(right, copy.label);
	gtk_box_pack_start(GTK_BOX(hbox), copy.label, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	/* progress bar */
	copy.progress = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(copy.progress), " ");
	gtk_box_pack_start(GTK_BOX(vbox), copy.progress, TRUE, TRUE, 4);
	/* file copy */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Filename: "));
	gtk_widget_modify_font(widget, bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	copy.flabel = gtk_label_new("");
	gtk_label_set_ellipsize(GTK_LABEL(copy.flabel), PANGO_ELLIPSIZE_START);
	gtk_label_set_width_chars(GTK_LABEL(copy.flabel), 25);
	gtk_misc_set_alignment(GTK_MISC(copy.flabel), 0, 0);
	gtk_size_group_add_widget(right, copy.flabel);
	gtk_box_pack_start(GTK_BOX(hbox), copy.flabel, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	/* file copy speed */
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_label_new(_("Speed: "));
	gtk_widget_modify_font(widget, bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0, 0);
	gtk_size_group_add_widget(left, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	copy.fspeed = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(copy.fspeed), 0, 0);
	gtk_size_group_add_widget(right, copy.fspeed);
	gtk_box_pack_start(GTK_BOX(hbox), copy.fspeed, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	/* file progress bar */
	copy.fprogress = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(copy.fprogress), " ");
	gtk_box_pack_start(GTK_BOX(vbox), copy.fprogress, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(copy.window), 4);
	gtk_container_add(GTK_CONTAINER(copy.window), vbox);
	g_idle_add(_copy_idle_first, &copy);
	_copy_refresh(&copy);
	gtk_widget_show_all(copy.window);
	pango_font_description_free(bold);
	return 0;
}

static void _copy_refresh(Copy * copy)
{
	char buf[32];
	double fraction;

	/* FIXME convert to UTF-8 */
	gtk_label_set_text(GTK_LABEL(copy->label), copy->filev[copy->cur]);
	snprintf(buf, sizeof(buf), _("File %u of %u"), copy->cur + 1,
			copy->filec - 1);
	fraction = copy->cur;
	fraction /= copy->filec - 1;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(copy->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(copy->progress),
			fraction);
}

static int _copy_error(Copy * copy, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(copy->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static void _copy_info(Copy * copy, char const * message, char const * info)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(copy->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
			GTK_BUTTONS_OK, "%s: %s", message, info);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Information"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static int _copy_confirm(Copy * copy, char const * dst)
{
	GtkWidget * dialog;
	int res;

	dialog = gtk_message_dialog_new(GTK_WINDOW(copy->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			_("%s will be overwritten\nProceed?"), dst);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (res == GTK_RESPONSE_YES) ? 1 : 0;
}

static void _copy_on_closex(void)
{
	gtk_main_quit();
}

static int _copy_single(Copy * copy, char const * src, char const * dst);
static gboolean _copy_idle_multiple(gpointer data);

static gboolean _copy_idle_first(gpointer data)
{
	Copy * copy = data;
	struct stat st;

	if(stat(copy->filev[copy->filec - 1], &st) != 0)
	{
		if(errno != ENOENT)
			_copy_error(copy, copy->filev[copy->filec - 1], 0);
		else if(copy->filec > 2)
		{
			errno = ENOTDIR;
			_copy_error(copy, copy->filev[copy->filec - 1], 0);
		}
		else
			_copy_single(copy, copy->filev[0], copy->filev[1]);
	}
	else if(S_ISDIR(st.st_mode))
	{
		g_idle_add(_copy_idle_multiple, copy);
		return FALSE;
	}
	else if(copy->filec > 2)
	{
		errno = ENOTDIR;
		_copy_error(copy, copy->filev[copy->filec - 1], 0);
	}
	else
		_copy_single(copy, copy->filev[0], copy->filev[1]);
	gtk_main_quit();
	return FALSE;
}

/* copy_single
 * XXX TOCTOU all over the place (*stat) but seem impossible to avoid */
static int _single_dir(Copy * copy, char const * src, char const * dst);
static int _single_fifo(Copy * copy, char const * dst);
static int _single_symlink(Copy * copy, char const * src, char const * dst);
static int _single_regular(Copy * copy, char const * src, char const * dst);
static int _single_p(Copy * copy, char const * dst, struct stat const * st);
static gboolean _single_timeout(gpointer data);

static int _copy_single(Copy * copy, char const * src, char const * dst)
{
	int ret;
	struct stat st;
	struct stat st2;
	guint timeout;

	gtk_label_set_text(GTK_LABEL(copy->flabel), src);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(copy->fprogress), 0.0);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(copy->fprogress), " ");
	if(*(copy->prefs) & PREFS_P) /* don't follow symlinks */
	{
		if(lstat(src, &st) != 0 && errno == ENOENT)
			return _copy_error(copy, src, 1);
	}
	else if(stat(src, &st) != 0 && errno == ENOENT) /* follow symlinks */
		return _copy_error(copy, src, 1);
	if(lstat(dst, &st2) == 0)
	{
		if(*(copy->prefs) & PREFS_i && _copy_confirm(copy, dst) != 1)
			return 0;
		if(unlink(dst) != 0)
			return _copy_error(copy, dst, 1);
	}
	if(S_ISDIR(st.st_mode))
		ret = _single_dir(copy, src, dst);
	else if(S_ISFIFO(st.st_mode))
		ret = _single_fifo(copy, dst);
	else if(S_ISLNK(st.st_mode))
		ret = _single_symlink(copy, src, dst);
	else
	{
		ret = _single_regular(copy, src, dst);
		timeout = g_timeout_add(250, _single_timeout, copy);
		gtk_main(); /* XXX ugly */
		g_source_remove(timeout);
	}
	if(ret != 0)
		return ret;
	if(*(copy->prefs) & PREFS_p) /* XXX TOCTOU */
		_single_p(copy, dst, &st);
	return 0;
}

/* single_dir */
static int _single_recurse(Copy * copy, char const * src, char const * dst);

static int _single_dir(Copy * copy, char const * src, char const * dst)
{
	if(*(copy->prefs) & PREFS_R)
		return _single_recurse(copy, src, dst);
	_copy_info(copy, src, _("Omitting directory"));
	return 0;
}

static int _single_recurse(Copy * copy, char const * src, char const * dst)
{
	int ret = 0;
	Copy copy2;
	Prefs prefs2 = *(copy->prefs);
	size_t srclen;
	size_t dstlen;
	DIR * dir;
	struct dirent * de;
	char * ssrc = NULL;
	char * sdst = NULL;
	char * p;

	memcpy(&copy2, copy, sizeof(copy2));
	copy2.prefs = &prefs2;
	if(mkdir(dst, 0777) != 0 && errno != EEXIST)
		return _copy_error(copy, dst, 1);
	srclen = strlen(src);
	dstlen = strlen(dst);
	if((dir = opendir(src)) == NULL)
		return _copy_error(copy, src, 1);
	prefs2 |= (prefs2 & PREFS_H) ? PREFS_P : 0;
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(ssrc, srclen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _copy_error(copy, src, 1);
			continue;
		}
		ssrc = p;
		if((p = realloc(sdst, dstlen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _copy_error(copy, src, 1);
			continue;
		}
		sdst = p;
		sprintf(ssrc, "%s/%s", src, de->d_name);
		sprintf(sdst, "%s/%s", dst, de->d_name);
		ret |= _copy_single(&copy2, ssrc, sdst);
	}
	closedir(dir);
	free(ssrc);
	free(sdst);
	return ret;
}

static int _single_fifo(Copy * copy, char const * dst)
{
	if(mkfifo(dst, 0666) != 0)
		return _copy_error(copy, dst, 1);
	return 0;
}

static int _single_symlink(Copy * copy, char const * src, char const * dst)
{
	char buf[PATH_MAX];
	ssize_t len;

	if((len = readlink(src, buf, sizeof(buf) - 1)) == -1)
		return _copy_error(copy, src, 1);
	buf[len] = '\0';
	if(symlink(buf, dst) != 0)
		return _copy_error(copy, dst, 1);
	return 0;
}

static gboolean _regular_idle_in(gpointer data);
static gboolean _regular_idle_out(gpointer data);

static int _single_regular(Copy * copy, char const * src, char const * dst)
{
	int ret = 0;
	int in_fd;
	int out_fd;
	struct stat st;

	if(gettimeofday(&copy->tv, NULL) != 0)
		return _copy_error(copy, "gettimeofday", 1);
	if((in_fd = open(src, O_RDONLY)) < 0)
		return _copy_error(copy, src, 1);
	if((out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		ret = _copy_error(copy, src, 1);
		close(in_fd);
		return ret;
	}
	copy->size = 0;
	if(fstat(in_fd, &st) == 0)
		copy->size = st.st_size;
	copy->cnt = 0;
	copy->buf_cnt = 0;
	copy->eof = 0;
	copy->in_channel = g_io_channel_unix_new(in_fd);
	g_io_channel_set_encoding(copy->in_channel, NULL, NULL);
	copy->in_id = 0;
	g_idle_add(_regular_idle_in, copy);
	copy->out_channel = g_io_channel_unix_new(out_fd);
	g_io_channel_set_encoding(copy->out_channel, NULL, NULL);
	copy->out_id = 0;
	return ret;
}

static gboolean _regular_channel(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _channel_in(Copy * copy, GIOChannel * source);
static gboolean _channel_out(Copy * copy, GIOChannel * source);
static void _out_rate(Copy * copy);

static gboolean _regular_idle_in(gpointer data)
{
	Copy * copy = data;

	if(copy->in_id == 0)
		copy->in_id = g_io_add_watch(copy->in_channel, G_IO_IN,
				_regular_channel, copy);
	return FALSE;
}

static gboolean _regular_channel(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Copy * copy = data;

	if(condition == G_IO_IN)
		return _channel_in(copy, source);
	else if(condition == G_IO_OUT)
		return _channel_out(copy, source);
	_copy_error(copy, copy->filev[copy->cur], 0);
	gtk_main_quit();
	return FALSE;
}

static gboolean _channel_in(Copy * copy, GIOChannel * source)
{
	GIOStatus status;
	gsize read;
	GError * error = NULL;

	copy->in_id = 0;
	status = g_io_channel_read_chars(source, &copy->buf[copy->buf_cnt],
			sizeof(copy->buf) - copy->buf_cnt, &read, &error);
	if(status == G_IO_STATUS_ERROR)
	{
		_copy_error(copy, copy->filev[copy->cur], 0);
		g_io_channel_unref(source);
		gtk_main_quit(); /* XXX ugly */
		return FALSE;
	}
	else if(status == G_IO_STATUS_EOF)
	{
		copy->eof = 1; /* reached end of input file */
		g_io_channel_close(source);
	}
	else if(copy->buf_cnt + read != sizeof(copy->buf))
		g_idle_add(_regular_idle_in, copy); /* continue to read */
	if(copy->buf_cnt == 0)
		g_idle_add(_regular_idle_out, copy); /* begin to write */
	copy->buf_cnt += read;
	return FALSE;
}

static gboolean _channel_out(Copy * copy, GIOChannel * source)
{
	gsize written;
	GError * error = NULL;

	copy->out_id = 0;
	/* write data */
	if(g_io_channel_write_chars(source, copy->buf, copy->buf_cnt, &written,
				&error) == G_IO_STATUS_ERROR)
	{
		_copy_error(copy, copy->filev[copy->cur], 0);
		gtk_main_quit(); /* XXX ugly */
		return FALSE;
	}
	if(copy->buf_cnt == sizeof(copy->buf))
		g_idle_add(_regular_idle_in, copy); /* read again */
	copy->buf_cnt -= written;
	memmove(copy->buf, &copy->buf[written], copy->buf_cnt);
	copy->cnt += written;
	_out_rate(copy);
	if(copy->buf_cnt > 0)
		g_idle_add(_regular_idle_out, copy); /* continue to write */
	else if(copy->eof == 1) /* reached end of input */
	{
		g_io_channel_close(copy->out_channel);
		gtk_main_quit(); /* XXX ugly */
	}
	return FALSE;
}

static void _out_rate(Copy * copy)
{
	gdouble fraction;
	GtkProgressBar * bar = GTK_PROGRESS_BAR(copy->fprogress);
	char buf[16];

	if(copy->size == 0 || copy->cnt == 0)
	{
		copy->fpulse = 1;
		return;
	}
	fraction = copy->cnt;
	fraction /= copy->size;
	if(gtk_progress_bar_get_fraction(bar) == fraction)
		return;
	gtk_progress_bar_set_fraction(bar, fraction);
	snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
	gtk_progress_bar_set_text(bar, buf);
}

static gboolean _regular_idle_out(gpointer data)
{
	Copy * copy = data;

	if(copy->out_id == 0)
		copy->out_id = g_io_add_watch(copy->out_channel, G_IO_OUT,
				_regular_channel, copy);
	return FALSE;
}

static int _single_p(Copy * copy, char const * dst, struct stat const * st)
{
	struct timeval tv[2];

	if(chown(dst, st->st_uid, st->st_gid) != 0)
	{
		_copy_error(copy, dst, 0);
		if(chmod(dst, st->st_mode & ~(S_ISUID | S_ISGID)) != 0)
			_copy_error(copy, dst, 0);
	}
	else if(chmod(dst, st->st_mode) != 0)
		_copy_error(copy, dst, 0);
	tv[0].tv_sec = st->st_atime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = st->st_mtime;
	tv[1].tv_usec = 0;
	if(utimes(dst, tv) != 0)
		_copy_error(copy, dst, 0);
	return 0;
}

static gboolean _single_timeout(gpointer data)
{
	Copy * copy = data;
	struct timeval tv;
	double rate;
	char buf[16];
	char const * unit = _("kB");

	if(copy->fpulse == 1)
	{
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(copy->fprogress));
		copy->fpulse = 0;
	}
	if(copy->cnt == 0)
	{
		gtk_label_set_text(GTK_LABEL(copy->fspeed), _("0.0 kB/s"));
		return TRUE;
	}
	if(gettimeofday(&tv, NULL) != 0)
		return _copy_error(copy, "gettimeofday", FALSE);
	if((tv.tv_sec = tv.tv_sec - copy->tv.tv_sec) < 0)
		tv.tv_sec = 0;
	if((tv.tv_usec = tv.tv_usec - copy->tv.tv_usec) < 0)
	{
		tv.tv_sec--;
		tv.tv_usec += 1000000;
	}
	rate = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	if((rate = copy->cnt / rate) > 1024)
	{
		rate /= 1024;
		unit = _("MB");
	}
	snprintf(buf, sizeof(buf), "%.1f %s/s", rate, unit);
	gtk_label_set_text(GTK_LABEL(copy->fspeed), buf);
	return TRUE;
}

static int _copy_multiple(Copy * copy, char const * src, char const * dst);

static gboolean _copy_idle_multiple(gpointer data)
{
	Copy * copy = data;

	_copy_multiple(copy, copy->filev[copy->cur],
			copy->filev[copy->filec - 1]);
	copy->cur++;
	if(copy->cur == copy->filec - 1)
	{
		gtk_main_quit();
		return FALSE;
	}
	_copy_refresh(copy);
	return TRUE;
}

static int _copy_multiple(Copy * copy, char const * src, char const * dst)
{
	int ret;
	char * to;
	size_t len;
	char * p;
	char * q;

	if((p = strdup(src)) == NULL)
		return _copy_error(copy, src, 1);
	to = basename(p);
	len = strlen(dst) + strlen(to) + 2;
	if((q = malloc(len * sizeof(char))) == NULL)
	{
		free(p);
		return _copy_error(copy, src, 1);
	}
	sprintf(q, "%s/%s", dst, to);
	ret = _copy_single(copy, src, q);
	free(p);
	free(q);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: copy [-fip] source_file target_file\n\
       copy [-fip] source_file ... target\n\
       copy -R [-H | -L | -P][-fip] source_file ... target\n\
       copy -r [-H | -L | -P][-fip] source_file ... target\n\
  -f	Do not prompt for confirmation if the destination path exists\n\
  -i	Prompt for confirmation if the destination path exists\n\
  -p	Duplicate characteristics of the source files\n\
  -R	Copy file hierarchies\n\
  -r	Copy file hierarchies\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	memset(&prefs, 0, sizeof(prefs));
	prefs |= PREFS_i | PREFS_H;
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "fipRrHLP")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & PREFS_i;
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs -= prefs & PREFS_f;
				prefs |= PREFS_i;
				break;
			case 'p':
				prefs -= prefs & PREFS_p;
				prefs |= PREFS_i;
				break;
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			case 'H':
				prefs -= prefs & (PREFS_L | PREFS_P);
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs -= prefs & (PREFS_H | PREFS_P);
				prefs |= PREFS_L;
				break;
			case 'P':
				prefs -= prefs & (PREFS_H | PREFS_L);
				prefs |= PREFS_P;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	_copy(&prefs, argc - optind, &argv[optind]);
	gtk_main();
	return 0;
}
