/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
/* This file is part of Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "mime.h"


/* View */
/* types */
typedef struct _View
{
	/* widgets */
	GtkWidget * window;
} View;


/* variables */
static Mime * _mime = NULL;
static unsigned int _view_cnt = 0;


/* functions */
static int _view_error(View * view, char const * message, int ret);

/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);


/* view_new */
static GtkWidget * _new_text(View * view, char const * path);

static View * _view_new(char const * path)
{
	View * view;
	struct stat st;
	char const * type;
	char buf[256];
	GtkWidget * widget;

	if((view = malloc(sizeof(*view))) == NULL)
		return NULL; /* FIXME handle error */
	view->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(lstat(path, &st) != 0)
	{
		_view_error(NULL, strerror(errno), 0);
		gtk_widget_destroy(view->window);
		free(view);
		return NULL;
	}
	if(_mime == NULL)
		_mime = mime_new();
	if((type = mime_type(_mime, path)) == NULL)
	{
		_view_error(NULL, "Unknown file type", 0);
		gtk_widget_destroy(view->window);
		free(view);
		return NULL;
	}
	snprintf(buf, sizeof(buf), "%s%s", "View - ", path);
	gtk_window_set_title(GTK_WINDOW(view->window), buf);
	g_signal_connect(view->window, "delete-event", G_CALLBACK(_on_closex),
			view);
	if(strncmp(type, "image/", 6) == 0)
		widget = gtk_image_new_from_pixbuf(
				gdk_pixbuf_new_from_file_at_size(path, -1, -1,
					NULL));
	else if(strncmp(type, "text/", 5) == 0)
	{
		widget = _new_text(view, path);
		gtk_window_set_default_size(GTK_WINDOW(view->window), 600, 400);
	}
	else
	{
		_view_error(NULL, "Unable to view file type", 0);
		gtk_widget_destroy(view->window);
		free(view);
		return NULL;
	}
	gtk_container_add(GTK_CONTAINER(view->window), widget);
	gtk_widget_show_all(view->window);
	_view_cnt++;
	return view;
}

static GtkWidget * _new_text(View * view, char const * path)
{
	GtkWidget * widget;
	GtkWidget * text;
	PangoFontDescription * desc;
	FILE * fp;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	char buf[BUFSIZ];
	size_t len;

	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text), FALSE);
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, "monospace");
	gtk_widget_modify_font(text, desc);
	pango_font_description_free(desc);
	gtk_container_add(GTK_CONTAINER(widget), text);
	if((fp = fopen(path, "r")) == NULL)
	{
		_view_error(view, strerror(errno), 0);
		return widget;
	}
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, buf, len);
	}
	fclose(fp);
	return widget;
}

/* view_delete */
static void _view_delete(View * view)
{
	gtk_widget_destroy(view->window);
	free(view);
	_view_cnt--;
}


/* useful */
static void _error_response(GtkDialog * dialog, gint arg, gpointer data);

static int _view_error(View * view, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(view != NULL ? GTK_WINDOW(view->window)
			: NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_response), NULL);
	gtk_widget_show(dialog);
	return ret;
}

static void _error_response(GtkDialog * dialog, gint arg, gpointer data)
{
	View * view = data;

	if(view != NULL)
		_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
}


/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	View * view = data;

	_view_delete(view);
	if(_view_cnt == 0)
		gtk_main_quit();
	return FALSE;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: view file...\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int i;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	for(i = optind; i < argc; i++)
		_view_new(argv[i]);
	gtk_main();
	return 0;
}
