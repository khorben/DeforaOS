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
#ifdef __NetBSD__
# include <sys/types.h>
# include <sys/statvfs.h>
#endif
#include "Browser.h"


/* Devices */
/* private */
/* types */
typedef struct _Devices
{
	guint source;
	GtkWidget * window;
	GtkListStore * store;
	GtkWidget * view;
	GdkPixbuf * folder;
} Devices;


/* prototypes */
static GtkWidget * _devices_init(BrowserPlugin * plugin);
static void _devices_destroy(BrowserPlugin * plugin);
static void _devices_refresh(BrowserPlugin * plugin);

/* callbacks */
static gboolean _devices_on_idle(gpointer data);
static void _devices_on_selection_changed(gpointer data);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	"Devices",
	_devices_init,
	_devices_destroy,
	_devices_refresh,
	NULL
};


/* private */
/* functions */
/* devices_init */
static GtkWidget * _devices_init(BrowserPlugin * plugin)
{
	Devices * devices;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	GtkIconTheme * icontheme;
	gint width;
	gint height;

	if((devices = object_new(sizeof(*devices))) == NULL)
		return NULL;
	plugin->priv = devices;
	devices->window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(devices->window),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	devices->store = gtk_list_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_STRING);
	devices->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				devices->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(devices->view), FALSE);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"pixbuf", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(devices->view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(devices->view), column);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(devices->view));
	g_signal_connect_swapped(G_OBJECT(treesel), "changed", G_CALLBACK(
				_devices_on_selection_changed), plugin);
	gtk_container_add(GTK_CONTAINER(devices->window), devices->view);
	icontheme = gtk_icon_theme_get_default();
	gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON, &width, &height);
	devices->folder = gtk_icon_theme_load_icon(icontheme, "folder-remote",
			width, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_widget_show_all(devices->window);
	devices->source = g_idle_add(_devices_on_idle, plugin);
	return devices->window;
}


/* devices_destroy */
static void _devices_destroy(BrowserPlugin * plugin)
{
	Devices * devices = plugin->priv;

	if(devices->source != 0)
		g_source_remove(devices->source);
	gtk_widget_destroy(devices->view);
	object_delete(devices);
}


/* devices_refresh */
static void _devices_refresh(BrowserPlugin * plugin)
{
	Devices * devices = plugin->priv;
	GtkTreeIter iter;
	struct statvfs * mnt;
	int res;
	int i;

	gtk_list_store_clear(devices->store);
#ifdef __NetBSD__
	if((res = getmntinfo(&mnt, ST_WAIT)) <= 0)
		return;
	for(i = 0; i < res; i++)
	{
		gtk_list_store_append(devices->store, &iter);
		gtk_list_store_set(devices->store, &iter, 0, devices->folder,
				1, mnt[i].f_mntonname, 2, mnt[i].f_mntonname,
				-1);
	}
#else
	gtk_list_store_append(devices->store, &iter);
	gtk_list_store_set(devices->store, &iter, 0, "Root filesystem", 1, "/",
			-1);
#endif
}


/* callbacks */
/* devices_on_idle */
static gboolean _devices_on_idle(gpointer data)
{
	BrowserPlugin * plugin = data;
	Devices * devices = plugin->priv;

	devices->source = 0;
	_devices_refresh(plugin);
	return FALSE;
}


/* devices_on_selection_changed */
static void _devices_on_selection_changed(gpointer data)
{
	BrowserPlugin * plugin = data;
	Devices * devices = plugin->priv;
	GtkTreeSelection * treesel;
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * location;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(devices->view));
	if(gtk_tree_selection_get_selected(treesel, &model, &iter) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, 1, &location, -1);
	plugin->helper->set_location(plugin->helper->browser, location);
	g_free(location);
}
