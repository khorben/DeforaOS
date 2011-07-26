/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <System.h>
#include "locker.h"
#include "../config.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* Locker */
/* private */
/* types */
struct _Locker
{
	LockerPluginHelper helper;

	/* preferences */
	int suspend;

	/* internal */
	GdkDisplay * display;
	int screen;
	int event;
	GtkWidget ** windows;
	size_t windows_cnt;

#if 0
	GtkWidget * unlock;
	GtkWidget * scale;
	guint source;
#else
	LockerPlugin * plugin;
#endif
};


/* prototypes */
static void _locker_action(Locker * locker, LockerAction action);

static void _locker_activate(Locker * locker);
static int _locker_error(Locker * locker, char const * message, int ret);
static void _locker_lock(Locker * locker);
static void _locker_unlock(Locker * locker);

/* callbacks */
static gboolean _lock_on_closex(void);
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);


/* public */
/* functions */
/* locker_new */
Locker * locker_new(int suspend, char const * name)
{
	Locker * locker;
	LockerPlugin * plugin;
	GdkScreen * screen;
	int error;
	GtkWidget * widget = NULL;
	size_t cnt;
	size_t i;
	GdkColor black;
	GdkRectangle rect;
	GdkWindow * root;

	if(name == NULL)
		name = "slider";
	if((locker = object_new(sizeof(*locker))) == NULL)
		return NULL;
	locker->helper.locker = locker;
	locker->helper.error = _locker_error;
	locker->helper.action = _locker_action;
	locker->suspend = (suspend != 0) ? 1 : 0;
	screen = gdk_screen_get_default();
	locker->display = gdk_screen_get_display(screen);
	locker->screen = gdk_x11_get_default_screen();
	cnt = gdk_screen_get_n_monitors(screen);
	locker->windows = NULL;
	locker->windows_cnt = cnt;
	if((plugin = plugin_new(LIBDIR, PACKAGE, "plugins", name)) != NULL
			&& (locker->plugin = plugin_lookup(plugin, "plugin"))
			!= NULL)
	{
		locker->plugin->helper = &locker->helper;
		widget = locker->plugin->init(locker->plugin);
	}
	if(widget == NULL || XScreenSaverQueryExtension(GDK_DISPLAY_XDISPLAY(
					locker->display), &locker->event,
				&error) == 0
			|| XScreenSaverRegister(
				GDK_DISPLAY_XDISPLAY(locker->display),
				locker->screen, getpid(), XA_INTEGER) == 0
			|| (locker->windows = malloc(sizeof(*locker->windows)
					* cnt)) == NULL)
	{
		locker_delete(locker);
		return NULL;
	}
	/* create windows */
	memset(&black, 0, sizeof(black));
	for(i = 0; i < locker->windows_cnt; i++)
	{
		locker->windows[i] = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gdk_screen_get_monitor_geometry(screen, i, &rect);
		gtk_window_move(GTK_WINDOW(locker->windows[i]), rect.x, rect.y);
		gtk_window_resize(GTK_WINDOW(locker->windows[i]), rect.width,
				rect.height);
		gtk_window_stick(GTK_WINDOW(locker->windows[i]));
		gtk_widget_modify_bg(locker->windows[i], GTK_STATE_NORMAL,
				&black);
		gtk_window_set_keep_above(GTK_WINDOW(locker->windows[i]), TRUE);
		g_signal_connect_swapped(G_OBJECT(locker->windows[i]),
				"delete-event", G_CALLBACK(_lock_on_closex),
				NULL);
	}
	gtk_container_set_border_width(GTK_CONTAINER(locker->windows[0]), 4);
	gtk_container_add(GTK_CONTAINER(locker->windows[0]), widget);
	root = gdk_get_default_root_window();
	XScreenSaverSelectInput(GDK_DISPLAY_XDISPLAY(locker->display),
			GDK_WINDOW_XWINDOW(root), ScreenSaverNotifyMask);
	gdk_x11_register_standard_event_type(locker->display, locker->event, 1);
	gdk_window_add_filter(root, _locker_on_filter, locker);
	return locker;
}


/* locker_delete */
void locker_delete(Locker * locker)
{
#if 0
	if(locker->source != 0)
		g_source_remove(locker->source);
#endif
	free(locker->windows);
	XScreenSaverUnregister(GDK_DISPLAY_XDISPLAY(locker->display),
			locker->screen);
	object_delete(locker);
}


/* private */
/* functions */
/* useful */
/* locker_action */
static void _locker_action(Locker * locker, LockerAction action)
{
	switch(action)
	{
		case LOCKER_ACTION_ACTIVATE:
			_locker_activate(locker);
			break;
		case LOCKER_ACTION_LOCK:
			_locker_lock(locker);
			break;
		case LOCKER_ACTION_UNLOCK:
			_locker_unlock(locker);
			break;
	}
	locker->plugin->action(locker->plugin, LOCKER_ACTION_ACTIVATE);
}


/* locker_activate */
static void _locker_activate(Locker * locker)
{
	XActivateScreenSaver(GDK_DISPLAY_XDISPLAY(locker->display));
}


/* locker_error */
static int _locker_error(Locker * locker, char const * message, int ret)
{
	if(locker == NULL)
		fprintf(stderr, "%s: %s\n", "locker", message);
	/* XXX otherwise popup an error dialog */
	return ret;
}


/* locker_lock */
static void _locker_lock(Locker * locker)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(i = 0; i < locker->windows_cnt; i++)
	{
		gtk_widget_show(locker->windows[i]);
		gtk_window_fullscreen(GTK_WINDOW(locker->windows[i]));
	}
	locker->plugin->action(locker->plugin, LOCKER_ACTION_LOCK);
}


/* locker_unlock */
static void _locker_unlock(Locker * locker)
{
	size_t i;

	if(locker->windows == NULL)
		return;
	for(i = 0; i < locker->windows_cnt; i++)
		gtk_widget_hide(locker->windows[i]);
}


/* callbacks */
/* locker_on_closex */
static gboolean _lock_on_closex(void)
{
	return TRUE;
}


/* locker_on_filter */
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Locker * locker = data;
	XScreenSaverNotifyEvent * xssne = xevent;

	if(xssne->type != locker->event)
		return GDK_FILTER_CONTINUE;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() 0x%x\n", __func__, event->type);
#endif
	switch(xssne->state)
	{
		case ScreenSaverOff:
			/* FIXME anything to do here? */
			break;
		case ScreenSaverOn:
			_locker_lock(locker);
			break;
		case ScreenSaverDisabled:
			/* FIXME implement */
			break;
	}
	return GDK_FILTER_CONTINUE;
}
