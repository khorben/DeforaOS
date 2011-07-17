/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
#include "Browser.h"


/* Devices */
/* private */
/* types */
typedef struct _Devices
{
	GtkWidget * window;
	GtkWidget * view;
} Devices;


/* prototypes */
static GtkWidget * _devices_init(BrowserPlugin * plugin);
static void _devices_destroy(BrowserPlugin * plugin);

/* callbacks */
static void _devices_on_selection_changed(gpointer data);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	"Devices",
	_devices_init,
	_devices_destroy,
	NULL
};


/* private */
/* functions */
/* devices_init */
static GtkWidget * _devices_init(BrowserPlugin * plugin)
{
	Devices * devices;
	GtkTreeSelection * treesel;

	if((devices = object_new(sizeof(*devices))) == NULL)
		return NULL;
	plugin->priv = devices;
	devices->window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(devices->window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	devices->view = gtk_tree_view_new();
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(devices->view));
	g_signal_connect_swapped(G_OBJECT(treesel), "changed", G_CALLBACK(
				_devices_on_selection_changed), plugin);
	gtk_container_add(GTK_CONTAINER(devices->window), devices->view);
	gtk_widget_show_all(devices->window);
	return devices->window;
}


/* devices_destroy */
static void _devices_destroy(BrowserPlugin * plugin)
{
	Devices * devices = plugin->priv;

	gtk_widget_destroy(devices->view);
	object_delete(devices);
}


/* callbacks */
/* devices_on_selection_changed */
static void _devices_on_selection_changed(gpointer data)
{
	BrowserPlugin * plugin = data;

	/* FIXME really implement */
	plugin->helper->set_location(plugin->helper->browser, "/mnt");
}
