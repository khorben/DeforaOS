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
	TASKS_ATOM_NET_ACTIVE_WINDOW = 0,
	TASKS_ATOM_NET_CLIENT_LIST,
	TASKS_ATOM_NET_WM_ICON,
	TASKS_ATOM_NET_WM_NAME,
	TASKS_ATOM_NET_WM_VISIBLE_NAME,
	TASKS_ATOM_NET_WM_WINDOW_TYPE,
	TASKS_ATOM_NET_WM_WINDOW_TYPE_NORMAL,
	TASKS_ATOM_UTF8_STRING
} TasksAtom;
#define TASKS_ATOM_LAST TASKS_ATOM_UTF8_STRING
#define TASKS_ATOM_COUNT (TASKS_ATOM_LAST + 1)

typedef struct _Tasks Tasks;

typedef struct _Task
{
	Tasks * tasks;
	Window window;
	GtkWidget * widget;
	gboolean delete;
} Task;

struct _Tasks
{
	Task ** tasks;
	size_t tasks_cnt;

	GtkWidget * hbox;
	int icon_width;
	int icon_height;

	Atom atom[TASKS_ATOM_COUNT];
	GdkDisplay * display;
	GdkScreen * screen;
	GdkWindow * root;
};


/* constants */
static const char * _tasks_atom[TASKS_ATOM_COUNT] =
{
	"_NET_ACTIVE_WINDOW",
	"_NET_CLIENT_LIST",
	"_NET_WM_ICON",
	"_NET_WM_NAME",
	"_NET_WM_VISIBLE_NAME",
	"_NET_WM_WINDOW_TYPE",
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"UTF8_STRING"
};


/* prototypes */
/* task */
static Task * _task_new(Tasks * tasks, Window window, char const * name,
		GdkPixbuf * pixbuf);
static void _task_delete(Task * Task);
static void _task_set(Task * task, char const * name, GdkPixbuf * pixbuf);

/* tasks */
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
static void _on_clicked(GtkWidget * widget, gpointer data);
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
	TRUE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* Task */
/* task_new */
static Task * _task_new(Tasks * tasks, Window window, char const * name,
		GdkPixbuf * pixbuf)
{
	Task * task;

	if((task = malloc(sizeof(*task))) == NULL)
		return NULL;
	task->tasks = tasks;
	task->window = window;
	task->widget = gtk_button_new();
	task->delete = FALSE;
#ifndef EMBEDDED
	gtk_button_set_alignment(GTK_BUTTON(task->widget), 0.0, 0.5);
	gtk_widget_set_size_request(task->widget, tasks->icon_width, -1);
#endif
	g_signal_connect(task->widget, "clicked", G_CALLBACK(_on_clicked),
			task);
	_task_set(task, name, pixbuf);
	return task;
}


/* task_delete */
static void _task_delete(Task * task)
{
	gtk_container_remove(GTK_CONTAINER(task->tasks->hbox), task->widget);
	free(task);
}


/* task_set */
static void _task_set(Task * task, char const * name, GdkPixbuf * pixbuf)
{
	GtkWidget * image;

#ifndef EMBEDDED
	gtk_button_set_label(task->widget, name);
#endif
	gtk_widget_set_tooltip_text(task->widget, name);
	if((image = gtk_button_get_image(GTK_BUTTON(task->widget))) == NULL)
	{
		if(pixbuf != NULL)
			image = gtk_image_new_from_pixbuf(pixbuf);
		else
			image = gtk_image_new_from_stock(
					GTK_STOCK_MISSING_IMAGE,
					GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_button_set_image(GTK_BUTTON(task->widget), image);
	}
	else if(pixbuf != NULL)
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
	else
		gtk_image_set_from_stock(GTK_IMAGE(image),
				GTK_STOCK_MISSING_IMAGE,
				GTK_ICON_SIZE_LARGE_TOOLBAR);
}


/* Tasks */
/* tasks_init */
static GtkWidget * _tasks_init(PanelApplet * applet)
{
	GtkWidget * ret;
	Tasks * tasks;

	if((tasks = malloc(sizeof(*tasks))) == NULL)
		return NULL;
	applet->priv = tasks;
	tasks->tasks = NULL;
	tasks->tasks_cnt = 0;
	tasks->hbox = gtk_hbox_new(TRUE, 0);
	g_signal_connect(G_OBJECT(tasks->hbox), "screen-changed", G_CALLBACK(
				_on_screen_changed), tasks);
	tasks->icon_width = 48;
	tasks->icon_height = 48;
	gtk_icon_size_lookup(GTK_ICON_SIZE_LARGE_TOOLBAR, &tasks->icon_width,
			&tasks->icon_height);
	tasks->display = NULL;
	tasks->screen = NULL;
	tasks->root = NULL;
	ret = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ret),
			GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(ret),
			tasks->hbox);
	return ret;
}


