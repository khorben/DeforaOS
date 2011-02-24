/* $Id$ */
static char _copyright[] =
"Copyright (c) 2011 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Panel */
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



#include <System.h>
#include <Desktop.h>
#include <sys/stat.h>
#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sysctl.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "window.h"
#include "common.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* Panel */
/* private */
/* types */
struct _Panel
{
	Config * config;

	PanelPrefs prefs;

	PanelAppletHelper top_helper;
	PanelWindow * top;
	PanelAppletHelper bottom_helper;
	PanelWindow * bottom;

	GdkWindow * root;
	gint root_width;		/* width of the root window	*/
	gint root_height;		/* height of the root window	*/

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_notebook;
	GtkWidget * pr_bottom_size;
	GtkWidget * pr_top_size;

	/* dialogs */
	GtkWidget * ab_window;
#ifndef EMBEDDED
	GtkWidget * lo_window;
#endif
	GtkWidget * sh_window;
};


/* constants */
#define PANEL_CONFIG_FILE ".panel"

static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static struct
{
	char const * alias;
	GtkIconSize iconsize;
} _panel_sizes[] =
{
	{ N_("large"),		PANEL_ICON_SIZE_LARGE },
	{ N_("small"),		PANEL_ICON_SIZE_SMALL },
	{ N_("smaller"),	PANEL_ICON_SIZE_SMALLER },
};


/* prototypes */
static gboolean _panel_can_suspend(void);

/* helpers */
static char const * _panel_helper_config_get(Panel * panel,
		char const * section, char const * variable);
static int _panel_helper_config_set(Panel * panel, char const * section,
		char const * variable, char const * value);
static int _panel_helper_error(Panel * panel, char const * message, int ret);
static void _panel_helper_about_dialog(Panel * panel);
#ifndef EMBEDDED
static void _panel_helper_logout_dialog(Panel * panel);
#endif
static void _panel_helper_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, PanelPosition position, gpointer data);
static void _panel_helper_position_menu_bottom(GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in, gpointer data);
static void _panel_helper_position_menu_top(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data);
static void _panel_helper_preferences_dialog(Panel * panel);
static void _panel_helper_shutdown_dialog(Panel * panel);
static int _panel_helper_suspend(Panel * panel);

static char * _config_get_filename(void);


/* public */
/* panel_new */
static int _new_config(Panel * panel);
static void _new_prefs(PanelPrefs * prefs, PanelPrefs const * user);
static GtkIconSize _new_size(Panel * panel, PanelPosition position);
static gboolean _on_idle(gpointer data);
static void _idle_load(Panel * panel, PanelPosition position,
		char const * plugins);
