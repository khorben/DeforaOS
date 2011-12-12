/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Locker */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <System.h>
#include <Desktop.h>
#include "locker.h"
#include "../config.h"
#define _(string) gettext(string)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* Locker */
/* private */
/* types */
typedef struct _LockerPlugins
{
	Plugin * pplugin;
	LockerPlugin * plugin;
} LockerPlugins;

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

	/* auth */
	Plugin * aplugin;
	LockerAuth * auth;
	LockerAuthHelper ahelper;

	/* demo */
	Plugin * dplugin;
	LockerDemo * demo;
	LockerDemoHelper dhelper;

	/* plug-ins */
	LockerPlugins * plugins;
	size_t plugins_cnt;
	LockerPluginHelper phelper;

	/* preferences */
	GtkWidget * pr_window;

	/* about */
	GtkWidget * ab_window;
};


/* constants */
#define LOCKER_CONFIG_FILE	".locker"

static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* prototypes */
static void _locker_about(Locker * locker);
static void _locker_action(Locker * locker, LockerAction action);
static void _locker_activate(Locker * locker);
static char const * _locker_auth_config_get(Locker * locker,
		char const * section, char const * variable);
static int _locker_auth_config_set(Locker * locker, char const * section,
		char const * variable, char const * value);
static char const * _locker_demo_config_get(Locker * locker,
		char const * section, char const * variable);
static int _locker_demo_config_set(Locker * locker, char const * section,
		char const * variable, char const * value);
static int _locker_error(Locker * locker, char const * message, int ret);
static void _locker_event(Locker * locker, LockerEvent event);
static void _locker_lock(Locker * locker);
static char const * _locker_plugin_config_get(Locker * locker,
		char const * section, char const * variable);
static int _locker_plugin_config_set(Locker * locker, char const * section,
		char const * variable, char const * value);
static void _locker_unlock(Locker * locker);

/* callbacks */
static gboolean _lock_on_closex(void);
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);
static gboolean _locker_on_map_event(gpointer data);
static void _locker_on_realize(GtkWidget * widget, gpointer data);


/* public */
/* functions */
/* locker_new */
static int _new_config(Locker * locker);
static int _new_demo(Locker * locker, char const * demo);
static void _new_helpers(Locker * locker);
static GtkWidget * _new_auth(Locker * locker, char const * plugin);
static int _new_plugins(Locker * locker);
static int _new_plugins_load(Locker * locker, char const * plugin);
static int _new_xss(Locker * locker, size_t cnt);

Locker * locker_new(int suspend, char const * demo, char const * auth)
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
	if((cnt = gdk_screen_get_n_monitors(screen)) < 1)
		cnt = 1;
	locker->windows = NULL;
	locker->windows_cnt = cnt;
	locker->dplugin = NULL;
	locker->demo = NULL;
	locker->plugins = NULL;
	locker->plugins_cnt = 0;
	locker->pr_window = NULL;
	locker->ab_window = NULL;
	/* check for errors */
	if(_new_config(locker) != 0
			|| _new_demo(locker, demo) != 0
			|| (widget = _new_auth(locker, auth)) == NULL
			|| _new_xss(locker, cnt) != 0)
	{
		_locker_error(NULL, error_get(), 1);
		locker_delete(locker);
		return NULL;
	}
	_new_plugins(locker);
	/* create windows */
	memset(&black, 0, sizeof(black));
	for(i = 0; i < locker->windows_cnt; i++)
	{
		locker->windows[i] = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gdk_screen_get_monitor_geometry(screen, i, &rect);
		gtk_window_move(GTK_WINDOW(locker->windows[i]), rect.x, rect.y);
		gtk_window_resize(GTK_WINDOW(locker->windows[i]), rect.width,
				rect.height);
		gtk_window_set_keep_above(GTK_WINDOW(locker->windows[i]), TRUE);
		gtk_window_stick(GTK_WINDOW(locker->windows[i]));
		gtk_widget_modify_bg(locker->windows[i], GTK_STATE_NORMAL,
				&black);
		g_signal_connect_swapped(G_OBJECT(locker->windows[i]),
				"delete-event", G_CALLBACK(_lock_on_closex),
				NULL);
		g_signal_connect(locker->windows[i], "realize", G_CALLBACK(
					_locker_on_realize), locker);
	}
	/* automatically grab keyboard and mouse */
	g_signal_connect_swapped(G_OBJECT(locker->windows[0]), "map-event",
			G_CALLBACK(_locker_on_map_event), locker);
	/* pack the authentication widget */
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

static GtkWidget * _new_auth(Locker * locker, char const * plugin)
{
	GtkWidget * widget;

	if(plugin == NULL)
		plugin = config_get(locker->config, NULL, "auth");
	if(plugin == NULL)
#ifdef EMBEDDED
		plugin = "slider";
#else
		plugin = "password";
#endif
	if((locker->aplugin = plugin_new(LIBDIR, PACKAGE, "auth", plugin))
			== NULL)
		return NULL;
	if((locker->auth = plugin_lookup(locker->aplugin, "plugin")) == NULL)
		return NULL;
	locker->auth->helper = &locker->ahelper;
	if(locker->auth->init == NULL
			|| (widget = locker->auth->init(locker->auth)) == NULL)
	{
		locker->auth = NULL;
		return NULL;
	}
	return widget;
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
	if(locker->demo->init != NULL && locker->demo->init(locker->demo) != 0)
	{
		locker->demo = NULL;
		return -1;
	}
	return 0;
}

