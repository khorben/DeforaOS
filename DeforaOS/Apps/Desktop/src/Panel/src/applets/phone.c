/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#include <stdlib.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include "Panel.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Phone */
/* private */
/* constants */
#define PHONE_EMBED_MESSAGE	"DEFORAOS_DESKTOP_PHONE_EMBED"


/* prototypes */
static GtkWidget * _phone_init(PanelApplet * applet);

/* callbacks */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static gboolean _on_plug_removed(void);
static void _on_screen_changed(GtkWidget * widget);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_phone_init,
	NULL,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* Phone */
/* phone_init */
static GtkWidget * _phone_init(PanelApplet * applet)
{
	GtkWidget * socket;

	socket = gtk_socket_new();
	g_signal_connect(G_OBJECT(socket), "plug-removed", G_CALLBACK(
				_on_plug_removed), NULL);
	g_signal_connect(G_OBJECT(socket), "screen-changed", G_CALLBACK(
				_on_screen_changed), NULL);
	gtk_widget_show(socket);
	return socket;
}


/* phone_do */
static void _phone_do(GtkWidget * widget, GdkNativeWindow window)
{
	gtk_socket_add_id(GTK_SOCKET(widget), window);
}


/* callbacks */
/* on_filter */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	GtkWidget * widget = data;
	XEvent * xev = xevent;

	if(xev->type != ClientMessage)
		return GDK_FILTER_CONTINUE;
	_phone_do(widget, xev->xclient.data.l[0]);
	return GDK_FILTER_CONTINUE;
}


/* on_plug_removed */
static gboolean _on_plug_removed(void)
{
	return TRUE;
}


/* on_screen_changed */
static void _on_screen_changed(GtkWidget * widget)
{
	gdk_add_client_message_filter(gdk_atom_intern(PHONE_EMBED_MESSAGE,
				FALSE), _on_filter, widget);
}