static GdkFilterReturn _on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Panel * panel_new(PanelPrefs const * prefs)
{
	Panel * panel;
	GdkScreen * screen;
	GdkRectangle rect;

	if((panel = object_new(sizeof(*panel))) == NULL)
		return NULL;
	_new_config(panel);
	_new_prefs(&panel->prefs, prefs);
	prefs = &panel->prefs;
	panel->top_helper.panel = panel;
	panel->top_helper.config_get = _panel_helper_config_get;
	panel->top_helper.config_set = _panel_helper_config_set;
	panel->top_helper.error = _panel_helper_error;
	panel->top_helper.icon_size = PANEL_ICON_SIZE_UNSET;
	panel->top_helper.about_dialog = _panel_helper_about_dialog;
#ifndef EMBEDDED
	panel->top_helper.logout_dialog = _panel_helper_logout_dialog;
#else
	panel->top_helper.logout_dialog = NULL;
#endif
	panel->top_helper.position_menu = _panel_helper_position_menu_top;
	panel->top_helper.preferences_dialog = _panel_helper_preferences_dialog;
	panel->top_helper.shutdown_dialog = _panel_helper_shutdown_dialog;
	panel->top_helper.suspend = (_panel_can_suspend())
		? _panel_helper_suspend : NULL;
	panel->top = NULL;
	panel->bottom_helper = panel->top_helper;
	panel->bottom_helper.position_menu = _panel_helper_position_menu_bottom;
	panel->bottom = NULL;
	switch(prefs->iconsize)
	{
		case PANEL_ICON_SIZE_LARGE:
		case PANEL_ICON_SIZE_SMALL:
		case PANEL_ICON_SIZE_SMALLER:
			panel->top_helper.icon_size = prefs->iconsize;
			panel->bottom_helper.icon_size = prefs->iconsize;
			break;
		case PANEL_ICON_SIZE_UNSET:
		default:
			panel->top_helper.icon_size = _new_size(panel,
					PANEL_POSITION_TOP);
			panel->bottom_helper.icon_size = _new_size(panel,
					PANEL_POSITION_BOTTOM);
			break;
	}
	panel->pr_window = NULL;
	panel->ab_window = NULL;
	if(panel->config == NULL)
	{
		panel_error(NULL, error_get(), 0); /* XXX put up a dialog box */
		panel_delete(panel);
		return NULL;
	}
	/* root window */
	screen = gdk_screen_get_default();
	panel->root = gdk_screen_get_root_window(screen);
	gdk_screen_get_monitor_geometry(screen, (prefs->monitor > 0
				&& prefs->monitor < gdk_screen_get_n_monitors(
					screen)) ? prefs->monitor : 0, &rect);
	panel->root_height = rect.height;
	panel->root_width = rect.width;
	panel->top = (prefs->position & PANEL_POSITION_TOP)
		? panel_window_new(PANEL_POSITION_TOP, &panel->top_helper,
				&rect) : NULL;
	panel->bottom = (prefs->position & PANEL_POSITION_BOTTOM)
		? panel_window_new(PANEL_POSITION_BOTTOM,
				&panel->bottom_helper, &rect) : NULL;
	/* manage root window events */
	gdk_add_client_message_filter(gdk_atom_intern(PANEL_CLIENT_MESSAGE,
				FALSE), _on_root_event, panel);
	/* load plug-ins when idle */
	g_idle_add(_on_idle, panel);
	return panel;
}

static int _new_config(Panel * panel)
{
	char * filename;

	if((panel->config = config_new()) == NULL)
		return -1;
	if((filename = _config_get_filename()) == NULL)
		return -1;
	config_load(panel->config, filename); /* we can ignore errors */
	free(filename);
	return 0;
}

static void _new_prefs(PanelPrefs * prefs, PanelPrefs const * user)
{
	size_t i;

	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
		if(gtk_icon_size_from_name(_panel_sizes[i].alias)
				== GTK_ICON_SIZE_INVALID)
			gtk_icon_size_register_alias(_panel_sizes[i].alias,
					_panel_sizes[i].iconsize);
	if(user != NULL)
	{
		memcpy(prefs, user, sizeof(*prefs));
		return;
	}
	prefs->iconsize = PANEL_ICON_SIZE_DEFAULT;
	prefs->monitor = -1;
	prefs->position = PANEL_POSITION_DEFAULT;
}

static GtkIconSize _new_size(Panel * panel, PanelPosition position)
{
	GtkIconSize ret = GTK_ICON_SIZE_INVALID;
	char const * variable = NULL;
	char const * p = NULL;

	switch(position)
	{
		case PANEL_POSITION_BOTTOM:
			variable = "bottom_size";
			break;
		case PANEL_POSITION_TOP:
			variable = "top_size";
			break;
	}
	if(variable != NULL)
		p = config_get(panel->config, NULL, variable);
	if(p == NULL)
		p = config_get(panel->config, NULL, "size");
	if(p != NULL)
		ret = gtk_icon_size_from_name(p);
	if(ret == GTK_ICON_SIZE_INVALID)
		ret = PANEL_ICON_SIZE_DEFAULT;
	return ret;
}

static gboolean _on_idle(gpointer data)
{
	Panel * panel = data;
#ifndef EMBEDDED
	const char * plugins = "volume,systray,battery,bluetooth,clock,swap"
		",memory,cpufreq,cpu,desktop,gps,gsm,lock,logout,main,pager"
		",tasks";
#else
	const char * plugins = "volume,systray,battery,bluetooth,clock,cpufreq"
		",gps,gsm,main,pager,tasks";
#endif
	char const * p;
	char const * top;
	char const * bottom;

	panel_show_preferences(panel, FALSE);
	if((p = config_get(panel->config, NULL, "plugins")) == NULL)
		p = plugins;
	if(panel->top != NULL)
		if((top = config_get(panel->config, NULL, "top")) != NULL
				|| (top = p) != NULL)
			_idle_load(panel, PANEL_POSITION_TOP, top);
	if(panel->bottom != NULL)
		if((bottom = config_get(panel->config, NULL, "bottom")) != NULL
				|| (bottom = p) != NULL)
			_idle_load(panel, PANEL_POSITION_BOTTOM, bottom);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(panel->pr_notebook), 0);
	return FALSE;
}

