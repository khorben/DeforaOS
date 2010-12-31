/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "common.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Panel */
/* private */
/* types */
struct _Panel
{
	Config * config;

	PanelPrefs prefs;

	gint height;

	gint icon_width;
	gint icon_height;

	PanelAppletHelper helper;
	GtkWidget * window;
	GtkWidget * hbox;

	GdkWindow * root;
	gint root_width;		/* width of the root window	*/
	gint root_height;		/* height of the root window	*/

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_notebook;
	GtkWidget * pr_bottom_size;
	GtkWidget * pr_top_size;
};


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif
#define PANEL_CONFIG_FILE ".panel"

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
/* helpers */
static char const * _panel_helper_config_get(Panel * panel,
		char const * section, char const * variable);
static int _panel_helper_config_set(Panel * panel, char const * section,
		char const * variable, char const * value);
static int _panel_helper_error(Panel * panel, char const * message, int ret);
#ifndef EMBEDDED
static int _panel_helper_logout_dialog(void);
#endif
static void _panel_helper_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data);
static void _panel_helper_preferences_dialog(Panel * panel);
static int _panel_helper_shutdown_dialog(void);

static char * _config_get_filename(void);


/* public */
/* panel_new */
static int _new_config(Panel * panel);
static void _new_prefs(PanelPrefs * prefs, PanelPrefs const * user);
static void _new_strut(Panel * panel, GdkRectangle * rect);
static gboolean _on_idle(gpointer data);
static gboolean _idle_load(Panel * panel, char const * plugins);
static gboolean _on_closex(void);

Panel * panel_new(PanelPrefs const * prefs)
{
	Panel * panel;
	char const * p = NULL;
	GdkScreen * screen;
	GdkRectangle rect;

	if((panel = malloc(sizeof(*panel))) == NULL)
	{
		panel_error(NULL, "malloc", 1);
		return NULL;
	}
	if(_new_config(panel) != 0)
	{
		/* FIXME visually warn the user */
		panel_delete(panel);
		return NULL;
	}
	_new_prefs(&panel->prefs, prefs);
	prefs = &panel->prefs;
	panel->icon_width = 48;
	panel->icon_height = 48;
	switch(prefs->iconsize)
	{
		case PANEL_ICON_SIZE_LARGE:
		case PANEL_ICON_SIZE_SMALL:
		case PANEL_ICON_SIZE_SMALLER:
			break;
		case PANEL_ICON_SIZE_UNSET:
		default:
			if(prefs->position == PANEL_POSITION_TOP)
				p = "top_size";
			else if(prefs->position == PANEL_POSITION_BOTTOM)
				p = "bottom_size";
			if(p == NULL || (p = config_get(panel->config, NULL, p))
					== NULL)
				p = config_get(panel->config, NULL, "size");
			if(p != NULL)
				panel->prefs.iconsize = gtk_icon_size_from_name(
						p);
			if(prefs->iconsize == GTK_ICON_SIZE_INVALID)
				panel->prefs.iconsize = PANEL_ICON_SIZE_DEFAULT;
			break;
	}
	if(gtk_icon_size_lookup(prefs->iconsize, &panel->icon_width,
			&panel->icon_height) != TRUE)
		error_set_print(PACKAGE, 0, _("Invalid panel size"));
	panel->helper.panel = panel;
	panel->helper.config_get = _panel_helper_config_get;
	panel->helper.config_set = _panel_helper_config_set;
	panel->helper.error = _panel_helper_error;
	panel->helper.icon_size = prefs->iconsize;
#ifndef EMBEDDED
	panel->helper.logout_dialog = _panel_helper_logout_dialog;
#else
	panel->helper.logout_dialog = NULL;
#endif
	panel->helper.position_menu = _panel_helper_position_menu;
	panel->helper.preferences_dialog = _panel_helper_preferences_dialog;
	panel->helper.shutdown_dialog = _panel_helper_shutdown_dialog;
	/* root window */
	screen = gdk_screen_get_default();
	panel->root = gdk_screen_get_root_window(screen);
	if(prefs->monitor > 0 && prefs->monitor < gdk_screen_get_n_monitors(
				screen))
		gdk_screen_get_monitor_geometry(screen, prefs->monitor, &rect);
	else
		gdk_screen_get_monitor_geometry(screen, 0, &rect);
	panel->root_width = rect.width;
	panel->root_height = rect.height;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d height=%d\n", __func__,
			panel->root_width, panel->root_height);
