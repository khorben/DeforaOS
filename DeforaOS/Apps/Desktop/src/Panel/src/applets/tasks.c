/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include "panel.h"
#include "../../config.h"


/* Tasks */
/* private */
/* types */
typedef enum _TasksAtom
{
	TASKS_ATOM_NET_CLIENT_LIST = 0,
	TASKS_ATOM_NET_WM_ICON,
	TASKS_ATOM_NET_WM_NAME,
	TASKS_ATOM_NET_WM_VISIBLE_NAME,
	TASKS_ATOM_UTF8_STRING
} TasksAtom;
#define TASKS_ATOM_LAST TASKS_ATOM_UTF8_STRING
#define TASKS_ATOM_COUNT (TASKS_ATOM_LAST + 1)

typedef struct _Tasks
{
	GtkWidget * hbox;

	Atom atom[TASKS_ATOM_COUNT];
	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;
} Tasks;


/* constants */
static const char * _tasks_atom[TASKS_ATOM_COUNT] =
{
	"_NET_CLIENT_LIST",
	"_NET_WM_ICON",
	"_NET_WM_NAME",
	"_NET_WM_VISIBLE_NAME",
	"UTF8_STRING"
};


/* prototypes */
static GtkWidget * _tasks_init(PanelApplet * applet);
static void _tasks_destroy(PanelApplet * applet);

/* accessors */
static int _tasks_get_text_property(Tasks * tasks, Window window, Atom property,
		char ** ret);
static int _tasks_get_window_property(Tasks * tasks, Window window,
		TasksAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret);

/* useful */
static void _tasks_do(Tasks * tasks);

/* callbacks */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
void _on_screen_changed(GtkWidget * widget, GdkScreen * previous,
		gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_tasks_init,
	_tasks_destroy,
	PANEL_APPLET_POSITION_START,
	NULL
};


/* private */
/* functions */
/* tasks_init */
static GtkWidget * _tasks_init(PanelApplet * applet)
{
	Tasks * tasks;

	if((tasks = malloc(sizeof(*tasks))) == NULL)
		return NULL;
	applet->priv = tasks;
	tasks->hbox = gtk_hbox_new(FALSE, 0);
	g_signal_connect(G_OBJECT(tasks->hbox), "screen-changed", G_CALLBACK(
				_on_screen_changed), tasks);
	tasks->display = NULL;
	tasks->screen = NULL;
	tasks->root = NULL;
	return tasks->hbox;
}


/* tasks_destroy */
static void _tasks_destroy(PanelApplet * applet)
{
	Tasks * tasks = applet->priv;

	free(tasks);
}


