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
	GdkPixbuf * folder;
	GtkTreeStore * store;
	GtkTreeModel * sorted;
	GtkWidget * view;
} Dirtree;


/* prototypes */
static GtkWidget * _dirtree_init(BrowserPlugin * plugin);
static void _dirtree_destroy(BrowserPlugin * plugin);
static void _dirtree_refresh(BrowserPlugin * plugin, char const * path);

/* callbacks */
static void _dirtree_on_row_activated(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);


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
	dirtree->sorted = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(
				dirtree->store));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(dirtree->sorted),
			1, GTK_SORT_ASCENDING);
	dirtree->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				dirtree->sorted));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dirtree->view), FALSE);
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
	treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dirtree->view));
	gtk_tree_selection_set_mode(treesel, GTK_SELECTION_SINGLE);
	g_signal_connect(dirtree->view, "row-activated", G_CALLBACK(
				_dirtree_on_row_activated), plugin);
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
static gboolean _refresh_child(Dirtree * dirtree, GtkTreeIter * parent,
		char const * path, char const * basename);

static void _dirtree_refresh(BrowserPlugin * plugin, char const * path)
{
	Dirtree * dirtree = plugin->priv;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->store);
	GtkTreeIter iter;
	GtkTreeIter siter;
	gboolean valid;
	GtkTreeSelection * treesel;
	size_t i;
	size_t j;
	char * p;
	char c;

	if(path == NULL || (p = strdup(path)) == NULL)
		return;
	valid = gtk_tree_model_iter_children(model, &iter, NULL);
	for(i = 0; valid == TRUE && p[i] != '\0'; i++)
	{
		if(p[i] != '/')
			continue;
		p[i] = '\0';
		for(j = i + 1; p[j] != '\0' && p[j] != '/'; j++);
		c = p[j];
		p[j] = '\0';
		valid = _refresh_child(dirtree, &iter, (i == 0) ? "/" : p,
				&p[i + 1]);
		p[i] = '/';
		p[j] = c;
	}
	free(p);
	if(valid == TRUE)
	{
		treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
					dirtree->view));
		gtk_tree_model_sort_convert_child_iter_to_iter(
				GTK_TREE_MODEL_SORT(dirtree->sorted), &siter,
				&iter);
		gtk_tree_selection_select_iter(treesel, &siter);
		_refresh_child(dirtree, &iter, path, NULL);
	}
}

static gboolean _refresh_child(Dirtree * dirtree, GtkTreeIter * parent,
		char const * path, char const * basename)
{
	gboolean ret = FALSE;
	DIR * dir;
	struct dirent * de;
	GtkTreeModel * model = GTK_TREE_MODEL(dirtree->store);
	GtkTreeIter iter;
	gboolean valid;
	GtkTreePath * p = NULL;
	GtkTreePath * s = NULL;
	String * q;
	gchar * r;

	if(basename == NULL)
		basename = "";
	for(valid = gtk_tree_model_iter_children(model, &iter, parent);
			valid == TRUE;
			valid = gtk_tree_store_remove(dirtree->store, &iter));
	if((dir = opendir(path)) == NULL)
		return FALSE;
	while((de = readdir(dir)) != NULL)
	{
		/* skip hidden folders except if we traverse it */
		if(de->d_name[0] == '.' && strcmp(de->d_name, basename) != 0)
			continue;
		if(de->d_type != DT_DIR) /* XXX d_type is not portable */
			continue;
		q = string_new_append(path, "/", de->d_name, NULL);
		r = (q != NULL) ? g_filename_display_basename(q) : NULL;
		gtk_tree_store_insert(dirtree->store, &iter, parent, -1);
		gtk_tree_store_set(dirtree->store, &iter, 0, dirtree->folder,
				1, (r != NULL) ? r : de->d_name, 2, q, -1);
		/* remember the folder we mean to traverse */
		if(p == NULL && strcmp(de->d_name, basename) == 0)
			p = gtk_tree_model_get_path(model, &iter);
		g_free(r);
		string_delete(q);
	}
	closedir(dir);
	if(p == NULL)
		return FALSE;
	s = gtk_tree_model_sort_convert_child_path_to_path(GTK_TREE_MODEL_SORT(
				dirtree->sorted), p);
	gtk_tree_view_expand_to_path(GTK_TREE_VIEW(dirtree->view), s);
	gtk_tree_path_free(s);
	ret = gtk_tree_model_get_iter(model, parent, p);
	gtk_tree_path_free(p);
	return ret;
}


/* callbacks */
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
