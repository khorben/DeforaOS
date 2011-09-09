/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - get rid of the code duplication in the preferences callbacks */



#include <System.h>
#include <Desktop.h>
#include <sys/stat.h>
#ifdef __NetBSD__
# include <sys/param.h>
# include <sys/sysctl.h>
#else
# include <fcntl.h>
#endif
#include <dirent.h>
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

	GdkScreen * screen;
	GdkWindow * root;
	gint root_width;		/* width of the root window	*/
	gint root_height;		/* height of the root window	*/

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_notebook;
	GtkListStore * pr_store;
	GtkWidget * pr_view;
	GtkWidget * pr_bottom_size;
	GtkListStore * pr_bottom_store;
	GtkWidget * pr_bottom_view;
	GtkWidget * pr_top_size;
	GtkListStore * pr_top_store;
	GtkWidget * pr_top_view;

	/* dialogs */
	GtkWidget * ab_window;
#ifndef EMBEDDED
	GtkWidget * lo_window;
#endif
	GtkWidget * sh_window;
};


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static char const _copyright[] =
"Copyright (c) 2011 DeforaOS Project <contact@defora.org>";

static const struct
{
	char const * name;
	char const * alias;
	GtkIconSize iconsize;
	gint size;
} _panel_sizes[] =
{
	{ "panel-large",	N_("Large"),	GTK_ICON_SIZE_LARGE_TOOLBAR,
		48 },
	{ "panel-small",	N_("Small"),	GTK_ICON_SIZE_SMALL_TOOLBAR,
		24 },
	{ "panel-smaller",	N_("Smaller"),	GTK_ICON_SIZE_MENU, 16 },
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
static int _panel_helper_lock(Panel * panel);
#ifndef EMBEDDED
static void _panel_helper_logout_dialog(Panel * panel);
#endif
static void _panel_helper_position_menu(Panel * panel, GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in, PanelPosition position);
static void _panel_helper_position_menu_bottom(Panel * panel, GtkMenu * menu,
		gint * x, gint * y, gboolean * push_in);
static void _panel_helper_position_menu_top(Panel * panel, GtkMenu * menu,
		gint * x, gint * y, gboolean * push_in);
static void _panel_helper_preferences_dialog(Panel * panel);
static void _panel_helper_shutdown_dialog(Panel * panel);
static int _panel_helper_suspend(Panel * panel);

static char * _config_get_filename(void);


/* public */
/* panel_new */
static int _new_config(Panel * panel);
static void _new_prefs(GdkScreen * screen, PanelPrefs * prefs,
		PanelPrefs const * user);
static GtkIconSize _new_size(Panel * panel, PanelPosition position);
static gboolean _on_idle(gpointer data);
static void _idle_load(Panel * panel, PanelPosition position,
		char const * plugins);
static GdkFilterReturn _on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Panel * panel_new(PanelPrefs const * prefs)
{
	Panel * panel;
	GdkRectangle rect;
	GtkIconSize iconsize;

	if((panel = object_new(sizeof(*panel))) == NULL)
		return NULL;
	_new_config(panel);
	panel->screen = gdk_screen_get_default();
	_new_prefs(panel->screen, &panel->prefs, prefs);
	prefs = &panel->prefs;
	panel->top_helper.panel = panel;
	panel->top_helper.config_get = _panel_helper_config_get;
	panel->top_helper.config_set = _panel_helper_config_set;
	panel->top_helper.error = _panel_helper_error;
	panel->top_helper.icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR;
	panel->top_helper.about_dialog = _panel_helper_about_dialog;
	panel->top_helper.lock = _panel_helper_lock;
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
	iconsize = GTK_ICON_SIZE_INVALID;
	if(prefs->iconsize != NULL)
		iconsize = gtk_icon_size_from_name(prefs->iconsize);
	switch(iconsize)
	{
		case GTK_ICON_SIZE_INVALID:
			panel->top_helper.icon_size = _new_size(panel,
					PANEL_POSITION_TOP);
			panel->bottom_helper.icon_size = _new_size(panel,
					PANEL_POSITION_BOTTOM);
			break;
		default:
			panel->top_helper.icon_size = iconsize;
			panel->bottom_helper.icon_size = iconsize;
			break;
	}
	panel->pr_window = NULL;
	panel->ab_window = NULL;
#ifndef EMBEDDED
	panel->lo_window = NULL;
#endif
	panel->sh_window = NULL;
	if(panel->config == NULL)
	{
		panel_error(NULL, error_get(), 0); /* XXX put up a dialog box */
		panel_delete(panel);
		return NULL;
	}
	/* root window */
	panel->root = gdk_screen_get_root_window(panel->screen);
	gdk_screen_get_monitor_geometry(panel->screen, (prefs->monitor > 0
				&& prefs->monitor < gdk_screen_get_n_monitors(
					panel->screen))
			? prefs->monitor : 0, &rect);
	panel->root_height = rect.height;
	panel->root_width = rect.width;
	panel->top = (config_get(panel->config, NULL, "top") != NULL)
		? panel_window_new(PANEL_POSITION_TOP, &panel->top_helper,
				&rect) : NULL;
	panel->bottom = (config_get(panel->config, NULL, "bottom") != NULL
			|| config_get(panel->config, NULL, "top") == NULL)
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

static void _new_prefs(GdkScreen * screen, PanelPrefs * prefs,
		PanelPrefs const * user)
{
	size_t i;
	gint width;
	gint height;

	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
	{
		if(gtk_icon_size_from_name(_panel_sizes[i].name)
				!= GTK_ICON_SIZE_INVALID)
			continue;
		if(gtk_icon_size_lookup(_panel_sizes[i].iconsize, &width,
					&height) != TRUE)
			width = height = _panel_sizes[i].size;
		gtk_icon_size_register(_panel_sizes[i].name, width, height);
	}
	if(user != NULL)
		memcpy(prefs, user, sizeof(*prefs));
	else
	{
		prefs->iconsize = PANEL_ICON_SIZE_DEFAULT;
		prefs->monitor = -1;
	}
#if GTK_CHECK_VERSION(2, 20, 0)
	if(prefs->monitor == -1)
		prefs->monitor = gdk_screen_get_primary_monitor(screen);
#endif
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
		ret = GTK_ICON_SIZE_SMALL_TOOLBAR;
	return ret;
}

static gboolean _on_idle(gpointer data)
{
	Panel * panel = data;
#ifndef EMBEDDED
	char const * plugins = "main,desktop,lock,logout,pager,tasks"
		",gsm,gps,bluetooth,battery,cpufreq,volume,systray,clock";
	char const * top = "main,lock,logout,separator,phone,spacer"
		",gsm,gps,bluetooth,battery,cpufreq,volume,systray,clock";
	char const * bottom = "desktop,tasks,pager";
#else /* EMBEDDED */
	char const * plugins = "main,desktop,keyboard,tasks,spacer"
		",gsm,gps,bluetooth,battery,cpufreq,volume,systray,clock,close";
	char const * top = "phone,spacer"
		",gsm,gps,bluetooth,battery,cpufreq,volume,systray,clock,close";
	char const * bottom = "main,keyboard,desktop,tasks";
#endif
	char const * p;

	panel_show_preferences(panel, FALSE);
	if((p = config_get(panel->config, NULL, "plugins")) != NULL)
		plugins = p;
	if(panel->top != NULL)
		if((p = config_get(panel->config, NULL, "top")) != NULL
				|| (p = top) != NULL)
			_idle_load(panel, PANEL_POSITION_TOP, p);
	if(panel->bottom != NULL)
		if((p = config_get(panel->config, NULL, "bottom")) != NULL
				|| (p = (panel->top != NULL) ? bottom : plugins)
				!= NULL)
			_idle_load(panel, PANEL_POSITION_BOTTOM, p);
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
	GtkWidget * vbox;

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
	panel_window_append(window, widget, pa->expand, pa->fill);
	if(pa->settings != NULL
			&& (widget = pa->settings(pa, FALSE, FALSE)) != NULL)
	{
		vbox = gtk_vbox_new(FALSE, 4);
		g_object_set_data(G_OBJECT(vbox), "applet", pa); /* XXX ugly */
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		gtk_widget_show(vbox);
		gtk_notebook_append_page(GTK_NOTEBOOK(panel->pr_notebook),
				vbox, gtk_label_new(pa->name));
	}
	return 0;
}


/* panel_show_preferences */
static void _show_preferences_window(Panel * panel);
static GtkWidget * _preferences_window_general(Panel * panel);
static GtkListStore * _preferences_window_general_model(void);
static GtkWidget * _preferences_window_general_view(GtkListStore * store);
static void _preferences_window_general_plugin_add(GtkListStore * store,
		char const * name);
static void _preferences_on_bottom_add(gpointer data);
static void _preferences_on_bottom_down(gpointer data);
static void _preferences_on_bottom_remove(gpointer data);
static void _preferences_on_bottom_up(gpointer data);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _cancel_plugins(Panel * panel);
static void _preferences_on_ok(gpointer data);
static void _preferences_on_top_add(gpointer data);
static void _preferences_on_top_down(gpointer data);
static void _preferences_on_top_remove(gpointer data);
static void _preferences_on_top_up(gpointer data);

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
	GtkWidget * widget;

	panel->pr_window = gtk_dialog_new_with_buttons(_("Panel preferences"),
			NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	gtk_window_set_default_size(GTK_WINDOW(panel->pr_window), 400, 300);
	g_signal_connect_swapped(G_OBJECT(panel->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), panel);
	g_signal_connect(G_OBJECT(panel->pr_window), "response",
			G_CALLBACK(_preferences_on_response), panel);
	panel->pr_notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(panel->pr_notebook), TRUE);
	/* general */
	widget = _preferences_window_general(panel);
	gtk_notebook_append_page(GTK_NOTEBOOK(panel->pr_notebook), widget,
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

static GtkWidget * _preferences_window_general(Panel * panel)
{
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * vbox2;
	GtkWidget * vbox3;
	GtkWidget * hbox;
	GtkWidget * frame;
	GtkWidget * widget;
	size_t i;

	/* FIXME this needs a restart to apply */
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	/* plugins */
	frame = gtk_frame_new(_("Plug-ins:"));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	panel->pr_store = _preferences_window_general_model();
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(panel->pr_store),
			2, GTK_SORT_ASCENDING);
	panel->pr_view = _preferences_window_general_view(panel->pr_store);
	gtk_container_add(GTK_CONTAINER(widget), panel->pr_view);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
	/* controls */
	vbox2 = gtk_vbox_new(FALSE, 4);
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_top_add), panel);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_top_up), panel);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_top_down), panel);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_top_remove), panel);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	widget = gtk_label_new(NULL);
	gtk_box_pack_end(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_DELETE, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_bottom_remove), panel);
	gtk_box_pack_end(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_bottom_down), panel);
	gtk_box_pack_end(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_bottom_up), panel);
	gtk_box_pack_end(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_stock(
				GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_preferences_on_bottom_add), panel);
	gtk_box_pack_end(GTK_BOX(vbox2), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, TRUE, 0);
	vbox2 = gtk_vbox_new(FALSE, 4);
	/* top plug-ins */
	frame = gtk_frame_new(_("Top panel:"));
	vbox3 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	panel->pr_top_size = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(panel->pr_top_size), NULL,
			_("Default"));
