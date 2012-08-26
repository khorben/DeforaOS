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



#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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


/* Delete */
/* private */
/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2
#define PREFS_R 0x4

typedef struct _DeleteDir DeleteDir;

typedef struct _Delete
{
	Prefs * prefs;
	unsigned int filec;
	char ** filev;
	unsigned int file_cur;

	struct dirent * de;
	DeleteDir ** dirv;
	size_t dirv_cnt;

	/* widgets */
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
} Delete;

struct _DeleteDir
{
	DIR * dir;
	char * filename;
};

/* prototypes */
static int _delete_error(Delete * delete, char const * message, int ret);
static int _delete_filename_error(Delete * delete, char const * filename,
		int ret);


/* functions */
/* delete */
static void _delete_refresh(Delete * delete, char const * filename);
/* callbacks */
static void _delete_on_cancel(gpointer data);
static void _delete_on_closex(gpointer data);
static gboolean _delete_idle(gpointer data);

static int _delete(Prefs * prefs, unsigned int filec, char * filev[])
{
	static Delete delete;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	PangoFontDescription * bold;

	if(filec < 1 || filev == NULL)
		return 1;
	delete.prefs = prefs;
	delete.filec = filec;
	delete.filev = filev;
	delete.file_cur = 0;
	delete.de = NULL;
	delete.dirv = NULL;
	delete.dirv_cnt = 0;
	/* graphical interface */
	delete.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(delete.window), FALSE);
	gtk_window_set_title(GTK_WINDOW(delete.window), _("Delete file(s)"));
	g_signal_connect(G_OBJECT(delete.window), "delete-event", G_CALLBACK(
			_delete_on_closex), &delete);
	vbox = gtk_vbox_new(FALSE, 4);
	/* current argument */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Deleting: "));
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	gtk_widget_modify_font(widget, bold);
	pango_font_description_free(bold);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	delete.label = gtk_label_new("");
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_label_set_ellipsize(GTK_LABEL(delete.label), PANGO_ELLIPSIZE_END);
	gtk_label_set_width_chars(GTK_LABEL(delete.label), 25);
#endif
	gtk_misc_set_alignment(GTK_MISC(delete.label), 0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), delete.label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	/* progress bar */
	delete.progress = gtk_progress_bar_new();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(delete.progress), " ");
	gtk_box_pack_start(GTK_BOX(vbox), delete.progress, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_delete_on_cancel), &delete);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(delete.window), 4);
	gtk_container_add(GTK_CONTAINER(delete.window), vbox);
	g_idle_add(_delete_idle, &delete);
	_delete_refresh(&delete, "");
	gtk_widget_show_all(delete.window);
	return 0;
}

static void _delete_refresh(Delete * delete, char const * filename)
{
	char * p;
	char buf[64];
	double fraction;

	if((p = g_filename_to_utf8(filename, -1, NULL, NULL, NULL)) != NULL)
		filename = p;
	gtk_label_set_text(GTK_LABEL(delete->label), filename);
	free(p);
	snprintf(buf, sizeof(buf), _("File %u of %u"), delete->file_cur + 1,
			delete->filec);
	fraction = delete->file_cur;
	fraction /= delete->filec;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(delete->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(delete->progress),
			fraction);
}

static void _delete_on_cancel(gpointer data)
{
	Delete * delete = data;
	size_t i;

	gtk_widget_hide(delete->window);
	for(i = delete->dirv_cnt; i >= 1; i--)
	{
		if(delete->dirv[i - 1]->dir != NULL)
			closedir(delete->dirv[i - 1]->dir);
		free(delete->dirv[i - 1]->filename);
		free(delete->dirv[i - 1]);
	}
	free(delete->dirv);
	gtk_main_quit();
}

static void _delete_on_closex(gpointer data)
{
	Delete * delete = data;

	_delete_on_cancel(delete);
}

static int _idle_do(Delete * delete);
static gboolean _delete_idle(gpointer data)
{
	Delete * delete = data;

	_idle_do(delete);
	if(delete->file_cur == delete->filec)
	{
		gtk_main_quit();
		return FALSE;
	}
	return TRUE;
}

static int _idle_do_file(Delete * delete, char const * filename);
static int _idle_do_readdir(Delete * delete);
static int _idle_do_closedir(Delete * delete);
static int _idle_ask_recursive(Delete * delete, char const * filename);
static int _idle_ask(Delete * delete, char const * filename);
static int _idle_do_opendir(Delete * delete, char const * filename);
static int _idle_do(Delete * delete)
{
	int ret;

	if(delete->dirv_cnt > 0)
	{
		ret = _idle_do_readdir(delete);
		if(delete->de != NULL)
			return ret;
		return _idle_do_closedir(delete);
	}
	ret = _idle_do_file(delete, delete->filev[delete->file_cur]);
	if(delete->dirv_cnt == 0)
		delete->file_cur++;
	return ret;
}

