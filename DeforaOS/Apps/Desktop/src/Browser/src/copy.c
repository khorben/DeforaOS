/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



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
#include <gtk/gtk.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x01
#define PREFS_i 0x02
#define PREFS_p 0x04
#define PREFS_R 0x08
#define PREFS_H 0x10
#define PREFS_L 0x20
#define PREFS_P 0x40


/* Copy */
/* types */
typedef struct _Copy
{
	Prefs * prefs;
	unsigned int filec;
	char ** filev;
	unsigned int cur;
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
} Copy;

/* functions */
static void _copy_refresh(Copy * copy);

/* callbacks */
static void _copy_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _copy_idle_first(gpointer data);

static int _copy(Prefs * prefs, int filec, char * filev[])
{
	static Copy copy;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	copy.prefs = prefs;
	copy.filec = filec;
	copy.filev = filev;
	copy.cur = 0;
	copy.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(copy.window), "Copy file(s)");
	g_signal_connect(G_OBJECT(copy.window), "delete_event", G_CALLBACK(
			_copy_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	copy.label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), copy.label, TRUE, TRUE, 4);
	copy.progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), copy.progress, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(copy.window), 4);
	gtk_container_add(GTK_CONTAINER(copy.window), vbox);
	g_idle_add(_copy_idle_first, &copy);
	_copy_refresh(&copy);
	gtk_widget_show_all(copy.window);
	return 0;
}

static void _copy_refresh(Copy * copy)
{
	char buf[256]; /* FIXME convert to UTF-8 */
	double fraction;

	snprintf(buf, sizeof(buf), "Copying file: %s", copy->filev[copy->cur]);
	gtk_label_set_text(GTK_LABEL(copy->label), buf);
	snprintf(buf, sizeof(buf), "File %u of %u", copy->cur, copy->filec - 1);
	fraction = (double)(copy->cur) / (double)copy->filec;
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
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
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
	gtk_window_set_title(GTK_WINDOW(dialog), "Information");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static int _copy_confirm(Copy * copy, char const * dst)
{
	int ret;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(copy->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, "%s will be overwritten\n"
			"Proceed?", dst);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret == GTK_RESPONSE_YES ? 1 : 0;
}

static void _copy_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
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

static int _copy_single(Copy * copy, char const * src, char const * dst)
{
	int ret;
	struct stat st;
	struct stat st2;

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
		ret = _single_regular(copy, src, dst);
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
	_copy_info(copy, src, "Omitting directory");
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

static int _single_regular(Copy * copy, char const * src, char const * dst)
{
	int ret = 0;
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;

	if((fsrc = fopen(src, "r")) == NULL)
		return _copy_error(copy, src, 1);
	if((fdst = fopen(dst, "w")) == NULL)
	{
		ret = _copy_error(copy, dst, 1);
		fclose(fsrc);
		return ret;
	}
	while((size = fread(buf, sizeof(char), sizeof(buf), fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != size)
			break;
	if(!feof(fsrc))
		ret |= _copy_error(copy, size == 0 ? src : dst, 1);
	if(fclose(fsrc) != 0)
		ret |= _copy_error(copy, src, 1);
	if(fclose(fdst) != 0)
		ret |= _copy_error(copy, dst, 1);
	return ret;
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

	to = basename(src); /* XXX src is const */
	len = strlen(src + strlen(to) + 2);
	if((p = malloc(len * sizeof(char))) == NULL)
		return _copy_error(copy, src, 1);
	sprintf(p, "%s/%s", dst, to);
	ret = _copy_single(copy, src, p);
	free(p);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: copy [-fip] source_file target_file\n\
       copy [-fip] source_file ... target\n\
       copy -R [-H | -L | -P][-fip] source_file ... target\n\
       copy -r [-H | -L | -P][-fip] source_file ... target\n\
  -f	Do not prompt for confirmation if the destination path exists\n\
  -i	Prompt for confirmation if the destination path exists\n\
  -p	Duplicate characteristics of the source files\n\
  -R	Copy file hierarchies\n\
  -r	Copy file hierarchies\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

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