#else
	panel->pr_top_size = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_top_size),
			_("Default"));
#endif
	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(
					panel->pr_top_size), NULL,
				_(_panel_sizes[i].alias));
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_top_size),
				_(_panel_sizes[i].alias));
#endif
	gtk_box_pack_start(GTK_BOX(vbox3), panel->pr_top_size, FALSE, TRUE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	panel->pr_top_store = _preferences_window_general_model();
	panel->pr_top_view = _preferences_window_general_view(
			panel->pr_top_store);
	gtk_container_add(GTK_CONTAINER(widget), panel->pr_top_view);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox3);
	gtk_box_pack_start(GTK_BOX(vbox2), frame, TRUE, TRUE, 0);
	/* bottom plug-ins */
	frame = gtk_frame_new(_("Bottom panel:"));
	vbox3 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox3), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	panel->pr_bottom_size = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(panel->pr_bottom_size),
			NULL, _("Default"));
#else
	panel->pr_bottom_size = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_bottom_size),
			_("Default"));
#endif
	for(i = 0; i < sizeof(_panel_sizes) / sizeof(*_panel_sizes); i++)
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(
					panel->pr_bottom_size), NULL,
				_(_panel_sizes[i].alias));
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(panel->pr_bottom_size),
				_(_panel_sizes[i].alias));