static void _new_helpers(Locker * locker)
{
	/* authentication helper */
	locker->ahelper.locker = locker;
	locker->ahelper.error = _locker_error;
	locker->ahelper.action = _locker_action;
	locker->ahelper.config_get = _locker_auth_config_get;
	locker->ahelper.config_set = _locker_auth_config_set;
	/* demo helper */
	locker->dhelper.locker = locker;
	locker->dhelper.error = _locker_error;
	locker->dhelper.config_get = _locker_demo_config_get;
	locker->dhelper.config_set = _locker_demo_config_set;
	/* plug-ins helper */
	locker->phelper.locker = locker;
	locker->phelper.error = _locker_error;
	locker->phelper.about_dialog = _locker_about;
	locker->phelper.action = _locker_action;
	locker->phelper.config_get = _locker_plugin_config_get;
	locker->phelper.config_set = _locker_plugin_config_set;
}

static int _new_plugins(Locker * locker)
{
	int ret = 0;
	char const * p;
	String * plugins;
	size_t i;
	int c;

	if((p = config_get(locker->config, NULL, "plugins")) == NULL)
		return 0;
	if((plugins = string_new(p)) == NULL)
		return -1;
	for(i = 0;; i++)
	{
		if(plugins[i] != ',' && plugins[i] != '\0')
			continue;
		c = plugins[i];
		plugins[i] = '\0';
		ret |= _new_plugins_load(locker, plugins);
		if(c == '\0')
			break;
		plugins += i + 1;
		i = 0;
	}
	string_delete(plugins);
	return ret;
}

static int _new_plugins_load(Locker * locker, char const * plugin)
{
	LockerPlugins * p;

	if((p = realloc(locker->plugins, sizeof(*p) * (locker->plugins_cnt
						+ 1))) == NULL)
		return _locker_error(NULL, strerror(errno), 1);
	locker->plugins = p;
	p = &locker->plugins[locker->plugins_cnt];
	if((p->pplugin = plugin_new(LIBDIR, PACKAGE, "plugins", plugin))
			== NULL)
		return _locker_error(NULL, error_get(), 1);
	if((p->plugin = plugin_lookup(p->pplugin, "plugin")) == NULL)
	{
		plugin_delete(p->pplugin);
		return _locker_error(NULL, error_get(), 1);
	}
	p->plugin->helper = &locker->phelper;
	if(p->plugin->init != NULL && p->plugin->init(p->plugin) != 0)
	{
		plugin_delete(p->pplugin);
		return _locker_error(NULL, error_get(), 1);
	}
	locker->plugins_cnt++;
	return 0;
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
		return -error_set_code(1, "%s", _(
					"Could not register as screensaver"));
	return 0;
}


/* locker_delete */
void locker_delete(Locker * locker)
{
	size_t i;

	/* FIXME also destroy plug-ins */
	if(locker->auth != NULL && locker->auth->destroy != NULL)
		locker->auth->destroy(locker->auth);
	if(locker->aplugin != NULL)
		plugin_delete(locker->aplugin);
	if(locker->demo != NULL)
	{
		if(locker->demo->remove != NULL)
			for(i = 0; i < locker->windows_cnt; i++)
				locker->demo->remove(locker->demo,
						locker->windows[i]);
		if(locker->demo->destroy != NULL)
			locker->demo->destroy(locker->demo);
	}
	if(locker->dplugin != NULL)
		plugin_delete(locker->dplugin);
	if(locker->ab_window != NULL)
		gtk_widget_destroy(locker->ab_window);
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
			_("Screensaver preferences"), NULL, 0,
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
			gtk_label_new(_("Authentication")));
	/* demos */
	/* FIXME implement */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_vbox_new(FALSE, 0),
			gtk_label_new(_("Demos")));
	/* plug-ins */
	/* FIXME implement */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), gtk_vbox_new(FALSE, 0),
			gtk_label_new(_("Plug-ins")));
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
/* locker_about */
static gboolean _about_on_closex(gpointer data);

static void _locker_about(Locker * locker)
{
	GtkWidget * dialog;

	if(locker->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(locker->ab_window));
		return;
	}
	dialog = desktop_about_dialog_new();
	locker->ab_window = dialog;
	g_signal_connect_swapped(G_OBJECT(dialog), "delete-event", G_CALLBACK(
				_about_on_closex), locker);
	desktop_about_dialog_set_name(dialog, PACKAGE);
	desktop_about_dialog_set_version(dialog, VERSION);
	desktop_about_dialog_set_authors(dialog, _authors);
	desktop_about_dialog_set_copyright(dialog, _copyright);
	desktop_about_dialog_set_logo_icon_name(dialog, "gnome-lockscreen");
	desktop_about_dialog_set_license(dialog, _license);
	desktop_about_dialog_set_translator_credits(dialog,
			_("translator-credits"));
	desktop_about_dialog_set_website(dialog, "http://www.defora.org/");
	gtk_widget_show(dialog);
}