static int _idle_do_file(Delete * delete, char const * filename)
{
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	_delete_refresh(delete, filename);
	if(lstat(filename, &st) != 0 && errno == ENOENT)
	{
		if(!(*(delete->prefs) & PREFS_f))
			return _delete_filename_error(delete, filename, 1);
		return 0;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(!(*(delete->prefs) & PREFS_R))
		{
			errno = EISDIR;
			return _delete_filename_error(delete, filename, 1);
		}
		else if((*(delete->prefs) & PREFS_f)
				|| _idle_ask_recursive(delete, filename) == 0)
			return _idle_do_opendir(delete, filename);
	}
	else if((*(delete->prefs) & PREFS_f)
			|| _idle_ask(delete, filename) == 0)
#ifdef DEBUG
		fprintf(stderr, "DEBUG: unlink(\"%s\")\n", filename);
#else
		if(unlink(filename) != 0)
			return _delete_filename_error(delete, filename, 1);
#endif
	return 0;
}

static int _idle_do_readdir(Delete * delete)
{
	int ret = 0;
	DIR * dir = delete->dirv[delete->dirv_cnt - 1]->dir;
	char const * parent;
	size_t len;
	char * p;

	if((delete->de = readdir(dir)) == NULL)
		return 0;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, delete->de->d_name);
#endif
	if(strcmp(delete->de->d_name, ".") == 0
			|| strcmp(delete->de->d_name, "..") == 0)
		return 0;
	parent = delete->dirv[delete->dirv_cnt - 1]->filename;
	len = strlen(parent) + strlen(delete->de->d_name) + 2;
	if((p = malloc(len)) == NULL)
		return _delete_filename_error(delete, parent, 1);
	snprintf(p, len, "%s/%s", parent, delete->de->d_name);
	ret = _idle_do_file(delete, p);
	free(p);
	return ret;
}

static int _idle_do_closedir(Delete * delete)
{
	DeleteDir * dd = delete->dirv[delete->dirv_cnt - 1];

	closedir(dd->dir);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: rmdir(\"%s\")\n", dd->filename);
#endif
	if(rmdir(dd->filename) != 0)
		_delete_filename_error(delete, dd->filename, 1);
	free(dd->filename);
	free(dd);
	if(--delete->dirv_cnt == 0)
	{
		free(delete->dirv);
		delete->dirv = NULL;
		delete->file_cur++;
	}
	return 0;
}

static int _idle_ask_recursive(Delete * delete, char const * filename)
{
	char * p;
	GtkWidget * dialog;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	if((p = g_filename_to_utf8(filename, -1, NULL, NULL, NULL)) != NULL)
		filename = p;
	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			_("%s is a directory.\nRecursively delete?"), filename);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Yes to all"), 1);
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	free(p);
	if(res == 1)
	{
		*(delete->prefs) = (*(delete->prefs) & ~PREFS_i) | PREFS_f;
		return 0;
	}
	return (res == GTK_RESPONSE_YES) ? 0 : 1;
}

static int _idle_ask(Delete * delete, char const * filename)
{
	int ret;
	char * p;
	GtkWidget * dialog;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	if((p = g_filename_to_utf8(filename, -1, NULL, NULL, NULL)) != NULL)
		filename = p;
	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Question"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			_("%s will be permanently deleted.\nContinue?"),
			filename);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Question"));
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("Yes to all"), 1);
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	free(p);
	if(ret == 1)
	{
		*(delete->prefs) = (*(delete->prefs) & ~PREFS_i) | PREFS_f;
		return 0;
	}
	return (ret == GTK_RESPONSE_YES) ? 0 : 1;
}

static int _idle_do_opendir(Delete * delete, char const * filename)
{
	DeleteDir * dd;
	DeleteDir ** d;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	if((dd = malloc(sizeof(*dd))) == NULL)
		return _delete_filename_error(delete, filename, 1);
	if((d = realloc(delete->dirv, sizeof(*d) * (delete->dirv_cnt + 1)))
			== NULL)
	{
		free(dd);
		return _delete_filename_error(delete, filename, 1);
	}
	delete->dirv = d;
	d = &delete->dirv[delete->dirv_cnt];
	if((dd->filename = strdup(filename)) == NULL)
	{
		free(dd);
		return _delete_filename_error(delete, filename, 1);
	}
	if((dd->dir = opendir(filename)) == NULL)
	{
		free(dd);
		return _delete_filename_error(delete, filename, 1);
	}
	*d = dd;
	delete->dirv_cnt++;
	return 0;
}


/* delete_error */
static int _delete_error(Delete * delete, char const * message, int ret)
{
	GtkWidget * dialog;
	char const * error = strerror(errno);

	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message,
#endif
			error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* delete_filename_error */
static int _delete_filename_error(Delete * delete, char const * filename,
		int ret)
{
	char * p;
	int error = errno;

	if((p = g_filename_to_utf8(filename, -1, NULL, NULL, NULL)) != NULL)
		filename = p;
	errno = error;
	ret = _delete_error(delete, filename, ret);
	free(p);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: delete [-fiRr] file...\n\
  -f	Do not prompt for confirmation and ignore errors\n\
  -i	Prompt for confirmation\n\
  -R	Remove file hierarchies\n\
  -r	Equivalent to -R\n"), stderr);
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
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "fiRr")) != -1)
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
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_delete(&prefs, argc - optind, &argv[optind]);
	gtk_main();
	return 0;
}
