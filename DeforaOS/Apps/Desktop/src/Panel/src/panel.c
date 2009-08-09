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



#include <System.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "panel.h"
#include "common.h"
#include "../config.h"


/* Panel */
/* private */
/* types */
struct _Panel
{
	PanelAppletHelper helper;
	GdkWindow * root;
	GtkWidget * window;
	GtkWidget * hbox;

	gint width;			/* width of the root window	*/
	gint height;			/* height of the root window	*/
};


/* constants */
#ifndef PREFIX
# define PREFIX "/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR PREFIX "/lib"
#endif


/* prototypes */
/* helpers */
static int _panel_logout_dialog(void);
static void _panel_position_menu(GtkMenu * menu, gint * x, gint * y,
		gboolean * push_in, gpointer data);


/* public */
/* panel_new */
static gboolean _on_idle(gpointer data);
static gboolean _on_button_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data);
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

Panel * panel_new(void)
{
	Panel * panel;
	GtkWidget * event;
	gint x;
	gint y;
	gint depth;

	if((panel = malloc(sizeof(*panel))) == NULL)
	{
		/* FIXME visually warn the user */
		panel_error(NULL, "malloc", 1);
		return NULL;
	}
	panel->helper.priv = panel;
	panel->helper.logout_dialog = _panel_logout_dialog;
	panel->helper.position_menu = _panel_position_menu;
	/* root window */
	panel->root = gdk_screen_get_root_window(
			gdk_display_get_default_screen(
				gdk_display_get_default()));
	gdk_window_get_geometry(panel->root, &x, &y, &panel->width,
			&panel->height, &depth);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() x=%d y=%d width=%d height=%d depth=%d\n",
			__func__, x, y, panel->width, panel->height, depth);
#endif
	/* panel */
	g_idle_add(_on_idle, panel);
	panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(panel->window), panel->width,
			PANEL_ICON_SIZE + (PANEL_BORDER_WIDTH * 2));
	gtk_window_set_type_hint(GTK_WINDOW(panel->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_move(GTK_WINDOW(panel->window), 0, panel->height
		- PANEL_ICON_SIZE - (PANEL_BORDER_WIDTH * 2));
	g_signal_connect(G_OBJECT(panel->window), "delete-event", G_CALLBACK(
				_on_closex), panel);
	event = gtk_event_box_new();
	g_signal_connect(G_OBJECT(event), "button-press-event", G_CALLBACK(
				_on_button_press), panel);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(event), panel->hbox);
	gtk_container_add(GTK_CONTAINER(panel->window), event);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_widget_show_all(panel->window);
	return panel;
}

static gboolean _on_idle(gpointer data)
{
	Panel * panel = data;
	/* FIXME load all plugins, a configuration file or ask the user */
#ifndef EMBEDDED
	const char * plugins[] = { "cpu", "clock", "desktop", "lock", "logout",
		"main", "memory", "pager", "tasks", NULL };
#else
	const char * plugins[] = { "clock", "desktop", "main", "tasks", NULL };
#endif
	size_t i;
	Plugin * plugin;
	PanelApplet * applet;
	GtkWidget * widget;

	for(i = 0; plugins[i] != NULL; i++)
		if((plugin = plugin_new(LIBDIR, PACKAGE, "applets", plugins[i]))
				!= NULL
				&& (applet = plugin_lookup(plugin, "applet"))
				!= NULL
				&& (applet->helper = &panel->helper) != NULL
				&& applet->init != NULL
				&& (widget = applet->init(applet)) != NULL)
		{
			switch(applet->position)
			{
				case PANEL_APPLET_POSITION_END:
					gtk_box_pack_end(GTK_BOX(panel->hbox),
							widget, FALSE, TRUE, 2);
					break;
				case PANEL_APPLET_POSITION_FIRST:
					gtk_box_pack_start(GTK_BOX(panel->hbox),
							widget, FALSE, TRUE, 2);
					gtk_box_reorder_child(GTK_BOX(
								panel->hbox),
							widget, 0);
					break;
				case PANEL_APPLET_POSITION_LAST:
					gtk_box_pack_end(GTK_BOX(panel->hbox),
							widget, FALSE, TRUE, 2);
					gtk_box_reorder_child(GTK_BOX(
								panel->hbox),
							widget, 0);
					break;
				case PANEL_APPLET_POSITION_START:
					gtk_box_pack_start(GTK_BOX(panel->hbox),
							widget, FALSE, TRUE, 2);
					break;
			}
			gtk_widget_show_all(widget);
		}
		else
			error_print(PACKAGE);
	return FALSE;
}

static gboolean _on_button_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	GtkWidget * menu;
	GtkWidget * menuitem;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(event->type != GDK_BUTTON_PRESS || event->button != 3)
		return FALSE;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() right-click\n", __func__);
#endif
	menu = gtk_menu_new();
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PROPERTIES,
			NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, data, event->button,
			event->time);
	return FALSE;
}

static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	return TRUE;
}


/* panel_delete */
void panel_delete(Panel * panel)
{
	/* FIXME destroy plugins */
	free(panel);
}


/* useful */
static int _error_text(char const * message, int ret);

int panel_error(Panel * panel, char const * message, int ret)
{
	GtkWidget * dialog;

	if(panel == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs("panel: ", stderr);
	perror(message);
	return ret;
}


/* private */
/* functions */
/* helpers */
/* panel_logout_dialog */
static int _panel_logout_dialog(void)
{
	GtkWidget * dialog;
	const char message[] = "This will log you out of the current session,"
		" therefore closing any application currently opened and losing"
		" any unsaved data.\nDo you really want to proceed?";
	int res;

	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_WARNING,
			GTK_BUTTONS_NONE, "%s", message);
	gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, "Logout", GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_window_set_keep_above(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res != GTK_RESPONSE_ACCEPT)
		return 1;
	gtk_main_quit();
	return 0;
}


/* panel_position_menu */
static void _panel_position_menu(GtkMenu * menu, gint * x, gint * y,
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
	*y = panel->height - PANEL_BORDER_WIDTH - PANEL_ICON_SIZE - req.height;
	*push_in = TRUE;
}