static void _idle_load(Panel * panel, PanelPosition position,
		char const * plugins)
{
	char * p;
	char * q;
	size_t i;

	if((p = string_new(plugins)) == NULL)
	{
		panel_error(panel, error_get(), FALSE);
		return;
	}
	for(q = p, i = 0;;)
	{
		if(q[i] == '\0')
		{
			if(panel_load(panel, position, q) != 0)
				error_print(PACKAGE); /* we can ignore errors */
			break;
		}
		if(q[i++] != ',')
			continue;
		q[i - 1] = '\0';
		if(panel_load(panel, position, q) != 0)
			error_print(PACKAGE); /* we can ignore errors */
		q += i;
		i = 0;
	}
	free(p);
}

static GdkFilterReturn _on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Panel * panel = data;
	XEvent * xe = xevent;
	XClientMessageEvent * xcme;
	PanelMessage message;

	if(xe->type != ClientMessage)
		return GDK_FILTER_CONTINUE;
	xcme = &xe->xclient;
	if(xcme->message_type != gdk_x11_get_xatom_by_name(
				PANEL_CLIENT_MESSAGE))
		return GDK_FILTER_CONTINUE;
	message = xcme->data.b[0];
	switch(message)
	{
		case PANEL_MESSAGE_SHOW:
			if(xcme->data.b[1] == PANEL_MESSAGE_SHOW_SETTINGS)
				panel_show_preferences(panel, TRUE);
			break;
	}
	return GDK_FILTER_CONTINUE;
}


/* panel_delete */
void panel_delete(Panel * panel)
{
	/* FIXME destroy plugins as well */
	if(panel->top != NULL)
		panel_window_delete(panel->top);
	if(panel->bottom != NULL)
		panel_window_delete(panel->bottom);
	if(panel->config != NULL)
		config_delete(panel->config);
	object_delete(panel);
}


/* useful */
/* panel_error */
static int _error_text(char const * message, int ret);
static gboolean _error_on_closex(GtkWidget * widget);
static void _error_on_response(GtkWidget * widget);

int panel_error(Panel * panel, char const * message, int ret)
{
	GtkWidget * dialog;

	if(panel == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s: %s", message, strerror(errno));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(G_OBJECT(dialog), "delete-event", G_CALLBACK(
				_error_on_closex), NULL);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				_error_on_response), NULL);
	gtk_widget_show_all(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}

static gboolean _error_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return FALSE;
}

static void _error_on_response(GtkWidget * widget)
{
	gtk_widget_destroy(widget);
}


/* panel_load */
int panel_load(Panel * panel, PanelPosition position, char const * applet)
{
	PanelWindow * window;
	PanelAppletHelper * helper;
	Plugin * plugin;
	PanelApplet * pa;
	GtkWidget * widget;

	if(position == PANEL_POSITION_BOTTOM && panel->bottom != NULL)
	{
		window = panel->bottom;
		helper = &panel->bottom_helper;
	}
	else if(position == PANEL_POSITION_TOP && panel->top != NULL)
	{
		window = panel->top;
		helper = &panel->top_helper;
	}
	else
		return -1;
	if((plugin = plugin_new(LIBDIR, PACKAGE, "applets", applet)) == NULL)
		return -1;
	if((pa = plugin_lookup(plugin, "applet")) == NULL
			|| (pa->helper = helper) == NULL
			|| pa->init == NULL || (widget = pa->init(pa)) == NULL)
	{
		plugin_delete(plugin);
		return -1;
	}
	panel_window_append(window, widget, pa->expand, pa->fill, pa->position);
	if(pa->settings != NULL
			&& (widget = pa->settings(pa, FALSE, FALSE)) != NULL)
		gtk_notebook_append_page(GTK_NOTEBOOK(panel->pr_notebook),
				widget, gtk_label_new(pa->name));
	return 0;
}


