/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <gtk/gtk.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2


/* Move */
/* types */
typedef struct _Move
{
	Prefs * prefs;
	unsigned int filec;
	char ** filev;
	unsigned int cur;
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
} Move;

/* functions */
static void _move_refresh(Move * move);

/* callbacks */
static void _move_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _move_idle_first(gpointer data);

static int _move(Prefs * prefs, int filec, char * filev[])
{
	static Move move;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	move.prefs = prefs;
	move.filec = filec;
	move.filev = filev;
	move.cur = 0;
	move.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(move.window), "Move file(s)");
	g_signal_connect(G_OBJECT(move.window), "delete_event", G_CALLBACK(
			_move_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	move.label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), move.label, TRUE, TRUE, 4);
	move.progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), move.progress, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(move.window), 4);
	gtk_container_add(GTK_CONTAINER(move.window), vbox);
	g_idle_add(_move_idle_first, &move);
	_move_refresh(&move);
	gtk_widget_show_all(move.window);
	return 0;
}

static void _move_refresh(Move * move)
{
	char buf[256];
	double fraction;

	snprintf(buf, sizeof(buf), "Moving file: %s", move->filev[move->cur]);
	gtk_label_set_text(GTK_LABEL(move->label), buf);
	snprintf(buf, sizeof(buf), "File %u of %u", move->cur, move->filec);
	fraction = (double)(move->cur) / (double)move->filec;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(move->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(move->progress),
			fraction);
}

static int _move_error(Move * move, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(move->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static int _move_confirm(Move * move, char const * dst)
{
	int ret;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(move->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, "%s will be overwritten\n"
			"Proceed?", dst);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret == GTK_RESPONSE_YES ? 1 : 0;
}

static void _move_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_main_quit();
}

static int _move_single(Move * move, char const * src, char const * dst);
static gboolean _move_idle_multiple(gpointer data);
static gboolean _move_idle_first(gpointer data)
{
	Move * move = data;
	struct stat st;

	if(stat(move->filev[move->filec - 1], &st) != 0)
	{
		if(errno != ENOENT)
			_move_error(move, move->filev[move->filec - 1], 0);
		else if(move->filec > 2)
		{
			errno = ENOTDIR;
			_move_error(move, move->filev[move->filec - 1], 0);
		}
		else
			_move_single(move, move->filev[0], move->filev[1]);
	}
	else if(S_ISDIR(st.st_mode))
	{
		g_idle_add(_move_idle_multiple, move);
		return FALSE;
	}
	else if(move->filec > 2)
	{
		errno = ENOTDIR;
		_move_error(move, move->filev[move->filec - 1], 0);
	}
	else
		_move_single(move, move->filev[0], move->filev[1]);
	gtk_main_quit();
	return FALSE;
}

/* move_single */
static int _single_dir(Move * move, char const * src, char const * dst);
static int _single_fifo(Move * move, char const * src, char const * dst);
static int _single_nod(Move * move, char const * src, char const * dst,
		mode_t mode, dev_t rdev);
static int _single_symlink(Move * move, char const * src, char const * dst);
static int _single_regular(Move * move, char const * src, char const * dst);
static int _single_p(Move * move, char const * dst, struct stat const * st);

static int _move_single(Move * move, char const * src, char const * dst)
{
	int ret;
	struct stat st;

	if(lstat(src, &st) != 0 && errno == ENOENT) /* XXX TOCTOU */
		return _move_error(move, src, 1);
	if(*(move->prefs) & PREFS_i
			&& (lstat(dst, &st) == 0 || errno != ENOENT)
			&& _move_confirm(move, dst) != 1)
		return 0;
	if(rename(src, dst) == 0)
		return 0;
	if(errno != EXDEV)
		return _move_error(move, src, 1);
	if(unlink(dst) != 0
			&& errno != ENOENT)
		return _move_error(move, dst, 1);
	if(lstat(src, &st) != 0)
		return _move_error(move, dst, 1);
	if(S_ISDIR(st.st_mode))
		ret = _single_dir(move, src, dst);
	else if(S_ISFIFO(st.st_mode))
		ret = _single_fifo(move, src, dst);
	else if(S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode))
		ret = _single_nod(move, src, dst, st.st_mode, st.st_rdev);
	else if(S_ISLNK(st.st_mode))
		ret = _single_symlink(move, src, dst);
	else if(!S_ISREG(st.st_mode)) /* FIXME not implemented */
	{
		errno = ENOSYS;
		return _move_error(move, src, 1);
	}
	else
		ret = _single_regular(move, src, dst);
	if(ret != 0)
		return ret;
	_single_p(move, dst, &st);
	return 0;
}

