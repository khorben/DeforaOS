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
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include "Browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Dirtree */
/* private */
/* types */
typedef struct _Dirtree
{
	guint source;
	GdkPixbuf * folder;
	GtkTreeStore * store;
	GtkTreeModel * sorted;
	GtkWidget * view;
} Dirtree;


/* prototypes */
static GtkWidget * _dirtree_init(BrowserPlugin * plugin);
static void _dirtree_destroy(BrowserPlugin * plugin);

static void _dirtree_refresh_folder(BrowserPlugin * plugin,
		GtkTreeIter * parent, char const * path, gboolean recurse);

/* callbacks */
static gboolean _dirtree_on_idle(gpointer data);
static void _dirtree_on_row_activated(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
static void _dirtree_on_row_expanded(GtkTreeView * view, GtkTreeIter * iter,
		GtkTreePath * path, gpointer data);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	N_("Directory tree"),
	"stock_folder",
	_dirtree_init,
	_dirtree_destroy,
	NULL,
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
	dirtree->source = 0;
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
	/* backend store */
	dirtree->store = gtk_tree_store_new(4, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_BOOLEAN);
	gtk_tree_store_insert(dirtree->store, &iter, NULL, -1);
	gtk_tree_store_set(dirtree->store, &iter, 0, dirtree->folder, 1, "/",
			2, "/", 3, TRUE, -1);
	/* sorted store */
	dirtree->sorted = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(
				dirtree->store));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(dirtree->sorted),
			1, GTK_SORT_ASCENDING);
	/* view */
	dirtree->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				dirtree->sorted));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dirtree->view), FALSE);
	/* columns */
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"pixbuf", 0, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dirtree->view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(NULL, renderer,
			"text", 1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(dirtree->view), column);
	/* selection */
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirtree->view));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	/* signals */
	g_signal_connect(dirtree->view, "row-activated", G_CALLBACK(
				_dirtree_on_row_activated), plugin);
	g_signal_connect(dirtree->view, "row-expanded", G_CALLBACK(
				_dirtree_on_row_expanded), plugin);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),
			dirtree->view);
	gtk_widget_show_all(widget);
	/* populate the root folder */
	dirtree->source = g_idle_add(_dirtree_on_idle, plugin);
	return widget;
}


/* dirtree_destroy */
static void _dirtree_destroy(BrowserPlugin * plugin)
{
	Dirtree * dirtree = plugin->priv;

	if(dirtree->source != 0)
		g_source_remove(dirtree->source);
	g_object_unref(dirtree->folder);
	object_delete(dirtree);
}


/* dirtree_refresh_folder */
static void _dirtree_refresh_folder(BrowserPlugin * plugin,
		GtkTreeIter * parent, char const * path, gboolean recurse)
{
	Dirtree * dirtree = plugin->priv;
	DIR * dir;
	struct dirent * de;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->store);
	GtkTreeIter iter;
	gboolean valid;
	String * q;
	gchar * r;
	gboolean b;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(parent, \"%s\", %s)\n", __func__, path,
			recurse ? "TRUE" : "FALSE");
#endif
	/* consider all the current nodes obsolete */
	for(valid = gtk_tree_model_iter_children(model, &iter, parent);
			valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
		gtk_tree_store_set(dirtree->store, &iter, 3, FALSE, -1);
	if((dir = opendir(path)) == NULL)
		return;
	if(strcmp(path, "/") == 0) /* XXX hack */
		path = "";
	while((de = readdir(dir)) != NULL)
	{
		/* skip hidden folders except if we traverse it */
		if(de->d_name[0] == '.')
			continue;
		if(de->d_type != DT_DIR) /* XXX d_type is not portable */
			continue;
		q = string_new_append(path, "/", de->d_name, NULL);
		r = (q != NULL) ? g_filename_display_basename(q) : NULL;
		gtk_tree_store_insert(dirtree->store, &iter, parent, -1);
		gtk_tree_store_set(dirtree->store, &iter, 0, dirtree->folder,
				1, (r != NULL) ? r : de->d_name, 2, q, 3, TRUE,
				-1);
		if(recurse)
			_dirtree_refresh_folder(plugin, &iter, q, FALSE);
		g_free(r);
		string_delete(q);
	}
	closedir(dir);
	/* move all of the obsolete nodes */
	for(valid = gtk_tree_model_iter_children(model, &iter, parent);
			valid == TRUE;)
	{
		gtk_tree_model_get(model, &iter, 3, &b, -1);
		valid = b ? gtk_tree_model_iter_next(model, &iter)
			: gtk_tree_store_remove(dirtree->store, &iter);
	}
}


/* callbacks */
/* dirtree_on_idle */
static gboolean _dirtree_on_idle(gpointer data)
{
	BrowserPlugin * plugin = data;
	Dirtree * dirtree = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->store);
	GtkTreeIter iter;

	dirtree->source = 0;
	gtk_tree_model_iter_children(model, &iter, NULL);
	_dirtree_refresh_folder(plugin, &iter, "/", TRUE);
	return FALSE;
}


/* dirtree_on_row_activated */
static void _dirtree_on_row_activated(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	BrowserPlugin * plugin = data;
	Dirtree * dirtree = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->sorted);
	GtkTreeIter iter;
	gchar * location;

	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, 2, &location, -1);
	plugin->helper->set_location(plugin->helper->browser, location);
	g_free(location);
}


/* dirtree_on_row_expanded */
static void _dirtree_on_row_expanded(GtkTreeView * view, GtkTreeIter * iter,
		GtkTreePath * path, gpointer data)
{
	BrowserPlugin * plugin = data;
	Dirtree * dirtree = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->store);
	GtkTreeIter child;
	gchar * p;

	gtk_tree_model_sort_convert_iter_to_child_iter(GTK_TREE_MODEL_SORT(
				dirtree->sorted), &child, iter);
	gtk_tree_model_get(model, &child, 2, &p, -1);
	_dirtree_refresh_folder(plugin, &child, p, TRUE);
	g_free(p);
}
