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
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Dirtree */
/* private */
/* types */
typedef struct _Dirtree
{
	GdkPixbuf * folder;
	GtkTreeStore * store;
	GtkWidget * view;
} Dirtree;


/* prototypes */
static GtkWidget * _dirtree_init(BrowserPlugin * plugin);
static void _dirtree_destroy(BrowserPlugin * plugin);
static void _dirtree_refresh(BrowserPlugin * plugin, char const * path);

/* callbacks */
static void _dirtree_on_selection_changed(gpointer data);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	N_("Directory tree"),
	"stock_folder",
	_dirtree_init,
	_dirtree_destroy,
	_dirtree_refresh,
	NULL
};


/* private */
/* functions */
/* dirtree_init */
static GtkWidget * _dirtree_init(BrowserPlugin * plugin)
{
	Dirtree * dirtree;
	BrowserPluginHelper * helper = plugin->helper;
	GtkIconTheme * icontheme;
	GError * error = NULL;
	GtkWidget * widget;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gint size;

	if((dirtree = object_new(sizeof(*dirtree))) == NULL)
		return NULL;
	plugin->priv = dirtree;
	icontheme = gtk_icon_theme_get_default();
	gtk_icon_size_lookup(GTK_ICON_SIZE_BUTTON, &size, &size);
	dirtree->folder = gtk_icon_theme_load_icon(icontheme, "stock_folder",
			size, GTK_ICON_LOOKUP_USE_BUILTIN, &error);
	if(dirtree->folder == NULL)
	{
		helper->error(helper->browser, error->message, 1);
		g_error_free(error);
	}
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	dirtree->store = gtk_tree_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_STRING);
	gtk_tree_store_insert(dirtree->store, &iter, NULL, -1);
	gtk_tree_store_set(dirtree->store, &iter, 0, dirtree->folder, 1, "/",
			2, "/", -1);
	dirtree->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				dirtree->store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dirtree->view), FALSE);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"pixbuf", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dirtree->view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dirtree->view), column);
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirtree->view));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	g_signal_connect_swapped(treesel, "changed", G_CALLBACK(
				_dirtree_on_selection_changed), plugin);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),
			dirtree->view);
	gtk_widget_show_all(widget);
	return widget;
}


/* dirtree_destroy */
static void _dirtree_destroy(BrowserPlugin * plugin)
{
	Dirtree * dirtree = plugin->priv;

	g_object_unref(dirtree->folder);
	object_delete(dirtree);
}


/* dirtree_refresh */
static void _dirtree_refresh(BrowserPlugin * plugin, char const * path)
{
	Dirtree * dirtree = plugin->priv;

	/* FIXME implement */
}


/* callbacks */
/* dirtree_on_selection_changed */
static void _dirtree_on_selection_changed(gpointer data)
{
	BrowserPlugin * plugin = data;
	Dirtree * dirtree = plugin->priv;
	GtkTreeSelection * treesel;
	GtkTreeModel * model;
	GtkTreeIter iter;
	gchar * location;

	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirtree->view));
	if(gtk_tree_selection_get_selected(treesel, &model, &iter) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, 2, &location, -1);
	plugin->helper->set_location(plugin->helper->browser, location);
	g_free(location);
}
