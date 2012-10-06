/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <System.h>
#include <Desktop.h>
#include "../src/panel.h"
#include "../config.h"

#ifdef PACKAGE
# undef PACKAGE
#endif
#define PACKAGE "panel-notify"
#include "helper.c"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif


/* private */
/* prototypes */
static int _notify(GtkIconSize iconsize, int timeout, char * applets[]);
/* callbacks */
static gboolean _notify_on_timeout(gpointer data);

static int _usage(void);


/* functions */
/* notify */
static int _notify(GtkIconSize iconsize, int timeout, char * applets[])
{
	Panel panel;
	char * filename;
	GtkWidget * box;
	GtkWidget * widget;
	size_t i;
	Plugin * plugin;
	PanelAppletHelper helper;
	PanelAppletDefinition * pad;
	PanelApplet * pa;

	if((panel.config = config_new()) == NULL)
		return error_print("panel-notify");
	if((filename = _config_get_filename()) != NULL
			&& config_load(panel.config, filename) != 0)
		error_print("panel-notify");
	free(filename);
	panel.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_window_set_has_resize_grip(GTK_WINDOW(panel.window), FALSE);
#endif
	gtk_container_set_border_width(GTK_CONTAINER(panel.window), 4);
	gtk_window_set_accept_focus(GTK_WINDOW(panel.window), FALSE);
	gtk_window_set_decorated(GTK_WINDOW(panel.window), FALSE);
	gtk_window_set_position(GTK_WINDOW(panel.window),
			GTK_WIN_POS_CENTER_ALWAYS);
	g_signal_connect(G_OBJECT(panel.window), "delete-event", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_window_set_title(GTK_WINDOW(panel.window), "Applet notifier");
	box = gtk_hbox_new(FALSE, 4);
	_helper_init(&helper, &panel, PANEL_APPLET_TYPE_NOTIFICATION, iconsize);
	for(i = 0; applets[i] != NULL; i++)
	{
		if((plugin = plugin_new(LIBDIR, "Panel", "applets", applets[i]))
				== NULL)
		{
			error_print(PACKAGE);
			continue;
		}
		if((pad = plugin_lookup(plugin, "applet")) == NULL)
		{
			plugin_delete(plugin);
			continue;
		}
		widget = NULL;
		if((pa = pad->init(&helper, &widget)) != NULL
				&& widget != NULL)
			gtk_box_pack_start(GTK_BOX(box), widget, pad->expand,
					pad->fill, 0);
	}
	gtk_container_add(GTK_CONTAINER(panel.window), box);
	gtk_widget_show_all(panel.window);
	panel.timeout = 0;
	if(timeout > 0)
		panel.timeout = g_timeout_add(timeout * 1000,
				_notify_on_timeout, &panel);
	gtk_main();
	return 0;
}


/* callbacks */
/* notify_on_timeout */
static gboolean _notify_on_timeout(gpointer data)
{
	Panel * panel = data;

	panel->timeout = 0;
	gtk_main_quit();
	return FALSE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: panel-notify [-L|-S|-X|-x][-t timeout] applet...\n"
"       panel-notify -l\n"
"  -L	Use icons the size of a large toolbar\n"
"  -S	Use icons the size of a small toolbar\n"
"  -X	Use huge icons\n"
"  -x	Use icons the size of menus\n"
"  -t	Time to wait before disappearing (0: unlimited)\n"
"  -l	Lists the plug-ins available\n", stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	GtkIconSize iconsize;
	GtkIconSize huge;
	int timeout = 3;
	int o;
	char * p;

	gtk_init(&argc, &argv);
	if((huge = gtk_icon_size_from_name("panel-huge"))
			== GTK_ICON_SIZE_INVALID)
		huge = gtk_icon_size_register("panel-huge", 64, 64);
	iconsize = huge;
	while((o = getopt(argc, argv, "LlSt:Xx")) != -1)
		switch(o)
		{
			case 'L':
				iconsize = GTK_ICON_SIZE_LARGE_TOOLBAR;
				break;
			case 'l':
				return _applet_list();
			case 'S':
				iconsize = GTK_ICON_SIZE_SMALL_TOOLBAR;
				break;
			case 't':
				timeout = strtoul(optarg, &p, 0);
				if(optarg[0] == '\0' || *p != '\0'
						|| timeout < 0)
					return _usage();
				break;
			case 'X':
				iconsize = huge;
				break;
			case 'x':
				iconsize = GTK_ICON_SIZE_MENU;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	_notify(iconsize, timeout, &argv[optind]);
	return 0;
}