/* panel_show_preferences */
static void _show_preferences_window(Panel * panel);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void panel_show_preferences(Panel * panel, gboolean show)
{
	if(panel->pr_window == NULL)
		_show_preferences_window(panel);
	if(show == FALSE)
		gtk_widget_hide(panel->pr_window);
	else
		gtk_window_present(GTK_WINDOW(panel->pr_window));
}

static void _show_preferences_window(Panel * panel)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;
	size_t i;

	panel->pr_window = gtk_dialog_new_with_buttons(_("Panel preferences"),
			NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(G_OBJECT(panel->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), panel);
	g_signal_connect(G_OBJECT(panel->pr_window), "response",
			G_CALLBACK(_preferences_on_response), panel);
	panel->pr_notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(panel->pr_notebook), TRUE);
	/* general */
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Top size:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	panel->pr_top_size = gtk_combo_box_new_text();
	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_top_size),
				_(_panel_sizes[i].alias));
	gtk_box_pack_start(GTK_BOX(hbox), panel->pr_top_size, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Bottom size:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	panel->pr_bottom_size = gtk_combo_box_new_text();
	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_bottom_size),
				_(_panel_sizes[i].alias));
	gtk_box_pack_start(GTK_BOX(hbox), panel->pr_bottom_size, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(panel->pr_notebook), vbox,
			gtk_label_new(_("General")));
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(panel->pr_window));
#else
	vbox = GTK_DIALOG(panel->pr_window)->vbox;
#endif
	gtk_box_pack_start(GTK_BOX(vbox), panel->pr_notebook, TRUE, TRUE, 0);
	/* FIXME implement a way to enable plug-ins per panel (and in order) */
	_preferences_on_cancel(panel);
	gtk_widget_show_all(vbox);
}

static gboolean _preferences_on_closex(gpointer data)
{
	Panel * panel = data;

	_preferences_on_cancel(panel);
	return TRUE;
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

static void _preferences_on_cancel(gpointer data)
{
	Panel * panel = data;
	char const * p;
	size_t i;
	const size_t cnt = sizeof(_panel_sizes) / sizeof(*_panel_sizes);

	gtk_widget_hide(panel->pr_window);
	if((p = config_get(panel->config, "", "bottom_size")) != NULL
			|| (p = config_get(panel->config, "", "size")) != NULL)
		for(i = 0; i < cnt; i++)
		{
			if(strcmp(p, _panel_sizes[i].alias) != 0)
				continue;
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						panel->pr_bottom_size), i);
			break;
		}
	if((p = config_get(panel->config, "", "top_size")) != NULL
			|| (p = config_get(panel->config, "", "size")) != NULL)
		for(i = 0; i < cnt; i++)
		{
			if(strcmp(p, _panel_sizes[i].alias) != 0)
				continue;
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						panel->pr_top_size), i);
			break;
		}
}

static void _preferences_on_ok(gpointer data)
{
	Panel * panel = data;
	gint i;
	const gint cnt = sizeof(_panel_sizes) / sizeof(*_panel_sizes);
	char * filename;

	gtk_widget_hide(panel->pr_window);
	if((i = gtk_combo_box_get_active(GTK_COMBO_BOX(panel->pr_bottom_size)))
			>= 0 && i < cnt)
		config_set(panel->config, NULL, "bottom_size",
				_panel_sizes[i].alias);
	if((i = gtk_combo_box_get_active(GTK_COMBO_BOX(panel->pr_top_size)))
			>= 0 && i < cnt)
		config_set(panel->config, NULL, "top_size",
				_panel_sizes[i].alias);
	if((filename = _config_get_filename()) != NULL)
		config_save(panel->config, filename);
	free(filename);
}


/* private */
/* functions */
static char * _config_get_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(PANEL_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, PANEL_CONFIG_FILE);
	return filename;
}


/* panel_can_suspend */
static gboolean _panel_can_suspend(void)
{
#ifdef __NetBSD__
	int sleep_state = -1;
	size_t size = sizeof(sleep_state);

	if(sysctlbyname("machdep.sleep_state", &sleep_state, &size, NULL, 0)
			== 0 && sleep_state == 0
			&& sysctlbyname("machdep.sleep_state", &sleep_state,
				&size, &sleep_state, size) == 0)
		return TRUE;
#else
	struct stat st;

	if(lstat("/proc/apm", &st) == 0)
		return TRUE;
#endif
	return TRUE;
}