#endif
	gtk_box_pack_start(GTK_BOX(vbox3), panel->pr_bottom_size, FALSE, TRUE,
			0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	panel->pr_bottom_store = _preferences_window_general_model();
	panel->pr_bottom_view = _preferences_window_general_view(
			panel->pr_bottom_store);
	gtk_container_add(GTK_CONTAINER(widget), panel->pr_bottom_view);
	gtk_box_pack_start(GTK_BOX(vbox3), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox3);
	gtk_box_pack_start(GTK_BOX(vbox2), frame, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	return vbox;
}

static GtkListStore * _preferences_window_general_model(void)
{
	GtkListStore * store;

	store = gtk_list_store_new(3, G_TYPE_STRING, GDK_TYPE_PIXBUF,
			G_TYPE_STRING);
	return store;
}

static GtkWidget * _preferences_window_general_view(GtkListStore * store)
{
	GtkWidget * view;
	GtkTreeSelection * treesel;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer,
			"pixbuf", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer,
			"text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
	return view;
}

static void _preferences_window_general_plugin_add(GtkListStore * store,
		char const * name)
{
	Plugin * p;
	PanelApplet * pa;
	GtkTreeIter iter;
	GtkIconTheme * theme;
	GdkPixbuf * pixbuf;

	if((p = plugin_new(LIBDIR, PACKAGE, "applets", name)) == NULL)
		return;
	if((pa = plugin_lookup(p, "applet")) == NULL)
	{
		plugin_delete(p);
		return;
	}
	theme = gtk_icon_theme_get_default();
	pixbuf = (pa->icon != NULL) ? gtk_icon_theme_load_icon(theme, pa->icon,
			24, 0, NULL) : NULL;
	if(pixbuf == NULL)
		pixbuf = gtk_icon_theme_load_icon(theme, "gnome-settings", 24,
				0, NULL);
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, name, 1, pixbuf, 2, pa->name, -1);
	plugin_delete(p);
}