/* tasks_destroy */
static void _tasks_destroy(PanelApplet * applet)
{
	Tasks * tasks = applet->priv;
	size_t i;

	for(i = 0; i < tasks->tasks_cnt; i++)
		free(tasks->tasks[i]);
	free(tasks->tasks);
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
	gdk_error_trap_push();
	res = XGetTextProperty(GDK_DISPLAY_XDISPLAY(tasks->display), window,
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
	gdk_error_trap_push();
	res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(tasks->display), window,
			tasks->atom[property], 0, G_MAXLONG, False, atom,
			&type, &format, cnt, &bytes, ret);
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


/* tasks_do */
static char * _do_name(Tasks * tasks, Window window);
static char * _do_name_text(Tasks * tasks, Window window, Atom property);
static char * _do_name_utf8(Tasks * tasks, Window window, Atom property);
static GdkPixbuf * _do_pixbuf(Tasks * tasks, Window window);
static int _do_tasks_add(Tasks * tasks, Window window, char const * name,
		GdkPixbuf * pixbuf);
static void _do_tasks_clean(Tasks * tasks);
static int _do_typehint_normal(Tasks * tasks, Window window);

static void _tasks_do(Tasks * tasks)
{
	unsigned long cnt = 0;
	Window * windows = NULL;
	unsigned long i;
	char * name;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(_tasks_get_window_property(tasks, GDK_WINDOW_XWINDOW(tasks->root),
				TASKS_ATOM_NET_CLIENT_LIST,
				XA_WINDOW, &cnt, (void*)&windows) != 0)
		return;
	for(i = 0; i < tasks->tasks_cnt; i++)
		tasks->tasks[i]->delete = TRUE;
	for(i = 0; i < cnt; i++)
	{
		if(_do_typehint_normal(tasks, windows[i]) != 0)
			continue;
		if((name = _do_name(tasks, windows[i])) == NULL)
			continue;
		_do_tasks_add(tasks, windows[i], name, _do_pixbuf(tasks,
					windows[i]));
		g_free(name);
	}
	_do_tasks_clean(tasks);
	XFree(windows);
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
	GdkPixbuf * ret;
	unsigned long cnt = 0;
	unsigned long * buf = NULL;
	unsigned long i;
	unsigned long width;
	unsigned long height;
	unsigned long size;
	unsigned char * pixbuf;
	unsigned long j;
	GdkPixbuf * p;

	if(_tasks_get_window_property(tasks, window, TASKS_ATOM_NET_WM_ICON,
				XA_CARDINAL, &cnt, (void*)&buf) != 0)
		return NULL;
	for(i = 0; i < cnt - 3; i++)
	{
		width = buf[i];
		height = buf[i + 1];
		if(i + 2 + (width * height) > cnt)
			break;
		if(width == 0 || height == 0 || width != height)
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
		p = gdk_pixbuf_new_from_data(pixbuf, GDK_COLORSPACE_RGB,
				TRUE, 8, width, height, width * 4,
				(GdkPixbufDestroyNotify)free, NULL);
		XFree(buf);
		if(width == tasks->icon_width)
			return p;
		ret = gdk_pixbuf_scale_simple(p, tasks->icon_width,
				tasks->icon_height,
				GDK_INTERP_BILINEAR);
		g_object_unref(p);
		return ret;
	}
	return NULL;
}

static int _do_tasks_add(Tasks * tasks, Window window, char const * name,
		GdkPixbuf * pixbuf)
{
	size_t i;
	Task * p = NULL;
	Task ** q;

	for(i = 0; i < tasks->tasks_cnt; i++)
		if(tasks->tasks[i]->window == window)
			break;
	if(i < tasks->tasks_cnt)
	{
		p = tasks->tasks[i];
#ifndef EMBEDDED
		gtk_button_set_label(GTK_BUTTON(p->widget), name);
#endif
		_task_set(p, name, pixbuf);
		p->delete = FALSE;
	}	
	else
	{
		if((q = realloc(tasks->tasks, (tasks->tasks_cnt + 1)
						* sizeof(*q))) == NULL)
			return 1;
		tasks->tasks = q;
		if((p = _task_new(tasks, window, name, pixbuf)) == NULL)
			return 1;
		tasks->tasks[tasks->tasks_cnt++] = p;
		gtk_widget_show_all(p->widget);
		gtk_box_pack_start(GTK_BOX(tasks->hbox), p->widget, FALSE, TRUE,
				0);
	}
	return 0;
}

static void _do_tasks_clean(Tasks * tasks)
{
	size_t i;
	size_t cnt;
	size_t j;
	Task ** q;

	for(i = 0, cnt = tasks->tasks_cnt; i < cnt;)
	{
		if(tasks->tasks[i]->delete == FALSE)
		{
			i++;
			continue;
		}
		_task_delete(tasks->tasks[i]);
		cnt--;
		for(j = i; j < cnt; j++)
			tasks->tasks[j] = tasks->tasks[j + 1];
		if((q = realloc(tasks->tasks, cnt * sizeof(*q))) != NULL
				|| cnt == 0)
			tasks->tasks = q;
	}
	tasks->tasks_cnt = cnt;
}

static int _do_typehint_normal(Tasks * tasks, Window window)
{
	Atom typehint;
	Atom * p;
	unsigned long cnt = 0;

	if(_tasks_get_window_property(tasks, window,
				TASKS_ATOM_NET_WM_WINDOW_TYPE, XA_ATOM, &cnt,
				(void*)&p) == 0)
	{
		typehint = *p;
		XFree(p);
		return typehint == tasks->atom[
			TASKS_ATOM_NET_WM_WINDOW_TYPE_NORMAL] ? 0 : 1;
	}
	/* FIXME return 1 if WM_TRANSIENT_FOR is set */
	return 0;
}


/* callbacks */
/* on_clicked */
static void _clicked_activate(Task * task);

static void _on_clicked(GtkWidget * widget, gpointer data)
{
	Task * task = data;

	_clicked_activate(task);
#ifdef EMBEDDED
	gtk_box_reorder_child(GTK_BOX(task->tasks->hbox), widget, 0);
#endif
}

static void _clicked_activate(Task * task)
{
	GdkDisplay * display;
	XEvent xev;

	display = task->tasks->display;
	xev.xclient.type = ClientMessage;
	xev.xclient.window = task->window;
	xev.xclient.message_type = task->tasks->atom[
		TASKS_ATOM_NET_ACTIVE_WINDOW];
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 2;
	xev.xclient.data.l[1] = gdk_x11_display_get_user_time(display);
	xev.xclient.data.l[2] = GDK_WINDOW_XWINDOW(task->tasks->root);
	gdk_error_trap_push();
	XSendEvent(GDK_DISPLAY_XDISPLAY(display),
			GDK_WINDOW_XWINDOW(task->tasks->root), False,
			SubstructureNotifyMask | SubstructureRedirectMask,
			&xev);
	gdk_error_trap_pop();
}


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