#endif
	/* panel */
	panel->pr_window = NULL;
	g_idle_add(_on_idle, panel);
	panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	panel->height = panel->icon_height + (PANEL_BORDER_WIDTH * 4);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() height=%d\n", __func__, panel->height);
#endif
	gtk_window_resize(GTK_WINDOW(panel->window), panel->root_width,
			panel->height);
	gtk_window_set_accept_focus(GTK_WINDOW(panel->window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(panel->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	if(prefs->position == PANEL_POSITION_TOP)
		gtk_window_move(GTK_WINDOW(panel->window), rect.x, 0);
	else
		gtk_window_move(GTK_WINDOW(panel->window), rect.x,
				rect.y + panel->root_height - panel->height);
	gtk_window_stick(GTK_WINDOW(panel->window));
	g_signal_connect(G_OBJECT(panel->window), "delete-event", G_CALLBACK(
				_on_closex), panel);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(panel->window), panel->hbox);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_widget_show_all(panel->window);
	_new_strut(panel, &rect);
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

static void _new_strut(Panel * panel, GdkRectangle * rect)
{
	GdkWindow * window;
	GdkAtom atom;
	GdkAtom cardinal;
	unsigned long strut[12];

#if GTK_CHECK_VERSION(2, 14, 0)
	window = gtk_widget_get_window(panel->window);
#else
	window = panel->window->window;
#endif
	cardinal = gdk_atom_intern("CARDINAL", FALSE);
	memset(strut, 0, sizeof(strut));
	if(panel->prefs.position == PANEL_POSITION_TOP)
	{
		strut[2] = panel->height;
		strut[8] = rect->x;
		strut[9] = rect->x + rect->width;
	}
	else
	{
		strut[3] = panel->height;
		strut[10] = rect->x;
		strut[11] = rect->x + rect->width;
	}
	atom = gdk_atom_intern("_NET_WM_STRUT", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 4);
	atom = gdk_atom_intern("_NET_WM_STRUT_PARTIAL", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 12);
}

static gboolean _on_idle(gpointer data)
{
	Panel * panel = data;
#ifndef EMBEDDED
	const char * plugins[] = { "volume", "systray", "battery", "bluetooth",
		"clock", "swap", "memory", "cpufreq", "cpu", "desktop", "gps",
		"gsm", "lock", "logout", "main", "pager", "tasks", NULL };
#else
	const char * plugins[] = { "volume", "systray", "battery", "bluetooth",
		"clock", "cpufreq", "gps", "gsm", "main", "pager", "tasks",
		NULL };
#endif
	char const * p;
	size_t i;

	panel_show_preferences(panel, FALSE);
	p = config_get(panel->config, NULL, (panel->prefs.position
				== PANEL_POSITION_TOP) ? "top" : "bottom");
	if(p != NULL || (p = config_get(panel->config, NULL, "plugins"))
			!= NULL)
		return _idle_load(panel, p);
	for(i = 0; plugins[i] != NULL; i++)
		if(panel_load(panel, plugins[i]) != 0)
			error_print(PACKAGE); /* we can ignore errors */
	return FALSE;
}

static gboolean _idle_load(Panel * panel, char const * plugins)
{
	char * p;
	char * q;
	size_t i;

	if((p = strdup(plugins)) == NULL)
		return panel_error(panel, "strdup", FALSE);
	for(q = p, i = 0;;)
	{
		if(q[i] == '\0')
		{
			if(panel_load(panel, q) != 0)
				error_print(PACKAGE); /* we can ignore errors */
			break;
		}
		if(q[i++] != ',')
			continue;
		q[i - 1] = '\0';
		if(panel_load(panel, q) != 0)
			error_print(PACKAGE); /* we can ignore errors */
		q += i;
		i = 0;
	}
	free(p);
	return FALSE;
}

static gboolean _on_closex(void)
{
	/* ignore delete events */
	return TRUE;
}


/* panel_delete */
void panel_delete(Panel * panel)
{
	/* FIXME destroy plugins as well */
	if(panel->config != NULL)
		config_delete(panel->config);
	free(panel);
}


/* useful */
/* panel_error */
int panel_error(Panel * panel, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
#endif
			"%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}


/* panel_load */
int panel_load(Panel * panel, char const * applet)
{
	Plugin * plugin;
	PanelApplet * pa;
	GtkWidget * widget;
	gboolean exp;
	gboolean fill;

	if((plugin = plugin_new(LIBDIR, PACKAGE, "applets", applet)) == NULL)
		return -1;
	if((pa = plugin_lookup(plugin, "applet")) == NULL
			|| (pa->helper = &panel->helper) == NULL
			|| pa->init == NULL || (widget = pa->init(pa)) == NULL)
	{
		plugin_delete(plugin);
		return -1;
	}
	exp = pa->expand;
	fill = pa->fill;
	switch(pa->position)
	{
		case PANEL_APPLET_POSITION_END:
			gtk_box_pack_end(GTK_BOX(panel->hbox), widget, exp,
					fill, 0);
			break;
		case PANEL_APPLET_POSITION_FIRST:
			gtk_box_pack_start(GTK_BOX(panel->hbox), widget, exp,
					fill, 0);
			gtk_box_reorder_child(GTK_BOX(panel->hbox), widget, 0);
			break;
		case PANEL_APPLET_POSITION_LAST:
			gtk_box_pack_end(GTK_BOX(panel->hbox), widget, exp,
					fill, 0);
			gtk_box_reorder_child(GTK_BOX(panel->hbox), widget, 0);
			break;
		case PANEL_APPLET_POSITION_START:
			gtk_box_pack_start(GTK_BOX(panel->hbox), widget, exp,
					fill, 0);
			break;
	}
	if(pa->settings != NULL
			&& (widget = pa->settings(pa, FALSE, FALSE)) != NULL)
		/* FIXME only affects the current panel's preferences */
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
	gtk_widget_show_all(panel->pr_window);
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
	if((p = config_get(panel->config, NULL, "bottom_size")) != NULL)
		for(i = 0; i < cnt; i++)
		{
			if(strcmp(p, _panel_sizes[i].alias) != 0)
				continue;
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						panel->pr_bottom_size), i);
			break;
		}
	if((p = config_get(panel->config, NULL, "top_size")) != NULL)
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
static int _error_text(char const * message, int ret);

static int _panel_helper_error(Panel * panel, char const * message, int ret)
{
	if(panel == NULL)
		return _error_text(message, ret);
	return panel_error(panel, message, ret);
}

static int _error_text(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


#ifndef EMBEDDED
/* panel_helper_logout_dialog */
static int _panel_helper_logout_dialog(void)
{
	GtkWidget * dialog;
	const char * message = _("This will log you out of the current session,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?");
	GtkWidget * widget;
	int res;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Logout"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);
	widget = gtk_button_new_with_label(_("Logout"));
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"gnome-logout", GTK_ICON_SIZE_BUTTON));
	gtk_widget_show_all(widget);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), widget,
			GTK_RESPONSE_ACCEPT);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Logout"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res != GTK_RESPONSE_ACCEPT)
		return 1;
	gtk_main_quit();
	return 0;
}
#endif


