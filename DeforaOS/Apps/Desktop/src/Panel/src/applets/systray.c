/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Pager Panel */
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
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "Panel.h"


/* Systray */
/* private */
/* types */
typedef struct _Systray
{
	GtkWidget * hbox;
	GtkWidget * owner;
} Systray;


/* constants */
#define SYSTEM_TRAY_REQUEST_DOCK	0
#define SYSTEM_TRAY_BEGIN_MESSAGE	1
#define SYSTEM_TRAY_CANCEL_MESSAGE	2


/* prototypes */
static GtkWidget * _systray_init(PanelApplet * applet);
static void _systray_destroy(PanelApplet * applet);

/* useful */
static void _systray_do(Systray * systray);

/* callbacks */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_systray_init,
	_systray_destroy,
	PANEL_APPLET_POSITION_LAST,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* systray_init */
static GtkWidget * _systray_init(PanelApplet * applet)
{
	Systray * systray;

	if((systray = malloc(sizeof(*systray))) == NULL)
	{
		applet->helper->error(applet->helper->priv, "malloc", 0);
		return NULL;
	}
	applet->priv = systray;
	systray->hbox = gtk_hbox_new(TRUE, 0);
	systray->owner = gtk_invisible_new();
	g_signal_connect(G_OBJECT(systray->hbox), "screen-changed", G_CALLBACK(
				_on_screen_changed), systray);
	gtk_widget_show(systray->hbox);
	return systray->hbox;
}


/* systray_destroy */
static void _systray_destroy(PanelApplet * applet)
{
	Systray * systray = applet->priv;

	free(systray);
}


/* useful */
/* systray_do */
static void _systray_do(Systray * systray)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* callbacks */
/* on_filter */
static GdkFilterReturn _filter_clientmessage(Systray * systray,
		XClientMessageEvent * xev);

static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Systray * systray = data;
	XEvent * xev = xevent;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(xev->type == ClientMessage)
		return _filter_clientmessage(systray, &xev->xclient);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_clientmessage(Systray * systray,
		XClientMessageEvent * xev)
{
	GtkWidget * socket;

	switch(xev->data.l[1])
	{
		case SYSTEM_TRAY_REQUEST_DOCK:
			socket = gtk_socket_new();
			gtk_widget_show(socket);
			gtk_box_pack_start(GTK_BOX(systray->hbox), socket,
					FALSE, TRUE, 0);
			gtk_socket_add_id(GTK_SOCKET(socket), xev->data.l[2]);
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() %ld\n", __func__,
					xev->data.l[2]);
#endif
			return GDK_FILTER_REMOVE;
	}
	return GDK_FILTER_CONTINUE;
}


/* on_screen_changed */
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data)
{
	Systray * systray = data;
	const char name[] = "_NET_SYSTEM_TRAY_S";
	char buf[sizeof(name) + 2];
	GdkAtom atom;
	GdkScreen * screen;
	GdkDisplay * display;
	GdkWindow * root;
	XEvent xev;

	screen = gtk_widget_get_screen(widget);
	snprintf(buf, sizeof(buf), "%s%d", name, gdk_screen_get_number(screen));
	atom = gdk_atom_intern(buf, FALSE);
	gtk_widget_realize(systray->owner);
	if(gdk_selection_owner_set(systray->owner->window, atom,
				gtk_get_current_event_time(), TRUE) != TRUE)
		return;
	display = gtk_widget_get_display(widget);
	root = gdk_screen_get_root_window(screen);
	xev.xclient.type = ClientMessage;
	xev.xclient.window = GDK_WINDOW_XWINDOW(root);
	xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(
			display, "MANAGER");
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = gtk_get_current_event_time();
	xev.xclient.data.l[1] = gdk_x11_atom_to_xatom(atom);
	xev.xclient.data.l[2] = GDK_WINDOW_XWINDOW(systray->owner->window);
	XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XWINDOW(root),
			False, StructureNotifyMask, &xev);
	memset(&xev.xclient.data, sizeof(xev.xclient.data), 0);
	gtk_widget_add_events(systray->owner, GDK_PROPERTY_CHANGE_MASK
			| GDK_STRUCTURE_MASK);
	gdk_window_add_filter(systray->owner->window, _on_filter, systray);
	_systray_do(systray);
}