/* accessors */
/* tasks_get_text_property */
static int _tasks_get_text_property(Tasks * tasks, Window window,
		Atom property, char ** ret)
{
	int res;
	XTextProperty text;
	int cnt;
	char ** list;
	int i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(tasks, window, %lu)\n", __func__, property);
#endif
	res = XGetTextProperty(GDK_DISPLAY_XDISPLAY(tasks->display), window,
			&text, property);
	if(res != True) /* XXX why is it not Success? */
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


/* tasks_get_window_property */
static int _tasks_get_window_property(Tasks * tasks, Window window,
		TasksAtom property, Atom atom, unsigned long * cnt,
		unsigned char ** ret)
{
	int res;
	Atom type;
	int format;
	unsigned long bytes;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(tasks, window, %s, %lu)\n", __func__,
			_tasks_atom[property], atom);
#endif
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(tasks->display), window,
			tasks->atom[property], 0, G_MAXLONG, False, atom,
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


/* tasks_do */
static char * _do_name(Tasks * tasks, Window window);
static char * _do_name_text(Tasks * tasks, Window window, Atom property);
static char * _do_name_utf8(Tasks * tasks, Window window, Atom property);
static GdkPixbuf * _do_pixbuf(Tasks * tasks, Window window);

static void _tasks_do(Tasks * tasks)
{
	unsigned long cnt = 0;
	Window * windows = NULL;
	unsigned long i;
	GdkWindow * window;
	char * name;
	GtkWidget * widget;
	GdkPixbuf * pixbuf;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(_tasks_get_window_property(tasks, GDK_WINDOW_XWINDOW(tasks->root),
				TASKS_ATOM_NET_CLIENT_LIST,
				XA_WINDOW, &cnt, (void*)&windows)
			!= 0)
		return;
	for(i = 0; i < cnt; i++)
	{
		if((window = gdk_window_foreign_new_for_display(tasks->display,
						windows[i])) == NULL)
			continue;
		if(gdk_window_get_type_hint(window)
				!= GDK_WINDOW_TYPE_HINT_NORMAL)
			continue;
		if((name = _do_name(tasks, windows[i])) == NULL)
			continue;
		widget = gtk_button_new_with_label(name);
		if((pixbuf = _do_pixbuf(tasks, windows[i])) != NULL)
			gtk_button_set_image(GTK_BUTTON(widget),
					gtk_image_new_from_pixbuf(pixbuf));
		gtk_widget_set_size_request(widget, 100, -1);
		gtk_widget_show_all(widget);
		gtk_box_pack_start(GTK_BOX(tasks->hbox), widget, FALSE, TRUE,
				2);
		g_free(name);
	}
}

static char * _do_name(Tasks * tasks, Window window)
{
	char * ret;

	if((ret = _do_name_utf8(tasks, window, TASKS_ATOM_NET_WM_VISIBLE_NAME))
			!= NULL)
		return ret;
	if((ret = _do_name_utf8(tasks, window, TASKS_ATOM_NET_WM_NAME)) != NULL)
		return ret;
	if((ret = _do_name_text(tasks, window, XA_WM_NAME)) != NULL)
		return ret;
	return g_strdup("(Untitled)");
}

static char * _do_name_text(Tasks * tasks, Window window, Atom property)
{
	char * ret = NULL;

	if(_tasks_get_text_property(tasks, window, property, (void*)&ret) != 0)
		return NULL;
	/* FIXME convert to UTF-8 */
	return ret;
}

static char * _do_name_utf8(Tasks * tasks, Window window, Atom property)
{
	char * ret = NULL;
	char * str = NULL;
	unsigned long cnt = 0;

	if(_tasks_get_window_property(tasks, window, property,
				tasks->atom[TASKS_ATOM_UTF8_STRING], &cnt,
				(void*)&str) != 0)
		return NULL;
	if(g_utf8_validate(str, cnt, NULL))
		ret = g_strndup(str, cnt);
	XFree(str);
	return ret;
}

static GdkPixbuf * _do_pixbuf(Tasks * tasks, Window window)
{
	unsigned long cnt = 0;
	unsigned long * buf = NULL;
	unsigned long i;
	unsigned long width;
	unsigned long height;
	unsigned long size;
	unsigned char * pixbuf;
	unsigned long j;

	if(_tasks_get_window_property(tasks, window, TASKS_ATOM_NET_WM_ICON,
				XA_CARDINAL, &cnt, (void*)&buf) != 0)
		return NULL;
	for(i = 0; i < cnt - 3; i++)
	{
		width = buf[i];
		height = buf[i + 1];
		if(i + 2 + (width * height) > cnt)
			break;
		if(width != 48 || height != 48)
			continue;
		size = width * height * 4;
		if((pixbuf = malloc(size)) == NULL)
			return NULL;
		for(i+=2, j = 0; j < size; i++)
		{
			pixbuf[j++] = (buf[i] >> 16) & 0xff; /* red */
			pixbuf[j++] = (buf[i] >> 8) & 0xff; /* green */
			pixbuf[j++] = buf[i] & 0xff; /* blue */
			pixbuf[j++] = buf[i] >> 24; /* alpha */
		}
		return gdk_pixbuf_new_from_data(pixbuf, GDK_COLORSPACE_RGB,
				TRUE, 8, width, height, width * 4,
				(GdkPixbufDestroyNotify)free, NULL);
	}
	return NULL;
}


/* callbacks */
/* on_filter */
static GdkFilterReturn _on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Tasks * tasks = data;
	XEvent * xev = xevent;

	if(xev->type != PropertyNotify)
		return GDK_FILTER_CONTINUE;
	if(xev->xproperty.atom != tasks->atom[TASKS_ATOM_NET_CLIENT_LIST])
		return GDK_FILTER_CONTINUE;
	_tasks_do(tasks);
	return GDK_FILTER_CONTINUE;
}


/* on_screen_changed */
void _on_screen_changed(GtkWidget * widget, GdkScreen * previous, gpointer data)
{
	Tasks * tasks = data;
	GdkEventMask events;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	tasks->screen = gtk_widget_get_screen(widget);
	tasks->display = gdk_screen_get_display(tasks->screen);
	tasks->root = gdk_screen_get_root_window(tasks->screen);
	events = gdk_window_get_events(tasks->root);
	gdk_window_set_events(tasks->root, events
			| GDK_PROPERTY_CHANGE_MASK);
	gdk_window_add_filter(tasks->root, _on_filter, tasks);
	/* atoms */
	for(i = 0; i < TASKS_ATOM_COUNT; i++)
		tasks->atom[i] = gdk_x11_get_xatom_by_name_for_display(
				tasks->display, _tasks_atom[i]);
	_tasks_do(tasks);
}