static void _preferences_on_bottom_add(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeSelection * treesel;
	gchar * p;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(panel->pr_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, 0, &p, -1);
	_preferences_window_general_plugin_add(panel->pr_bottom_store, p);
	g_free(p);
}

static void _preferences_on_bottom_down(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeIter iter2;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_bottom_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	iter2 = iter;
	if(!gtk_tree_model_iter_next(model, &iter))
		return;
	gtk_list_store_swap(panel->pr_bottom_store, &iter, &iter2);
}

static void _preferences_on_bottom_remove(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_bottom_view));
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void _preferences_on_bottom_up(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeIter iter2;
	GtkTreePath * path;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_bottom_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_path_prev(path);
	gtk_tree_model_get_iter(model, &iter2, path);
	gtk_tree_path_free(path);
	gtk_list_store_swap(panel->pr_bottom_store, &iter, &iter2);
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
	size_t cnt = sizeof(_panel_sizes) / sizeof(*_panel_sizes);
	GtkWidget * widget;
	PanelApplet * pa;

	gtk_widget_hide(panel->pr_window);
	_cancel_plugins(panel);
	if((p = config_get(panel->config, NULL, "bottom_size")) == NULL
			&& (p = config_get(panel->config, "", "size")) == NULL)
		gtk_combo_box_set_active(GTK_COMBO_BOX(panel->pr_bottom_size),
				0);
	else
		for(i = 0; i < cnt; i++)
		{
			if(strcmp(p, _panel_sizes[i].name) != 0)
				continue;
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						panel->pr_bottom_size), i + 1);
			break;
		}
	if((p = config_get(panel->config, NULL, "top_size")) == NULL
			&& (p = config_get(panel->config, "", "size")) == NULL)
		gtk_combo_box_set_active(GTK_COMBO_BOX(panel->pr_top_size), 0);
	else
		for(i = 0; i < cnt; i++)
		{
			if(strcmp(p, _panel_sizes[i].name) != 0)
				continue;
			gtk_combo_box_set_active(GTK_COMBO_BOX(
						panel->pr_top_size), i + 1);
			break;
		}
	/* XXX applets should be known from Panel already */
	cnt = gtk_notebook_get_n_pages(GTK_NOTEBOOK(panel->pr_notebook));
	for(i = 1; i < cnt; i++)
	{
		widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(
					panel->pr_notebook), i);
		if(widget == NULL || (pa = g_object_get_data(G_OBJECT(widget),
						"applet")) == NULL)
			continue;
		pa->settings(pa, FALSE, TRUE);
	}
}

