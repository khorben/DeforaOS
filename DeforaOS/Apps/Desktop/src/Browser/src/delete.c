/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
#include <gtk/gtk.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2
#define PREFS_R 0x4


/* Delete */
/* types */
typedef struct _Delete
{
	Prefs * prefs;
	unsigned int filec;
	char ** filev;
	unsigned int cur;
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
} Delete;

/* functions */
static void _delete_refresh(Delete * delete);

/* callbacks */
static void _delete_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _delete_idle(gpointer data);

static int _delete(Prefs * prefs, int filec, char * filev[])
{
	static Delete delete;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	delete.prefs = prefs;
	delete.filec = filec;
	delete.filev = filev;
	delete.cur = 0;
	delete.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(delete.window), "Delete file(s)");
	g_signal_connect(G_OBJECT(delete.window), "delete_event", G_CALLBACK(
			_delete_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	delete.label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox), delete.label, TRUE, TRUE, 4);
	delete.progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), delete.progress, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(delete.window), 4);
	gtk_container_add(GTK_CONTAINER(delete.window), vbox);
	g_idle_add(_delete_idle, &delete);
	_delete_refresh(&delete);
	gtk_widget_show_all(delete.window);
	return 0;
}

static void _delete_refresh(Delete * delete)
{
	char buf[256];
	double fraction;

	snprintf(buf, sizeof(buf), "Deleting file: %s",
			delete->filev[delete->cur]);
	gtk_label_set_text(GTK_LABEL(delete->label), buf);
	snprintf(buf, sizeof(buf), "File %u of %u", delete->cur, delete->filec);
	fraction = (double)(delete->cur) / (double)delete->filec;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(delete->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(delete->progress),
			fraction);
}

static int _delete_error(Delete * delete, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static void _delete_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_main_quit();
}

static int _idle_do(Delete * delete, char const * filename);
static gboolean _delete_idle(gpointer data)
{
	Delete * delete = data;

	_idle_do(delete, delete->filev[delete->cur]);
	delete->cur++;
	if(delete->cur == delete->filec)
	{
		gtk_main_quit();
		return FALSE;
	}
	_delete_refresh(delete);
	return TRUE;
}

static int _idle_ask_recursive(Delete * delete, char const * filename);
static int _idle_ask(Delete * delete, char const * filename);
static int _idle_do_recursive(Delete * delete, char const * filename);
static int _idle_do(Delete * delete, char const * filename)
{
	struct stat st;

	if(lstat(filename, &st) != 0 && errno == ENOENT)
	{
		if(!(*(delete->prefs) & PREFS_f))
			return _delete_error(delete, filename, 1);
		return 0;
	}
	if(S_ISDIR(st.st_mode))
	{
		if(!(*(delete->prefs) & PREFS_R))
		{
			errno = EISDIR;
			return _delete_error(delete, filename, 1);
		}
		else if((*(delete->prefs) & PREFS_f)
				|| _idle_ask_recursive(delete, filename) == 0)
			return _idle_do_recursive(delete, filename);
	}
	else if((*(delete->prefs) & PREFS_f)
			|| _idle_ask(delete, filename) == 0)
		if(unlink(filename) != 0)
			return _delete_error(delete, filename, 1);
	return 0;
}

static int _idle_ask_recursive(Delete * delete, char const * filename)
{
	int ret;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			"%s is a directory\nRecursively delete?", filename);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes to all", 1);
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(ret == 1)
	{
		*(delete->prefs) = (*(delete->prefs) & ~PREFS_i) | PREFS_f;
		return 0;
	}
	return ret == GTK_RESPONSE_YES ? 0 : 1;
}

static int _idle_ask(Delete * delete, char const * filename)
{
	int ret;
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
			"%s will be permanently deleted\nContinue?", filename);
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes to all", 1);
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(ret == 1)
	{
		*(delete->prefs) = (*(delete->prefs) & ~PREFS_i) | PREFS_f;
		return 0;
	}
	return ret == GTK_RESPONSE_YES ? 0 : 1;
}

/* FIXME does not update or redraw the dialog */
static int _idle_do_recursive(Delete * delete, char const * filename)
{
	int ret = 0;
	DIR * dir;
	struct dirent * de;
	size_t len = strlen(filename) + 2;
	char * path;
	char * p;

	if((dir = opendir(filename)) == NULL)
		return _delete_error(delete, filename, 1);
	if((path = malloc(len)) == NULL)
	{
		closedir(dir);
		return _delete_error(delete, filename, 1);
	}
	sprintf(path, "%s/", filename);
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(path, len + strlen(de->d_name))) == NULL)
			break;
		path = p;
		strcpy(&path[len - 1], de->d_name);
		ret |= _idle_do(delete, path);
	}
	free(path);
	closedir(dir);
	if(de != NULL)
		return _delete_error(delete, filename, 1);
	if(rmdir(filename) != 0) /* FIXME confirm */
		return _delete_error(delete, filename, 1);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: delete [-fiRr] file...\n\
  -f	Do not prompt for confirmation or output error messages\n\
  -i	Prompt for confirmation\n\
  -R	Remove file hierarchies\n\
  -r	Equivalent to -R\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

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
