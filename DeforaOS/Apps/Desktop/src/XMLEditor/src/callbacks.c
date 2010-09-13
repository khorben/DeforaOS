/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop XMLEditor */
static char const _license[] =
"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by the\n"
"Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.\n";



#include <stdlib.h>
#include <libintl.h>
#include <Desktop.h>
#include "xmleditor.h"
#include "callbacks.h"
#include "../config.h"


/* private */
/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* public */
/* functions */
gboolean on_closex(gpointer data)
{
	XMLEditor * xmleditor = data;

	return xmleditor_close(xmleditor);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	/* FIXME implement */
}


/* on_file_close */
void on_file_close(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_close(xmleditor);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open(xmleditor, NULL);
}


/* on_file_open */
void on_file_open(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open_dialog(xmleditor);
}


/* on_file_save */
void on_file_save(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save(xmleditor);
}


/* on_file_save_as */
void on_file_save_as(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save_as_dialog(xmleditor);
}


/* on_help_about */
static gboolean _about_on_closex(gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif

void on_help_about(gpointer data)
{
	XMLEditor * xmleditor = data;
	static GtkWidget * window = NULL;
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		xmleditor_error(xmleditor, "malloc", 0);
		return;
	}
	window = desktop_about_dialog_new();
#if 0 /* XXX move this function to xmleditor.c */
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				xmleditor->window));
#endif
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	desktop_about_dialog_set_name(window, PACKAGE);
	desktop_about_dialog_set_version(window, VERSION);
	desktop_about_dialog_set_authors(window, _authors);
	desktop_about_dialog_set_copyright(window, _copyright);
	desktop_about_dialog_set_logo_icon_name(window, "text-editor");
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	gtk_widget_show(window);
}

static gboolean _about_on_closex(gpointer data)
{
	GtkWidget * widget = data;

	gtk_widget_hide(widget);
	return TRUE;
}


/* toolbar */
/* on_close */
void on_close(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_close(xmleditor);
}


/* on_new */
void on_new(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open(xmleditor, NULL);
}


/* on_open */
void on_open(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_open_dialog(xmleditor);
}


/* on_save */
void on_save(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save(xmleditor);
}


/* on_save_as */
void on_save_as(gpointer data)
{
	XMLEditor * xmleditor = data;

	xmleditor_save_as_dialog(xmleditor);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	XMLEditor * xmleditor = data;

	on_edit_preferences(xmleditor);
}