static void _cancel_plugins(Panel * panel)
{
	DIR * dir;
	struct dirent * de;
	char const ext[] = ".so";
	size_t len;
	char * q;
	char const * r;
	char c;
	size_t i;

	gtk_list_store_clear(panel->pr_store);
	gtk_list_store_clear(panel->pr_bottom_store);
	gtk_list_store_clear(panel->pr_top_store);
	if((dir = opendir(LIBDIR "/" PACKAGE "/applets")) == NULL)
		return;
	/* plug-ins */
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name)) < sizeof(ext))
			continue;
		if(strcmp(&de->d_name[len - sizeof(ext) + 1], ext) != 0)
			continue;
		de->d_name[len - sizeof(ext) + 1] = '\0';
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, de->d_name);
#endif
		_preferences_window_general_plugin_add(panel->pr_store,
				de->d_name);
	}
	closedir(dir);
	/* top panel */
	r = config_get(panel->config, NULL, "top");
	q = (r != NULL) ? strdup(r) : NULL;
	for(i = 0, r = q; q != NULL; i++)
	{
		if(q[i] != '\0' && q[i] != ',')
			continue;
		c = q[i];
		q[i] = '\0';
		_preferences_window_general_plugin_add(panel->pr_top_store, r);
		if(c == '\0')
			break;
		r = &q[i + 1];
	}
	free(q);
	/* bottom panel */
	r = config_get(panel->config, NULL, "bottom");
	q = (r != NULL) ? strdup(r) : NULL;
	for(i = 0, r = q; q != NULL; i++)
	{
		if(q[i] != '\0' && q[i] != ',')
			continue;
		c = q[i];
		q[i] = '\0';
		_preferences_window_general_plugin_add(panel->pr_bottom_store,
				r);
		if(c == '\0')
			break;
		r = &q[i + 1];
	}
	free(q);
}

static void _preferences_on_ok(gpointer data)
{
	Panel * panel = data;
	gint i;
	gint cnt = sizeof(_panel_sizes) / sizeof(*_panel_sizes);
	GtkTreeModel * model;
	GtkTreeIter iter;
	gboolean valid;
	gchar * p;
	String * value;
	String * sep;
	char * filename;
	GtkWidget * widget;
	PanelApplet * pa;

	gtk_widget_hide(panel->pr_window);
	/* top panel */
	if((i = gtk_combo_box_get_active(GTK_COMBO_BOX(panel->pr_top_size)))
			>= 0 && i <= cnt)
		config_set(panel->config, NULL, "top_size", (i > 0)
				? _panel_sizes[i - 1].name : NULL);
	model = GTK_TREE_MODEL(panel->pr_top_store);
	value = NULL;
	sep = "";
	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, 0, &p, -1);
		string_append(&value, sep);
		string_append(&value, p);
		sep = ",";
		g_free(p);
	}
	config_set(panel->config, NULL, "top", value);
	string_delete(value);
	/* bottom panel */
	if((i = gtk_combo_box_get_active(GTK_COMBO_BOX(panel->pr_bottom_size)))
			>= 0 && i <= cnt)
		config_set(panel->config, NULL, "bottom_size", (i > 0)
				? _panel_sizes[i - 1].name : NULL);
	model = GTK_TREE_MODEL(panel->pr_bottom_store);
	value = NULL;
	sep = "";
	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, 0, &p, -1);
		string_append(&value, sep);
		string_append(&value, p);
		sep = ",";
		g_free(p);
	}
	config_set(panel->config, NULL, "bottom", value);
	string_delete(value);
	/* XXX applets should be known from Panel already */
	cnt = gtk_notebook_get_n_pages(GTK_NOTEBOOK(panel->pr_notebook));
	for(i = 1; i < cnt; i++)
	{
		widget = gtk_notebook_get_nth_page(GTK_NOTEBOOK(
					panel->pr_notebook), i);
		if(widget == NULL || (pa = g_object_get_data(G_OBJECT(widget),
						"applet")) == NULL)
			continue;
		pa->settings(pa, TRUE, FALSE);
	}
	if((filename = _config_get_filename()) != NULL)
		config_save(panel->config, filename);
	free(filename);
}