/* panel_helper_position_menu */
static void _panel_helper_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data)
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
	if(panel->prefs.position == PANEL_POSITION_TOP)
		*y = panel->height;
	else
		*y = panel->root_height - panel->height - req.height;
	*push_in = TRUE;
}


/* panel_helper_preferences_dialog */
static void _panel_helper_preferences_dialog(Panel * panel)
{
	panel_show_preferences(panel, TRUE);
}


/* panel_helper_shutdown_dialog */
static int _panel_helper_shutdown_dialog(void)
{
	GtkWidget * dialog;
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
	enum { RES_CANCEL, RES_REBOOT, RES_SHUTDOWN };
	int res;
	char * reboot[] = { "/sbin/shutdown", "shutdown", "-r", "now", NULL };
	char * shutdown[] = { "/sbin/shutdown", "shutdown",
#ifdef __NetBSD__
		"-p",
#else
		"-h",
#endif
		"now", NULL };

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE, "%s", _("Shutdown"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, RES_CANCEL,
			_("Restart"), RES_REBOOT, NULL);
	widget = gtk_button_new_with_label(_("Shutdown"));
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"gnome-shutdown", GTK_ICON_SIZE_BUTTON));
	gtk_widget_show_all(widget);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), widget, RES_SHUTDOWN);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Shutdown"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res == RES_SHUTDOWN)
		g_spawn_async(NULL, shutdown, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, NULL, NULL);
	else if(res == RES_REBOOT)
		g_spawn_async(NULL, reboot, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, NULL, NULL);
	else
		return 1;
	return 0;
}
