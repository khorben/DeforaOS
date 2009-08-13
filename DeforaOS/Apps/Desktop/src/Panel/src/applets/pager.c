/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
#include <string.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "panel.h"


/* Pager */
/* private */
/* types */
typedef enum _PagerAtom
{
	PAGER_ATOM_NET_CURRENT_DESKTOP = 0,
	PAGER_ATOM_NET_NUMBER_OF_DESKTOPS
} PagerAtom;
#define PAGER_ATOM_LAST PAGER_ATOM_NET_NUMBER_OF_DESKTOPS
#define PAGER_ATOM_COUNT (PAGER_ATOM_LAST + 1)

typedef struct _Pager
{
	GtkWidget * hbox;

	GtkWidget ** widgets;
	size_t widgets_cnt;

	Atom atom[PAGER_ATOM_COUNT];
	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;
} Pager;


/* constants */
static const char * _pager_atom[PAGER_ATOM_COUNT] =
{
	"_NET_CURRENT_DESKTOP",
	"_NET_NUMBER_OF_DESKTOPS"
};


/* prototypes */
static GtkWidget * _pager_init(PanelApplet * applet);
static void _pager_destroy(PanelApplet * applet);

/* accessors */
static int _pager_get_window_property(Pager * pager, Window window,
		PagerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret);

/* useful */
static void _pager_do(Pager * pager);

/* callbacks */
static void _on_clicked(GtkWidget * widget, gpointer data);
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_pager_init,
	_pager_destroy,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* pager_init */
static GtkWidget * _pager_init(PanelApplet * applet)
{
	Pager * pager;

	if((pager = malloc(sizeof(*pager))) == NULL)
		return NULL;
	applet->priv = pager;
	pager->hbox = gtk_hbox_new(TRUE, 0);
	g_signal_connect(G_OBJECT(pager->hbox), "screen-changed", G_CALLBACK(
				_on_screen_changed), pager);
	pager->widgets = NULL;
	pager->widgets_cnt = 0;
	pager->screen = NULL;
	pager->display = NULL;
	pager->root = NULL;
	return pager->hbox;
}


/* pager_destroy */
static void _pager_destroy(PanelApplet * applet)
{
	Pager * pager = applet->priv;

	free(pager);
}


/* accessors */
/* pager_get_window_property */
static int _pager_get_window_property(Pager * pager, Window window,
		PagerAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(pager, window, %s, %lu)\n", __func__,
			_pager_atom[property], atom);
#endif
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(pager->display), window,
			pager->atom[property], 0, G_MAXLONG, False, atom,
			&type, &format, cnt, &bytes, ret);
	if(res != Success)
		return 1;
	if(type != atom)
	{
		if(*ret != NULL)
			XFree(*ret);
		*ret = NULL;
		return 1;
	}
	return 0;
}


/* useful */
static void _pager_do(Pager * pager)
{
	unsigned long cnt = 0;
	unsigned long l;
	unsigned long * p;
	unsigned long i;
	GtkWidget ** q;
	char buf[16];

	if(_pager_get_window_property(pager, GDK_WINDOW_XWINDOW(pager->root),
				PAGER_ATOM_NET_NUMBER_OF_DESKTOPS,
				XA_CARDINAL, &cnt, (void*)&p) != 0)
		return;
	l = *p;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %ld\n", __func__, l);
#endif
	XFree(p);
	for(i = 0; i < pager->widgets_cnt; i++)
		if(pager->widgets[i] != NULL)
		{
			gtk_widget_destroy(pager->widgets[i]);
			pager->widgets[i] = NULL;
		}
	if((q = realloc(pager->widgets, l * sizeof(*q))) == NULL)
		return;
	pager->widgets = q;
	pager->widgets_cnt = l;
	for(i = 0; i < l; i++)
	{
		snprintf(buf, sizeof(buf), "Desk %ld\n", i + 1);
		pager->widgets[i] = gtk_button_new_with_label(buf);
		g_signal_connect(G_OBJECT(pager->widgets[i]), "clicked",
				G_CALLBACK(_on_clicked), pager);
		gtk_box_pack_start(GTK_BOX(pager->hbox), pager->widgets[i],
				FALSE, TRUE, 0);
	}
	gtk_widget_show_all(pager->hbox);
}


/* callbacks */
/* on_clicked */
static void _on_clicked(GtkWidget * widget, gpointer data)
{
	Pager * pager = data;
	size_t i;
	GdkScreen * screen;
	GdkDisplay * display;
	GdkWindow * root;
	XEvent xev;

	for(i = 0; i < pager->widgets_cnt; i++)
		if(pager->widgets[i] == widget)
			break;
	if(i == pager->widgets_cnt)
		return;
	screen = gtk_widget_get_screen(widget);
	display = gtk_widget_get_display(widget);
	root = gdk_screen_get_root_window(screen);
	xev.xclient.type = ClientMessage;
	xev.xclient.window = GDK_WINDOW_XWINDOW(root);
	xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(
			display, "_NET_CURRENT_DESKTOP");
	xev.xclient.format = 32;
	memset(&xev.xclient.data, sizeof(xev.xclient.data), 0);
	xev.xclient.data.l[0] = i;
	XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XWINDOW(root),
			False,
			SubstructureNotifyMask | SubstructureRedirectMask,
			&xev);
}


/* on_filter */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Pager * pager = data;
	XEvent * xev = xevent;

	if(xev->type != PropertyNotify)
		return GDK_FILTER_CONTINUE;
	if(xev->xproperty.atom != pager->atom[
			PAGER_ATOM_NET_NUMBER_OF_DESKTOPS])
		return GDK_FILTER_CONTINUE;
	_pager_do(pager);
	return GDK_FILTER_CONTINUE;
}


/* on_screen_changed */
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data)
{
	Pager * pager = data;
	GdkEventMask events;
	size_t i;

	if(pager->screen == NULL)
	{
		pager->screen = gtk_widget_get_screen(widget);
		pager->display = gdk_screen_get_display(pager->screen);
		pager->root = gdk_screen_get_root_window(pager->screen);
		events = gdk_window_get_events(pager->root);
		gdk_window_set_events(pager->root, events
				| GDK_PROPERTY_CHANGE_MASK);
		gdk_window_add_filter(pager->root, _on_filter, pager);
		/* atoms */
		for(i = 0; i < PAGER_ATOM_COUNT; i++)
			pager->atom[i] = gdk_x11_get_xatom_by_name_for_display(
					pager->display, _pager_atom[i]);
	}
	_pager_do(pager);
}