static void _preferences_on_top_add(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeSelection * treesel;
	gchar * p;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(panel->pr_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	gtk_tree_model_get(model, &iter, 0, &p, -1);
	_preferences_window_general_plugin_add(panel->pr_top_store, p);
	g_free(p);
}

static void _preferences_on_top_down(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeIter iter2;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_top_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	iter2 = iter;
	if(!gtk_tree_model_iter_next(model, &iter))
		return;
	gtk_list_store_swap(panel->pr_top_store, &iter, &iter2);
}

static void _preferences_on_top_remove(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_top_view));
	if(gtk_tree_selection_get_selected(treesel, &model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void _preferences_on_top_up(gpointer data)
{
	Panel * panel = data;
	GtkTreeModel * model;
	GtkTreeIter iter;
	GtkTreeIter iter2;
	GtkTreePath * path;
	GtkTreeSelection * treesel;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				panel->pr_top_view));
	if(!gtk_tree_selection_get_selected(treesel, &model, &iter))
		return;
	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_path_prev(path);
	gtk_tree_model_get_iter(model, &iter2, path);
	gtk_tree_path_free(path);
	gtk_list_store_swap(panel->pr_top_store, &iter, &iter2);
}


/* private */
/* functions */
/* config_get_filename */
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

	if(access("/sys/power/state", W_OK) == 0)
		return TRUE;
	if(lstat("/proc/apm", &st) == 0)
		return TRUE;
#endif
	return FALSE;
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
	desktop_about_dialog_set_program_name(panel->ab_window, PACKAGE);
	desktop_about_dialog_set_translator_credits(panel->ab_window,
			_("translator-credits"));
	desktop_about_dialog_set_version(panel->ab_window, VERSION);
	desktop_about_dialog_set_website(panel->ab_window,
			"http://www.defora.org/");
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


/* panel_helper_lock */
static int _panel_helper_lock(Panel * panel)
{
	/* FIXME default to calling XActivateScreenSaver() */
	char const * command = "xset s activate";
	char const * p;
	GError * error = NULL;

	if((p = config_get(panel->config, "lock", "command")) != NULL)
		command = p;
	if(g_spawn_command_line_async(command, &error) != TRUE)
		return panel_error(panel, error->message, 1);
	return 0;
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
	panel->lo_window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
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
static void _panel_helper_position_menu(Panel * panel, GtkMenu * menu, gint * x,
		gint * y, gboolean * push_in, PanelPosition position)
{
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
static void _panel_helper_position_menu_bottom(Panel * panel, GtkMenu * menu,
		gint * x, gint * y, gboolean * push_in)
{
	_panel_helper_position_menu(panel, menu, x, y, push_in,
			PANEL_POSITION_BOTTOM);
}


/* panel_helper_position_menu_top */
static void _panel_helper_position_menu_top(Panel * panel, GtkMenu * menu,
		gint * x, gint * y, gboolean * push_in)
{
	_panel_helper_position_menu(panel, menu, x, y, push_in,
			PANEL_POSITION_TOP);
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
	panel->sh_window = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_QUESTION,
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
	g_signal_connect(G_OBJECT(panel->sh_window), "delete-event", G_CALLBACK(
				_shutdown_dialog_on_closex), panel);
	g_signal_connect(G_OBJECT(panel->sh_window), "response", G_CALLBACK(
				_shutdown_dialog_on_response), panel);
	gtk_widget_show_all(panel->sh_window);
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
	int fd;
	char * suspend[] = { "/usr/bin/sudo", "sudo", "/usr/bin/apm", "-s",
		NULL };
	GError * error = NULL;
#endif

#ifdef __NetBSD__
	if(sysctlbyname("machdep.sleep_state", NULL, NULL, &sleep_state,
				sizeof(sleep_state)) != 0)
		return panel_error(panel, "sysctl", 1);
#else
	if((fd = open("/sys/power/state", O_WRONLY)) >= 0)
	{
		write(fd, "mem\n", 4);
		close(fd);
		return 0;
	}
	if(g_spawn_async(NULL, suspend, NULL, G_SPAWN_FILE_AND_ARGV_ZERO, NULL,
				NULL, NULL, &error) != TRUE)
		return panel_error(panel, error->message, 1);
#endif
	_panel_helper_lock(panel); /* XXX may already be suspended */
	return 0;
}