/* helpers */
/* panel_helper_config_get */
static char const * _panel_helper_config_get(Panel * panel,
		char const * section, char const * variable)
{
	return config_get(panel->config, section, variable);
}


/* panel_helper_config_set */
static int _panel_helper_config_set(Panel * panel, char const * section,
		char const * variable, char const * value)
{
	/* FIXME also save the configuration */
	return config_set(panel->config, section, variable, value);
}


/* panel_helper_error */
static int _panel_helper_error(Panel * panel, char const * message, int ret)
{
	return panel_error(panel, message, ret);
}


/* panel_helper_about_dialog */
static gboolean _about_on_closex(gpointer data);

static void _panel_helper_about_dialog(Panel * panel)
{
	if(panel->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(panel->ab_window));
		return;
	}
	panel->ab_window = desktop_about_dialog_new();
	desktop_about_dialog_set_authors(panel->ab_window, _authors);
	desktop_about_dialog_set_copyright(panel->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(panel->ab_window,
			"panel-settings"); /* XXX */
	desktop_about_dialog_set_license(panel->ab_window, _license);
	desktop_about_dialog_set_name(panel->ab_window, PACKAGE);
	desktop_about_dialog_set_version(panel->ab_window, VERSION);
	gtk_window_set_position(GTK_WINDOW(panel->ab_window),
			GTK_WIN_POS_CENTER_ALWAYS);
	g_signal_connect_swapped(G_OBJECT(panel->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), panel);
	gtk_widget_show(panel->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Panel * panel = data;

	gtk_widget_hide(panel->ab_window);
	return TRUE;
}


#ifndef EMBEDDED
/* panel_helper_logout_dialog */
static gboolean _logout_dialog_on_closex(gpointer data);
static void _logout_dialog_on_response(GtkWidget * widget, gint response);

static void _panel_helper_logout_dialog(Panel * panel)
{
	const char * message = _("This will log you out of the current session,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?");
	GtkWidget * widget;

	if(panel->lo_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(panel->lo_window));
		return;
	}
	panel->lo_window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Logout"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				panel->lo_window),
#endif
			"%s", message);
	gtk_dialog_add_buttons(GTK_DIALOG(panel->lo_window), GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);
	widget = gtk_button_new_with_label(_("Logout"));
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"gnome-logout", GTK_ICON_SIZE_BUTTON));
	gtk_widget_show_all(widget);
	gtk_dialog_add_action_widget(GTK_DIALOG(panel->lo_window), widget,
			GTK_RESPONSE_ACCEPT);
	gtk_window_set_keep_above(GTK_WINDOW(panel->lo_window), TRUE);
	gtk_window_set_position(GTK_WINDOW(panel->lo_window),
			GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(panel->lo_window), _("Logout"));
	g_signal_connect(G_OBJECT(panel->lo_window), "delete-event", G_CALLBACK(
				_logout_dialog_on_closex), panel);
	g_signal_connect(G_OBJECT(panel->lo_window), "response", G_CALLBACK(
				_logout_dialog_on_response), panel);
	gtk_widget_show_all(panel->lo_window);
}

static gboolean _logout_dialog_on_closex(gpointer data)
{
	Panel * panel = data;

	gtk_widget_hide(panel->lo_window);
	return TRUE;
}

static void _logout_dialog_on_response(GtkWidget * widget, gint response)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_ACCEPT)
		gtk_main_quit();
}
#endif


/* panel_helper_position_menu */
static void _panel_helper_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, PanelPosition position, gpointer data)
{
	Panel * panel = data;
	GtkRequisition req;

	gtk_widget_size_request(GTK_WIDGET(menu), &req);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d, height=%d\n", __func__,
			req.width, req.height);
#endif
	if(req.height <= 0)
		return;
	*x = (req.width < panel->root_width - PANEL_BORDER_WIDTH)
		? PANEL_BORDER_WIDTH : 0;
	if(position == PANEL_POSITION_TOP)
		*y = panel_window_get_height(panel->top);
	else if(position == PANEL_POSITION_BOTTOM)
		*y = panel->root_height
			- panel_window_get_height(panel->bottom) - req.height;
	else /* XXX generic */
		*y = panel->root_height - req.height;
	*push_in = TRUE;
}