static gboolean _about_on_closex(gpointer data)
{
	Locker * locker = data;

	gtk_widget_hide(locker->ab_window);
	return TRUE;
}


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
}


/* locker_activate */
static void _locker_activate(Locker * locker)
{
	_locker_event(locker, LOCKER_EVENT_ACTIVATING);
	if(locker->auth->action(locker->auth, LOCKER_ACTION_ACTIVATE) == 0)
		XActivateScreenSaver(GDK_DISPLAY_XDISPLAY(locker->display));
}


/* locker_auth_config_get */
static char const * _locker_auth_config_get(Locker * locker,
		char const * section, char const * variable)
{
	char const * ret;
	String * s;

	if((s = string_new_append("auth::", section, NULL)) == NULL)
		return NULL;
	ret = config_get(locker->config, s, variable);
	string_delete(s);
	return ret;
}


/* locker_auth_config_set */
static int _locker_auth_config_set(Locker * locker, char const * section,
		char const * variable, char const * value)
{
	int ret;
	String * s;

	if((s = string_new_append("auth::", section, NULL)) == NULL)
		return -1;
	ret = config_set(locker->config, s, variable, value);
	string_delete(s);
	return ret;
}


/* locker_demo_config_get */
static char const * _locker_demo_config_get(Locker * locker,
		char const * section, char const * variable)
{
	char const * ret;
	String * s;

	if((s = string_new_append("demo::", section, NULL)) == NULL)
		return NULL;
	ret = config_get(locker->config, s, variable);
	string_delete(s);
	return ret;
}


/* locker_demo_config_set */
static int _locker_demo_config_set(Locker * locker, char const * section,
		char const * variable, char const * value)
{
	int ret;
	String * s;

	if((s = string_new_append("demo::", section, NULL)) == NULL)
		return -1;
	ret = config_set(locker->config, s, variable, value);
	string_delete(s);
	return ret;
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
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* locker_event */
static void _locker_event(Locker * locker, LockerEvent event)
{
	size_t i;
	LockerPlugin * lp;

	for(i = 0; i < locker->plugins_cnt; i++)
	{
		lp = locker->plugins[i].plugin;
		if(lp->event != NULL)
			lp->event(lp, event);
	}
}


/* locker_lock */
static void _locker_lock(Locker * locker)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_locker_event(locker, LOCKER_EVENT_LOCKING);
	for(i = 0; i < locker->windows_cnt; i++)
	{
		gtk_widget_show(locker->windows[i]);
		gtk_window_fullscreen(GTK_WINDOW(locker->windows[i]));
	}
	locker->auth->action(locker->auth, LOCKER_ACTION_LOCK);
}


/* locker_plugin_config_get */
static char const * _locker_plugin_config_get(Locker * locker,
		char const * section, char const * variable)
{
	char const * ret;
	String * s;

	if((s = string_new_append("plugin::", section, NULL)) == NULL)
		return NULL;
	ret = config_get(locker->config, s, variable);
	string_delete(s);
	return ret;
}


/* locker_plugin_config_set */
static int _locker_plugin_config_set(Locker * locker, char const * section,
		char const * variable, char const * value)
{
	int ret;
	String * s;

	if((s = string_new_append("plugin::", section, NULL)) == NULL)
		return -1;
	ret = config_set(locker->config, s, variable, value);
	string_delete(s);
	return ret;
}


/* locker_unlock */
static void _locker_unlock(Locker * locker)
{
	size_t i;

	_locker_event(locker, LOCKER_EVENT_UNLOCKING);
	/* ungrab keyboard and mouse */
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
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


/* locker_on_map_event */
static gboolean _locker_on_map_event(gpointer data)
{
	Locker * locker = data;
	GdkWindow * window;
	GdkGrabStatus status;

	/* FIXME detect if this is the first window */
	/* FIXME the mouse may already be grabbed (Panel's lock button...) */
	/* grab keyboard and mouse */
	if((window = gtk_widget_get_window(locker->windows[0])) == NULL)
		_locker_error(NULL, "Failed to grab input", 1);
	else
	{
		if((status = gdk_keyboard_grab(window, TRUE, GDK_CURRENT_TIME))
				!= GDK_GRAB_SUCCESS)
			_locker_error(NULL, "Failed to grab keyboard", 1);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: keyboard grab status=%u\n", status);
#endif
		if((status = gdk_pointer_grab(window, TRUE, 0, window, NULL,
						GDK_CURRENT_TIME))
				!= GDK_GRAB_SUCCESS)
			_locker_error(NULL, "Failed to grab mouse", 1);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: mouse grab status=%u\n", status);
#endif
	}
	return FALSE;
}


/* locker_on_realize */
static void _locker_on_realize(GtkWidget * widget, gpointer data)
{
	Locker * locker = data;

	if(locker->demo != NULL && locker->demo->add != NULL)
		locker->demo->add(locker->demo, widget);
}
