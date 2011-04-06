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



#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sysctl.h>
#else
# include <fcntl.h>
#endif
#include <unistd.h>
#include <stdlib.h>
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
	/* preferences */
	int suspend;

	/* internal */
	GdkDisplay * display;
	int screen;
	int event;
	GtkWidget ** windows;
	size_t windows_cnt;
	GtkWidget * unlock;
	GtkWidget * scale;
	guint source;
};


/* prototypes */
static void _locker_activate(Locker * locker);
static int _locker_error(Locker * locker, char const * message, int ret);
static void _locker_lock(Locker * locker);
static void _locker_unlock(Locker * locker);
static void _locker_unlock_dialog(Locker * locker);


/* public */
/* functions */
/* locker_new */
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Locker * locker_new(int suspend)
{
	Locker * locker;
	GdkScreen * screen;
	int error;
	Window root;

	if((locker = object_new(sizeof(*locker))) == NULL)
		return NULL;
	locker->suspend = (suspend != 0) ? 1 : 0;
	screen = gdk_screen_get_default();
	locker->display = gdk_screen_get_display(screen);
	locker->screen = gdk_x11_get_default_screen();
	locker->windows = NULL;
	locker->windows_cnt = 0;
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
	free(locker->windows);
	XScreenSaverUnregister(GDK_DISPLAY_XDISPLAY(locker->display),
			locker->screen);
	object_delete(locker);
}


/* private */
/* functions */
/* useful */
/* locker_activate */
static gboolean _activate_on_timeout(gpointer data);

static void _locker_activate(Locker * locker)
{
	if(locker->source != 0)
		g_source_remove(locker->source);
	XActivateScreenSaver(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()));
	if(locker->suspend != 0)
		locker->source = g_timeout_add(10000, _activate_on_timeout,
				locker);
	else
		locker->source = 0;
}

static gboolean _activate_on_timeout(gpointer data)
{
	Locker * locker = data;
	/* XXX this code is duplicated from DeforaOS' Desktop Panel */
#ifdef __NetBSD__
	int sleep_state = 3;
#else
	int fd;
	char * suspend[] = { "/usr/bin/sudo", "sudo", "/usr/bin/apm", "-s",
		NULL };
	GError * error = NULL;
#endif

	locker->source = 0;
#ifdef __NetBSD__
	if(sysctlbyname("machdep.sleep_state", NULL, NULL, &sleep_state,
				sizeof(sleep_state)) != 0)
		_locker_error(NULL, "sysctl", 1);
#else
	if((fd = open("/sys/power/state", O_WRONLY)) >= 0)
	{
		write(fd, "mem\n", 4);
		close(fd);
	}
	else if(g_spawn_async(NULL, suspend, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, NULL, &error) != TRUE)
		_locker_error(NULL, error->message, 1);
#endif
	return FALSE;
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
static gboolean _lock_on_closex(void);
static gboolean _lock_on_timeout(gpointer data);
static void _lock_on_value_changed(gpointer data);
static gboolean _lock_on_value_timeout(gpointer data);

static void _locker_lock(Locker * locker)
{
	size_t i;
	GdkDisplay * display;
	GdkScreen * screen;
	GdkColor black;
	size_t cnt;
	GdkRectangle rect;
	GtkWidget * hbox;
	GtkWidget * widget;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(locker->source != 0)
		g_source_remove(locker->source);
	locker->source = 0;
	if(locker->windows != NULL)
	{
		gtk_widget_hide(locker->unlock);
		gtk_range_set_value(GTK_RANGE(locker->scale), 0.0);
		for(i = 0; i < locker->windows_cnt; i++)
		{
			gtk_widget_show(locker->windows[i]);
			gtk_window_fullscreen(GTK_WINDOW(locker->windows[i]));
		}
		return;
	}
	display = gdk_display_get_default();
	screen = gdk_display_get_default_screen(display);
	memset(&black, 0, sizeof(black));
	cnt = gdk_screen_get_n_monitors(screen);
	if((locker->windows = malloc(sizeof(*locker->windows) * cnt)) == NULL)
		return; /* XXX report error */
	locker->windows_cnt = cnt;
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
	/* FIXME implement a plug-in system instead */
	gtk_container_set_border_width(GTK_CONTAINER(locker->windows[0]), 4);
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
	gtk_widget_set_size_request(locker->scale, 240, -1);
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
	gtk_container_add(GTK_CONTAINER(locker->windows[0]), locker->unlock);
	for(i = 0; i < locker->windows_cnt; i++)
	{
		gtk_widget_show(locker->windows[i]);
		gtk_window_fullscreen(GTK_WINDOW(locker->windows[i]));
	}
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
	_locker_activate(locker);
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
	size_t i;

	if(locker->windows == NULL)
		return;
	for(i = 0; i < locker->windows_cnt; i++)
		gtk_widget_hide(locker->windows[i]);
}


/* locker_unlock_dialog */
static gboolean _unlock_dialog_on_timeout(gpointer data);

static void _locker_unlock_dialog(Locker * locker)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(locker->windows == NULL)
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
	_locker_activate(locker);
	return FALSE;
}
