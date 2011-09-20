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
	/* settings */
	int suspend;
	Config * config;

	/* internal */
	GdkDisplay * display;
	int screen;
	int event;
	GtkWidget ** windows;
	size_t windows_cnt;

	/* demo */
	Plugin * dplugin;
	LockerDemo * demo;
	LockerDemoHelper dhelper;

	/* plugin */
	Plugin * pplugin;
	LockerPlugin * plugin;
	LockerPluginHelper phelper;

	/* preferences */
	GtkWidget * pr_window;
};


/* constants */
#define LOCKER_CONFIG_FILE	".locker"


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
static void _locker_on_realize(GtkWidget * widget, gpointer data);


/* public */
/* functions */
/* locker_new */
static int _new_config(Locker * locker);
static int _new_demo(Locker * locker, char const * demo);
static void _new_helpers(Locker * locker);
static GtkWidget * _new_plugin(Locker * locker, char const * plugin);
static int _new_xss(Locker * locker, size_t cnt);

Locker * locker_new(int suspend, char const * demo, char const * plugin)
{
	Locker * locker;
	GdkScreen * screen;
	GtkWidget * widget = NULL;
	size_t cnt;
	size_t i;
	GdkColor black;
	GdkRectangle rect;
	GdkWindow * root;

	if((locker = object_new(sizeof(*locker))) == NULL)
	{
		_locker_error(NULL, error_get(), 1);
		return NULL;
	}
	_new_helpers(locker);
	locker->suspend = (suspend != 0) ? 1 : 0;
	screen = gdk_screen_get_default();
	locker->display = gdk_screen_get_display(screen);
	locker->screen = gdk_x11_get_default_screen();
	cnt = gdk_screen_get_n_monitors(screen);
	locker->windows = NULL;
	locker->windows_cnt = cnt;
	locker->dplugin = NULL;
	locker->demo = NULL;
	locker->pr_window = NULL;
	/* check for errors */
	if(_new_config(locker) != 0
			|| _new_demo(locker, demo) != 0
			|| (widget = _new_plugin(locker, plugin)) == NULL
			|| _new_xss(locker, cnt) != 0)
	{
		_locker_error(NULL, error_get(), 1);
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
		g_signal_connect(locker->windows[i], "realize", G_CALLBACK(
					_locker_on_realize), locker);
	}
	gtk_container_set_border_width(GTK_CONTAINER(locker->windows[0]), 4);
	gtk_container_add(GTK_CONTAINER(locker->windows[0]), widget);
	root = gdk_get_default_root_window();
	XScreenSaverSelectInput(GDK_DISPLAY_XDISPLAY(locker->display),
			GDK_WINDOW_XWINDOW(root), ScreenSaverNotifyMask);
	gdk_x11_register_standard_event_type(locker->display, locker->event, 1);
	gdk_window_add_filter(root, _locker_on_filter, locker);
	gdk_display_add_client_message_filter(locker->display, gdk_atom_intern(
				LOCKER_CLIENT_MESSAGE, FALSE),
			_locker_on_filter, locker);
	return locker;
}

static int _new_config(Locker * locker)
{
	char const * homedir;
	char * filename;

	if((locker->config = config_new()) == NULL)
		return -1;
	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	if((filename = malloc(strlen(homedir) + sizeof(LOCKER_CONFIG_FILE) + 1))
			== NULL)
		return -1;
	sprintf(filename, "%s/%s", homedir, LOCKER_CONFIG_FILE);
	if(config_load(locker->config, filename) != 0)
		_locker_error(NULL, error_get(), 1);
	free(filename);
	return 0;
}

static int _new_demo(Locker * locker, char const * demo)
{
	if(demo == NULL && (demo = config_get(locker->config, NULL, "demo"))
			== NULL)
		return 0;
	if((locker->dplugin = plugin_new(LIBDIR, PACKAGE, "demos", demo))
			== NULL)
		return -1;
	if((locker->demo = plugin_lookup(locker->dplugin, "demo")) == NULL)
		return -1;
	locker->demo->helper = &locker->dhelper;
	if(locker->demo->init(locker->demo) != 0)
	{
		locker->demo = NULL;
		return -1;
	}
	return 0;
}

static void _new_helpers(Locker * locker)
{
	locker->dhelper.locker = locker;
	locker->dhelper.error = _locker_error;
	locker->phelper.locker = locker;
	locker->phelper.error = _locker_error;
	locker->phelper.action = _locker_action;
}

static GtkWidget * _new_plugin(Locker * locker, char const * plugin)
{
	GtkWidget * widget;

	if(plugin == NULL)
		plugin = config_get(locker->config, NULL, "plugin");
	if(plugin == NULL)
#ifdef EMBEDDED
		plugin = "slider";
#else
		plugin = "password";
#endif
	if((locker->pplugin = plugin_new(LIBDIR, PACKAGE, "plugins", plugin))
			== NULL)
		return NULL;
	if((locker->plugin = plugin_lookup(locker->pplugin, "plugin")) == NULL)
		return NULL;
	locker->plugin->helper = &locker->phelper;
	if((widget = locker->plugin->init(locker->plugin)) == NULL)
	{
		locker->plugin = NULL;
		return NULL;
	}
	return widget;
}

