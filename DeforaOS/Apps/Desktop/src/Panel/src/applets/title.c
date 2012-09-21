/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "Panel.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Title */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
	GtkWidget * widget;

	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;

	Atom atom_active;
	Atom atom_name;
	Atom atom_utf8_string;
	Atom atom_visible_name;
} Title;


/* prototypes */
/* title */
static Title * _title_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _title_destroy(Title * title);

/* accessors */
static int _title_get_text_property(Title * title, Window window, Atom property,
		char ** ret);

/* useful */
static void _title_do(Title * title);

/* callbacks */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Title",
	NULL,
	NULL,
	_title_init,
	_title_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* title_init */
static Title * _title_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
	Title * title;
	PangoFontDescription * bold;

	if((title = malloc(sizeof(*title))) == NULL)
		return NULL;
	title->helper = helper;
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	title->widget = gtk_label_new("");
	gtk_widget_modify_font(title->widget, bold);
	pango_font_description_free(bold);
	g_signal_connect(G_OBJECT(title->widget), "screen-changed", G_CALLBACK(
				_on_screen_changed), title);
	title->display = NULL;
	title->screen = NULL;
	title->root = NULL;
	title->atom_active = 0;
	title->atom_name = 0;
	title->atom_visible_name = 0;
	gtk_widget_show(title->widget);
	*widget = title->widget;
	return title;
}


/* title_destroy */
static void _title_destroy(Title * title)
{
	free(title);
}


/* accessors */
/* title_get_window_property */
static int _title_get_window_property(Title * title, Window window,
		Atom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(title, window, %lu, %lu)\n", __func__,
			property, atom);
#endif
	gdk_error_trap_push();
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(title->display), window,
			property, 0, G_MAXLONG, False, atom, &type, &format,
			cnt, &bytes, ret);
	if(gdk_error_trap_pop() != 0 || res != Success)
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


/* title_get_text_property */
static int _title_get_text_property(Title * title, Window window, Atom property,
		char ** ret)
{
	int res;
	XTextProperty text;
	int cnt;
	char ** list;
	int i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(title, window, %lu)\n", __func__, property);
#endif
	gdk_error_trap_push();
	res = XGetTextProperty(GDK_DISPLAY_XDISPLAY(title->display), window,
			&text, property);
	if(gdk_error_trap_pop() != 0 || res == 0)
		return 1;
	cnt = gdk_text_property_to_utf8_list(gdk_x11_xatom_to_atom(
				text.encoding), text.format, text.value,
			text.nitems, &list);
	if(cnt > 0)
	{
		*ret = list[0];
		for(i = 1; i < cnt; i++)
			g_free(list[i]);
		g_free(list);
	}
	else
		*ret = NULL;
	if(text.value != NULL)
		XFree(text.value);
	return 0;
}


/* title_do */
static char * _do_name(Title * title, Window window);
static char * _do_name_text(Title * title, Window window, Atom property);
static char * _do_name_utf8(Title * title, Window window, Atom property);

static void _title_do(Title * title)
{
	unsigned long cnt = 0;
	Window * window;
	char * name;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(_title_get_window_property(title, GDK_WINDOW_XID(title->root),
				title->atom_active, XA_WINDOW, &cnt,
				(void*)&window) != 0 || cnt != 1)
	{
		gtk_label_set(GTK_LABEL(title->widget), "");
		return;
	}
	name = _do_name(title, *window);
	XFree(window);
	gtk_label_set(GTK_LABEL(title->widget), (name != NULL) ? name : "");
	free(name);
}

static char * _do_name(Title * title, Window window)
{
	char * ret;

	if((ret = _do_name_utf8(title, window, title->atom_visible_name))
			!= NULL)
		return ret;
	if((ret = _do_name_utf8(title, window, title->atom_name)) != NULL)
		return ret;
	if((ret = _do_name_text(title, window, XA_WM_NAME)) != NULL)
		return ret;
	return g_strdup(_("(Untitled)"));
}

static char * _do_name_text(Title * title, Window window, Atom property)
{
	char * ret = NULL;

	if(_title_get_text_property(title, window, property, (void*)&ret) != 0)
		return NULL;
	return ret;
}

static char * _do_name_utf8(Title * title, Window window, Atom property)
{
	char * ret = NULL;
	char * str = NULL;
	unsigned long cnt = 0;

	if(_title_get_window_property(title, window, property,
				title->atom_utf8_string, &cnt, (void*)&str)
			!= 0)
		return NULL;
	if(g_utf8_validate(str, cnt, NULL))
		ret = g_strndup(str, cnt);
	XFree(str);
	return ret;
}


/* callbacks */
/* on_filter */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Title * title = data;
	XEvent * xev = xevent;

	if(xev->type != PropertyNotify)
		return GDK_FILTER_CONTINUE;
	if(xev->xproperty.atom != title->atom_active)
		return GDK_FILTER_CONTINUE;
	_title_do(title);
	return GDK_FILTER_CONTINUE;
}


/* on_screen_changed */
static void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data)
{
	Title * title = data;
	GdkEventMask events;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	title->screen = gtk_widget_get_screen(widget);
	title->display = gdk_screen_get_display(title->screen);
	title->root = gdk_screen_get_root_window(title->screen);
	events = gdk_window_get_events(title->root);
	gdk_window_set_events(title->root, events
			| GDK_PROPERTY_CHANGE_MASK);
	gdk_window_add_filter(title->root, _on_filter, title);
	title->atom_active = gdk_x11_get_xatom_by_name_for_display(
			title->display, "_NET_ACTIVE_WINDOW");
	title->atom_name = gdk_x11_get_xatom_by_name_for_display(
			title->display, "_NET_WM_NAME");
	title->atom_utf8_string = gdk_x11_get_xatom_by_name_for_display(
			title->display, "UTF8_STRING");
	title->atom_visible_name = gdk_x11_get_xatom_by_name_for_display(
			title->display, "_NET_WM_VISIBLE_NAME");
	_title_do(title);
}
