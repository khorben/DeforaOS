/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <gtk/gtk.h>


/* dirtree */
/* variables */
static GdkPixbuf * _folder = NULL;


/* functions */
/* dirtree_new */
static int _dirtree_add(GtkTreeStore * store, GtkTreeIter * iter);
/* callbacks */
static gboolean _on_dirtree_delete(GtkWidget * widget, GdkEvent * event,
		gpointer data);

static int _dirtree_new(char * pathname)
{
	GtkIconTheme * theme;
	GtkWidget * scrolled;
	GtkWidget * window;
	GtkTreeStore * store;
	GtkTreeIter iter;
	GtkWidget * treeview;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(_folder == NULL)
	{
		theme = gtk_icon_theme_get_default();
		_folder = gtk_icon_theme_load_icon(theme, "stock_folder", 16, 0,
				NULL);
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Directory tree");
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_on_dirtree_delete), NULL);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	store = gtk_tree_store_new(3, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), 2,
			GTK_SORT_ASCENDING);
	gtk_tree_store_append(store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, _folder, 1, pathname, 2, basename(
				pathname), -1);
	_dirtree_add(store, &iter);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("Icon", renderer,
			"pixbuf", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer,
			"text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_container_add(GTK_CONTAINER(scrolled), treeview);
	gtk_container_add(GTK_CONTAINER(window), scrolled);
	gtk_widget_show_all(window);
	return 0;
}

static int _dirtree_add(GtkTreeStore * store, GtkTreeIter * iter)
{
	GValue pathname;
	const char * p;
	size_t len;
	char * str;
	DIR * dir;
	struct dirent * de;
	char * q;
	GtkTreeIter iter2;

	memset(&pathname, 0, sizeof(pathname));
	gtk_tree_model_get_value(GTK_TREE_MODEL(store), iter, 1, &pathname);
	p = g_value_get_string(&pathname);
	len = strlen(p) + 1;
	if((dir = opendir(p)) == NULL
			|| (str = malloc(len)) == NULL)
	{
		perror(p);
		g_value_unset(&pathname);
		return 1;
	}
	snprintf(str, len + 1, "%s/", p);
	while((de = readdir(dir)) != NULL)
	{
		if(!(de->d_type & DT_DIR)
				|| strcmp(".", de->d_name) == 0
				|| strcmp("..", de->d_name) == 0)
			continue;
		if((q = realloc(str, len + de->d_namlen + 1)) == NULL)
			continue;
		str = q;
		snprintf(&str[len], de->d_namlen + 1, "%s", de->d_name);
		gtk_tree_store_append(store, &iter2, iter);
		gtk_tree_store_set(store, &iter2, 0, _folder, 1, str,
				2, de->d_name, -1);
		_dirtree_add(store, &iter2);
	}
	closedir(dir);
	free(str);
	g_value_unset(&pathname);
	return 0;
}

static gboolean _on_dirtree_delete(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_main_quit();
	return FALSE;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: dirtree [pathname]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * root = "/";

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 == argc)
		root = argv[optind];
	else if(optind != argc)
		return _usage();
	_dirtree_new(root);
	gtk_main();
	return 0;
}