static int _single_dir(Move * move, char const * src, char const * dst)
{
	if(mkdir(dst, 0777) != 0)
		return _move_error(move, dst, 1);
	if(rmdir(src) != 0) /* FIXME probably gonna fail, recurse before */
		_move_error(move, src, 0);
	return 0;
}

static int _single_fifo(Move * move, char const * src, char const * dst)
{
	if(mkfifo(dst, 0666) != 0)
		return _move_error(move, dst, 1);
	if(unlink(src) != 0)
		_move_error(move, src, 0);
	return 0;
}

static int _single_nod(Move * move, char const * src, char const * dst,
		mode_t mode, dev_t rdev)
{
	if(mknod(dst, mode, rdev) != 0)
		return _move_error(move, dst, 1);
	if(unlink(src) != 0)
		_move_error(move, src, 0);
	return 0;
}

static int _single_symlink(Move * move, char const * src, char const * dst)
{
	char buf[PATH_MAX];
	ssize_t i;

	if((i = readlink(src, buf, sizeof(buf) - 1)) == -1)
		return _move_error(move, src, 1);
	buf[i] = '\0';
	if(symlink(buf, dst) != 0)
		return _move_error(move, dst, 1);
	if(unlink(src) != 0)
		_move_error(move, src, 0);
	return 0;
}

static int _single_regular(Move * move, char const * src, char const * dst)
{
	FILE * fp;
	char buf[BUFSIZ];
	size_t i;

	if((fp = fopen(dst, "w+")) == NULL)
		return _move_error(move, dst, 1);
	while((i = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
		if(fwrite(buf, sizeof(char), i, fp) != i)
			break;
	if(fclose(fp) != 0
			|| i != 0)
		return _move_error(move, dst, 1);
	if(unlink(src) != 0)
		_move_error(move, src, 0);
	return 0;
}

static int _single_p(Move * move, char const * dst, struct stat const * st)
{
	struct timeval tv[2];

	if(lchown(dst, st->st_uid, st->st_gid) != 0) /* XXX TOCTOU */
	{
		_move_error(move, dst, 0);
		if(chmod(dst, st->st_mode & ~(S_ISUID | S_ISGID)) != 0)
			_move_error(move, dst, 0);
	}
	else if(chmod(dst, st->st_mode) != 0)
		_move_error(move, dst, 0);
	tv[0].tv_sec = st->st_atime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = st->st_mtime;
	tv[1].tv_usec = 0;
	if(utimes(dst, tv) != 0)
		_move_error(move, dst, 0);
	return 0;
}

static int _move_multiple(Move * move, char const * src, char const * dst);
static gboolean _move_idle_multiple(gpointer data)
{
	Move * move = data;

	_move_multiple(move, move->filev[move->cur],
			move->filev[move->filec - 1]);
	move->cur++;
	if(move->cur == move->filec - 1)
	{
		gtk_main_quit();
		return FALSE;
	}
	_move_refresh(move);
	return TRUE;
}

static int _move_multiple(Move * move, char const * src, char const * dst)
{
	int ret;
	char * to;
	size_t len;
	char * p;

	to = basename(src); /* XXX src is const */
	len = strlen(src + strlen(to) + 2);
	if((p = malloc(len * sizeof(char))) == NULL)
		return _move_error(move, src, 1);
	sprintf(p, "%s/%s", dst, to);
	ret = _move_single(move, src, p);
	free(p);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: move [-fi] file...\n\
  -f	Do not prompt for confirmation if the destination path exists\n\
  -i	Prompt for confirmation if the destination path exists\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "fi")) != -1)
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
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	_move(&prefs, argc - optind, &argv[optind]);
	gtk_main();
	return 0;
}
