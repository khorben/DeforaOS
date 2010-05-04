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
#include "Panel.h"
#include "common.h"
#include "../config.h"
#define _(string) gettext(string)


/* Panel */
/* private */
/* types */
struct _Panel
{
	Config * config;

	gint height;

	gint icon_width;
	gint icon_height;

	PanelAppletHelper helper;
	GtkWidget * window;
	GtkWidget * hbox;

	GdkWindow * root;
	gint root_width;		/* width of the root window	*/
	gint root_height;		/* height of the root window	*/
};


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif
#define PANEL_CONFIG_FILE ".panel"


/* prototypes */
/* helpers */
static char const * _panel_helper_config_get(void * priv, char const * section,
		char const * variable);
static int _panel_helper_error(void * priv, char const * message, int ret);
#ifndef EMBEDDED
static int _panel_helper_logout_dialog(void);
#endif
static void _panel_helper_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data);
static int _panel_helper_shutdown_dialog(void);


/* public */
/* panel_new */
static int _new_config(Panel * panel);
static gboolean _on_idle(gpointer data);
static gboolean _idle_load(Panel * panel, char const * plugins);
static gboolean _on_closex(void);

Panel * panel_new(PanelPrefs * prefs)
{
	Panel * panel;
	GdkScreen * screen;
	int monitor;
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
	panel->icon_width = 48;
	panel->icon_height = 48;
	if(prefs->iconsize != PANEL_ICON_SIZE_SMALL
			&& prefs->iconsize != PANEL_ICON_SIZE_SMALLER)
		prefs->iconsize = PANEL_ICON_SIZE_LARGE;
	if(gtk_icon_size_lookup(prefs->iconsize, &panel->icon_width,
			&panel->icon_height) != TRUE)
		error_set_print(PACKAGE, 0, "Invalid panel size");
	panel->helper.priv = panel;
	panel->helper.config_get = _panel_helper_config_get;
	panel->helper.error = _panel_helper_error;
	panel->helper.icon_size = prefs->iconsize;
#ifndef EMBEDDED
	panel->helper.logout_dialog = _panel_helper_logout_dialog;
#else
	panel->helper.logout_dialog = NULL;
#endif
	panel->helper.position_menu = _panel_helper_position_menu;
	panel->helper.shutdown_dialog = _panel_helper_shutdown_dialog;
	/* root window */
	panel->root = gdk_screen_get_root_window(gdk_screen_get_default());
	screen = gdk_screen_get_default();
	if(prefs != NULL && prefs->monitor > 0
			&& prefs->monitor < gdk_screen_get_n_monitors(screen))
		monitor = prefs->monitor;
	else
		monitor = 0;
	gdk_screen_get_monitor_geometry(screen, monitor, &rect);
	panel->root_width = rect.width;
	panel->root_height = rect.height;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() width=%d height=%d\n", __func__,
			panel->root_width, panel->root_height);
#endif
	/* panel */
	g_idle_add(_on_idle, panel);
	panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	panel->height = panel->icon_height + (PANEL_BORDER_WIDTH * 8);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() height=%d\n", __func__, panel->height);
#endif
	gtk_window_resize(GTK_WINDOW(panel->window), panel->root_width,
			panel->height);
	gtk_window_set_type_hint(GTK_WINDOW(panel->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_move(GTK_WINDOW(panel->window), rect.x,
			rect.y + panel->root_height - panel->height);
	gtk_window_stick(GTK_WINDOW(panel->window));
	g_signal_connect(G_OBJECT(panel->window), "delete-event", G_CALLBACK(
				_on_closex), panel);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(panel->window), panel->hbox);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_widget_show_all(panel->window);
	return panel;
}

static int _new_config(Panel * panel)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((panel->config = config_new()) == NULL)
		return 1;
	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(PANEL_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return 1;
	snprintf(filename, len, "%s/%s", homedir, PANEL_CONFIG_FILE);
	config_load(panel->config, filename); /* we can ignore errors */
	free(filename);
	return 0;
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

	if((p = config_get(panel->config, NULL, "plugins")) != NULL)
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
int panel_error(Panel * panel, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
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
	return 0;
}


/* private */
/* functions */
/* helpers */
/* panel_helper_config_get */
static char const * _panel_helper_config_get(void * priv, char const * section,
		char const * variable)
{
	Panel * panel = priv;

	return config_get(panel->config, section, variable);
}


/* panel_helper_error */
static int _error_text(char const * message, int ret);

static int _panel_helper_error(void * priv, char const * message, int ret)
{
	Panel * panel = priv;

	if(priv == NULL)
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
			GTK_BUTTONS_NONE, "%s", _("Logout"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s", message);
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
	*x = PANEL_BORDER_WIDTH;
	*y = panel->root_height - panel->height - req.height;
	*push_in = TRUE;
}


/* panel_helper_shutdown_dialog */
static int _panel_helper_shutdown_dialog(void)
{
	GtkWidget * dialog;
	GtkWidget * widget;
	const char * message = _("This will shutdown your computer,"
			" therefore closing any application currently opened"
			" and losing any unsaved data.\n"
			"Do you really want to proceed?");
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