/* panel_helper_position_menu_bottom */
static void _panel_helper_position_menu_bottom(GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in, gpointer data)
{
	_panel_helper_position_menu(menu, x, y, push_in, PANEL_POSITION_BOTTOM,
			data);
}


/* panel_helper_position_menu_top */
static void _panel_helper_position_menu_top(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data)
{
	_panel_helper_position_menu(menu, x, y, push_in, PANEL_POSITION_TOP,
			data);
}


/* panel_helper_preferences_dialog */
static void _panel_helper_preferences_dialog(Panel * panel)
{
	panel_show_preferences(panel, TRUE);
}


/* panel_helper_shutdown_dialog */
static gboolean _shutdown_dialog_on_closex(gpointer data);
static void _shutdown_dialog_on_response(GtkWidget * widget, gint response,
		gpointer data);
enum { RES_CANCEL, RES_REBOOT, RES_SHUTDOWN };

static void _panel_helper_shutdown_dialog(Panel * panel)
{
	GtkWidget * widget;
#ifdef EMBEDDED
	const char * message = _("This will shutdown your device,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?");
#else
	const char * message = _("This will shutdown your computer,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?");
#endif

	if(panel->sh_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(panel->sh_window));
		return;
	}
	panel->sh_window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Shutdown"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				panel->sh_window),
#endif
			"%s", message);
	gtk_dialog_add_buttons(GTK_DIALOG(panel->sh_window), GTK_STOCK_CANCEL,
			RES_CANCEL, _("Restart"), RES_REBOOT, NULL);
	widget = gtk_button_new_with_label(_("Shutdown"));
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"gnome-shutdown", GTK_ICON_SIZE_BUTTON));
	gtk_widget_show_all(widget);
	gtk_dialog_add_action_widget(GTK_DIALOG(panel->sh_window), widget,
			RES_SHUTDOWN);
	gtk_window_set_keep_above(GTK_WINDOW(panel->sh_window), TRUE);
	gtk_window_set_position(GTK_WINDOW(panel->sh_window),
			GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(panel->sh_window), _("Shutdown"));
	g_signal_connect(G_OBJECT(panel->lo_window), "delete-event", G_CALLBACK(
				_shutdown_dialog_on_closex), panel);
	g_signal_connect(G_OBJECT(panel->lo_window), "response", G_CALLBACK(
				_shutdown_dialog_on_response), panel);
	gtk_widget_show_all(panel->lo_window);
}

static gboolean _shutdown_dialog_on_closex(gpointer data)
{
	Panel * panel = data;

	gtk_widget_hide(panel->sh_window);
	return TRUE;
}

static void _shutdown_dialog_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	Panel * panel = data;
	char * reboot[] = { "/sbin/shutdown", "shutdown", "-r", "now", NULL };
	char * shutdown[] = { "/sbin/shutdown", "shutdown",
#ifdef __NetBSD__
		"-p",
#else
		"-h",
#endif
		"now", NULL };
	char ** argv;
	GError * error = NULL;

	gtk_widget_hide(widget);
	if(response == RES_SHUTDOWN)
		argv = shutdown;
	else if(response == RES_REBOOT)
		argv = reboot;
	else
		return;
	if(g_spawn_async(NULL, argv, NULL, G_SPAWN_FILE_AND_ARGV_ZERO, NULL,
				NULL, NULL, &error) != TRUE)
		panel_error(panel, error->message, 1);
}


/* panel_helper_suspend */
static int _panel_helper_suspend(Panel * panel)
{
#ifdef __NetBSD__
	int sleep_state = 3;
#else
	char * suspend[] = { "/usr/bin/sudo", "sudo", "/sbin/apm", "-s", NULL };
	GError * error = NULL;
#endif

#ifdef __NetBSD__
	if(sysctlbyname("machdep.sleep_state", NULL, NULL, &sleep_state,
				sizeof(sleep_state)) != 0)
		return panel_error(panel, "sysctl", 1);
	return 0;
#else
	if(g_spawn_async(NULL, suspend, NULL, G_SPAWN_FILE_AND_ARGV_ZERO, NULL,
				NULL, NULL, &error) != TRUE)
		return panel_error(panel, error->message, 1);
	return 0;
#endif
}