static int _new_xss(Locker * locker, size_t cnt)
{
	int error;

	/* register as screensaver */
	if(XScreenSaverQueryExtension(GDK_DISPLAY_XDISPLAY(locker->display),
				&locker->event, &error) == 0
			|| XScreenSaverRegister(GDK_DISPLAY_XDISPLAY(
					locker->display), locker->screen,
				getpid(), XA_INTEGER) == 0
			|| (locker->windows = malloc(sizeof(*locker->windows)
					* cnt)) == NULL)
		return -error_set_code(1, "%s", "Could not register as"
				" screensaver");
	return 0;
}


/* locker_delete */
void locker_delete(Locker * locker)
{
	if(locker->plugin != NULL)
		locker->plugin->destroy(locker->plugin);
	if(locker->pplugin != NULL)
		plugin_delete(locker->pplugin);
	if(locker->demo != NULL)
		locker->demo->destroy(locker->demo);
	if(locker->dplugin != NULL)
		plugin_delete(locker->dplugin);
	free(locker->windows);
	XScreenSaverUnregister(GDK_DISPLAY_XDISPLAY(locker->display),
			locker->screen);
	if(locker->config != NULL)
		config_delete(locker->config);
	object_delete(locker);
}


/* useful */
/* locker_show_preferences */
static void _preferences_window(Locker * locker);
/* callbacks */
static void _preferences_on_cancel(gpointer data);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_ok(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);

void locker_show_preferences(Locker * locker, gboolean show)
{
	if(locker->pr_window == NULL)
		_preferences_window(locker);
	if(locker->pr_window != NULL)
	{
		if(show)
			gtk_window_present(GTK_WINDOW(locker->pr_window));
		else
			gtk_widget_hide(locker->pr_window);
		return;
	}
}

static void _preferences_window(Locker * locker)
{
	GtkWidget * vbox;
	GtkWidget * notebook;

	locker->pr_window = gtk_dialog_new_with_buttons(
			"Screensaver preferences", NULL, 0,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_window_set_default_size(GTK_WINDOW(locker->pr_window), 400, 300);
	g_signal_connect_swapped(locker->pr_window, "delete-event", G_CALLBACK(
				_preferences_on_closex), locker);
	g_signal_connect(locker->pr_window, "response", G_CALLBACK(
				_preferences_on_response), locker);
	notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	/* authentication */
	/* FIXME implement */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_vbox_new(FALSE, 0),
			gtk_label_new("Authentication"));
	/* demos */
	/* FIXME implement */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_vbox_new(FALSE, 0),
			gtk_label_new("Demos"));
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(locker->pr_window));
#else
	vbox = GTK_DIALOG(locker->pr_window)->vbox;
#endif
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	_preferences_on_cancel(locker);
	gtk_widget_show_all(vbox);
}

static void _preferences_on_cancel(gpointer data)
{
	/* FIXME implement */
}

static gboolean _preferences_on_closex(gpointer data)
{
	Locker * locker = data;

	gtk_widget_hide(locker->pr_window);
	return TRUE;
}

static void _preferences_on_ok(gpointer data)
{
	/* FIXME implement */
}

static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_OK)
		_preferences_on_ok(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_preferences_on_cancel(data);
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
		case LOCKER_ACTION_SHOW_PREFERENCES:
			locker_show_preferences(locker, TRUE);
			return;
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
	GtkWidget * dialog;

	if(locker == NULL)
	{
		fprintf(stderr, "%s: %s\n", "locker", message);
		return ret;
	}
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
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
static GdkFilterReturn _filter_client_message(Locker * locker,
		XClientMessageEvent * xclient);
static GdkFilterReturn _filter_xscreensaver_notify(Locker * locker,
		XScreenSaverNotifyEvent * xssne);

static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Locker * locker = data;
	XEvent * xev = xevent;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() 0x%x 0x%x\n", __func__, xev->type,
			event->type);
#endif
	if(xev->type == ClientMessage)
		return _filter_client_message(locker, xevent);
	else if(xev->type == locker->event)
		return _filter_xscreensaver_notify(locker, xevent);
	else
		return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_client_message(Locker * locker,
		XClientMessageEvent * xclient)
{
	LockerAction action;
	gboolean show;

	if(xclient->message_type != gdk_x11_get_xatom_by_name(
				LOCKER_CLIENT_MESSAGE)
			|| xclient->data.b[0] != LOCKER_MESSAGE_ACTION)
		return GDK_FILTER_CONTINUE;
	action = xclient->data.b[1];
	switch(action)
	{
		case LOCKER_ACTION_ACTIVATE:
			_locker_activate(locker);
			break;
		case LOCKER_ACTION_LOCK:
			_locker_lock(locker);
			break;
		case LOCKER_ACTION_SHOW_PREFERENCES:
			show = xclient->data.b[1] ? TRUE : FALSE;
			locker_show_preferences(locker, show);
			break;
		case LOCKER_ACTION_UNLOCK:
			_locker_unlock(locker);
			break;
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_xscreensaver_notify(Locker * locker,
		XScreenSaverNotifyEvent * xssne)
{
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


/* locker_on_realize */
static void _locker_on_realize(GtkWidget * widget, gpointer data)
{
	Locker * locker = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %lu\n", __func__, GDK_WINDOW_XWINDOW(
				widget->window));
#endif
	if(locker->demo != NULL && locker->demo->add != NULL)
		locker->demo->add(locker->demo, GDK_WINDOW_XWINDOW(
					widget->window));
}
