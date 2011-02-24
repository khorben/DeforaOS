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
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <System.h>
#include "locker.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* Locker */
/* private */
/* types */
struct _Locker
{
	GdkDisplay * display;
	int screen;
	int event;
	GtkWidget * window;
	GtkWidget * unlock;
	GtkWidget * scale;
	guint source;
};


/* prototypes */
static void _locker_lock(Locker * locker);
static void _locker_unlock(Locker * locker);
static void _locker_unlock_dialog(Locker * locker);


/* public */
/* functions */
/* locker_new */
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Locker * locker_new(void)
{
	Locker * locker;
	GdkScreen * screen;
	int error;
	Window root;

	if((locker = object_new(sizeof(*locker))) == NULL)
		return NULL;
	screen = gdk_screen_get_default();
	locker->display = gdk_screen_get_display(screen);
	locker->screen = gdk_x11_get_default_screen();
	locker->window = NULL;
	locker->source = 0;
	if(XScreenSaverQueryExtension(GDK_DISPLAY_XDISPLAY(locker->display),
				&locker->event, &error) == 0
			|| XScreenSaverRegister(
				GDK_DISPLAY_XDISPLAY(locker->display),
				locker->screen, getpid(), XA_INTEGER) == 0)
	{
		object_delete(locker);
		return NULL;
	}
	root = gdk_x11_get_default_root_xwindow();
	XScreenSaverSelectInput(GDK_DISPLAY_XDISPLAY(locker->display), root,
			ScreenSaverNotifyMask);
	gdk_x11_register_standard_event_type(locker->display, locker->event, 1);
	gdk_window_add_filter(gdk_get_default_root_window(), _locker_on_filter,
			locker);
	return locker;
}

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
			_locker_unlock_dialog(locker);
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


/* locker_delete */
void locker_delete(Locker * locker)
{
	if(locker->source != 0)
		g_source_remove(locker->source);
	XScreenSaverUnregister(GDK_DISPLAY_XDISPLAY(locker->display),
			locker->screen);
	object_delete(locker);
}


/* private */
/* functions */
/* useful */
/* locker_lock */
static gboolean _lock_on_closex(void);
static gboolean _lock_on_timeout(gpointer data);
static void _lock_on_value_changed(gpointer data);
static gboolean _lock_on_value_timeout(gpointer data);

static void _locker_lock(Locker * locker)
{
	GdkColor black;
	GtkWidget * hbox;
	GtkWidget * widget;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(locker->window != NULL)
	{
		gtk_widget_hide(locker->unlock);
		gtk_range_set_value(GTK_RANGE(locker->scale), 0.0);
		gtk_widget_show(locker->window);
		return;
	}
	locker->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(locker->window), 4);
	memset(&black, 0, sizeof(black));
	gtk_widget_modify_bg(locker->window, GTK_STATE_NORMAL, &black);
	gtk_window_fullscreen(GTK_WINDOW(locker->window));
	gtk_window_set_keep_above(GTK_WINDOW(locker->window), TRUE);
	g_signal_connect_swapped(G_OBJECT(locker->window), "delete-event",
			G_CALLBACK(_lock_on_closex), NULL);
	/* FIXME implement a plug-in system instead */
	locker->unlock = gtk_vbox_new(FALSE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_image_new_from_icon_name("stock_lock",
			GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_misc_set_alignment(GTK_MISC(widget), 1.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(widget), 0, 96);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	locker->scale = gtk_hscale_new_with_range(0.0, 100.0, 0.1);
	gtk_range_set_value(GTK_RANGE(locker->scale), 0.0);
	gtk_scale_set_draw_value(GTK_SCALE(locker->scale), FALSE);
	gtk_widget_set_size_request(locker->scale, 400, -1);
	g_signal_connect_swapped(G_OBJECT(locker->scale), "value-changed",
			G_CALLBACK(_lock_on_value_changed), locker);
	gtk_box_pack_start(GTK_BOX(hbox), locker->scale, FALSE, TRUE, 0);
	widget = gtk_image_new_from_icon_name("stock_lock-open",
			GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_misc_set_padding(GTK_MISC(widget), 0, 96);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_widget_show_all(hbox);
	gtk_box_pack_end(GTK_BOX(locker->unlock), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(locker->window), locker->unlock);
	if(locker->source != 0)
		g_source_remove(locker->source);
	locker->source = 0;
	gtk_widget_show(locker->window);
}

static gboolean _lock_on_closex(void)
{
	return TRUE;
}

static gboolean _lock_on_timeout(gpointer data)
{
	Locker * locker = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	locker->source = 0;
#if 0 /* FIXME makes it currently impossible to unlock the screen again */
	_locker_lock(locker);
#endif
	return FALSE;
}

static void _lock_on_value_changed(gpointer data)
{
	Locker * locker = data;
	gdouble value;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(locker->source != 0)
		g_source_remove(locker->source);
	locker->source = 0;
	value = gtk_range_get_value(GTK_RANGE(locker->scale));
	if(value == 100.0)
		_locker_unlock(locker);
	else if(value != 0.0)
		locker->source = g_timeout_add(1000, _lock_on_value_timeout,
				locker);
}

static gboolean _lock_on_value_timeout(gpointer data)
{
	Locker * locker = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_range_set_value(GTK_RANGE(locker->scale), 0.0);
	locker->source = g_timeout_add(3000, _lock_on_timeout, locker);
	return FALSE;
}


/* locker_unlock */
static void _locker_unlock(Locker * locker)
{
	if(locker->window == NULL)
		return;
	gtk_widget_hide(locker->window);
}


/* locker_unlock_dialog */
static gboolean _unlock_dialog_on_timeout(gpointer data);

static void _locker_unlock_dialog(Locker * locker)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(locker->window == NULL)
		return;
	gtk_widget_show(locker->unlock);
	if(locker->source != 0)
		g_source_remove(locker->source);
	locker->source = g_timeout_add(3000, _unlock_dialog_on_timeout,
			locker);
}

static gboolean _unlock_dialog_on_timeout(gpointer data)
{
	Locker * locker = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	locker->source = 0;
#if 0 /* FIXME makes it currently impossible to unlock the screen again */
	_locker_lock(locker);
#endif
	return FALSE;
}
