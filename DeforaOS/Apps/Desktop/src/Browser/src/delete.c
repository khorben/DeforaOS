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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>


/* Delete */
/* types */
typedef struct _Delete
{
	GtkWidget * window;
	GtkWidget * label;
	GtkWidget * progress;
	int filec;
	char ** filev;
	int cur;
	int err_cnt;
} Delete;

/* functions */
static void _delete_refresh(Delete * delete);

/* callbacks */
static void _delete_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _delete_idle(gpointer data);

static int _delete(int filec, char * filev[])
{
	static Delete delete;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	delete.filec = filec;
	delete.filev = filev;
	delete.cur = 0;
	delete.err_cnt = 0;
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
	snprintf(buf, sizeof(buf), "File %u of %u", delete->cur+1,
			delete->filec);
	fraction = (double)(delete->cur + 1) / (double)delete->filec;
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(delete->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(delete->progress),
			fraction);
}

static void _error_on_close(GtkDialog * dialog, gint arg, gpointer data);
static int _delete_error(Delete * delete, char const * message, int ret)
{
	GtkWidget * dialog;

	delete->err_cnt++;
	dialog = gtk_message_dialog_new(GTK_WINDOW(delete->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK, "%s: %s",
			delete->filev[delete->cur-1], strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(dialog, "response", G_CALLBACK(_error_on_close),
			delete);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_on_close(GtkDialog * dialog, gint arg, gpointer data)
{
	Delete * delete = data;

	gtk_widget_destroy(GTK_WIDGET(dialog));
	delete->err_cnt--;
	if(delete->cur != delete->filec)
		return;
	if(delete->err_cnt == 0)
		gtk_main_quit();
}

static void _delete_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_main_quit();
}

static gboolean _delete_idle(gpointer data)
{
	Delete * delete = (Delete*)data;

	if(unlink(delete->filev[delete->cur++]) != 0)
		_delete_error(delete, delete->filev[delete->cur-1], 0);
	if(delete->cur == delete->filec)
	{
		if(delete->err_cnt == 0)
			gtk_main_quit();
		return FALSE;
	}
	_delete_refresh(delete);
	return TRUE;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: delete file...\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_delete(argc - optind, &argv[optind]);
	gtk_main();
	return 0;
}
