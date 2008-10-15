/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>


/* types */
typedef struct _Prefs
{
	int flags;
	char * theme;
} Prefs;
#define PREFS_l 1

enum { COL_NAME = 0, COL_PIXBUF, COL_COUNT };


/* iconlist */
static int _iconlist_list(char * theme);
static int _iconlist_do(void);

static int _iconlist(Prefs * prefs)
{
	if(prefs->flags & PREFS_l)
		return _iconlist_list(prefs->theme);
	return _iconlist_do();
}

static int _iconlist_list(char * theme)
{
	GtkIconTheme * icontheme;
	GList * list;
	GList * p;
	gint * sizes;
	gint * q;

	if(theme == NULL)
		icontheme = gtk_icon_theme_get_default();
	else
	{
		icontheme = gtk_icon_theme_new();
		gtk_icon_theme_set_custom_theme(icontheme, theme);
	}
	if((list = gtk_icon_theme_list_icons(icontheme, NULL)) == NULL)
		return 1;
	for(p = list; p != NULL; p = p->next)
	{
		printf("%s:", (char*)p->data);
		if((sizes = gtk_icon_theme_get_icon_sizes(icontheme, p->data))
				== NULL)
		{
			puts(" unknown\n");
			continue;
		}
		for(q = sizes; *q != 0; q++)
			printf(" %d", *q);
		putc('\n', stdout);
		g_free(sizes);
	}
	g_list_foreach(list, (GFunc)g_free, NULL);
	g_free(list);
	return 0;
}

/* iconlist_do */
static void _do_iconview(GtkWidget * iconview, char const * theme);
/* callbacks */
static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
static void _on_theme_activate(GtkWidget * widget, gpointer data);

static int _iconlist_do(void)
{
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * label;
	GtkWidget * entry;
	GtkWidget * scrolled;
	GtkListStore * store;
	GtkWidget * iconview;

	/* window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Icon list");
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	g_signal_connect(window, "delete_event", G_CALLBACK(_on_closex), NULL);
	/* vbox */
	vbox = gtk_vbox_new(FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_item_new();
	label = gtk_label_new("Theme: ");
	gtk_container_add(GTK_CONTAINER(toolitem), label);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_item_new();
	entry = gtk_entry_new();
	gtk_container_add(GTK_CONTAINER(toolitem), entry);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	/* scrolled window */
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	/* icon view */
	store = gtk_list_store_new(COL_COUNT, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
	g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(
				_on_theme_activate), iconview); /* late */
	g_object_unref(store);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(iconview),
			GTK_SELECTION_MULTIPLE);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(iconview), 0);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(iconview), 1);
	_do_iconview(iconview, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled), iconview);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

static gboolean _on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
	return FALSE;
}

static void _on_theme_activate(GtkWidget * widget, gpointer data)
{
	GtkWidget * iconview = data;

	_do_iconview(iconview, gtk_entry_get_text(GTK_ENTRY(widget)));
}

static void _do_iconview(GtkWidget * iconview, char const * theme)
{
	GtkIconTheme * icontheme;
	GList * list;
	GList * p;
	GtkListStore * store;
	GtkTreeIter iter;
	GdkPixbuf * pixbuf;

	if(theme == NULL)
		icontheme = gtk_icon_theme_get_default();
	else
	{
		icontheme = gtk_icon_theme_new();
		gtk_icon_theme_set_custom_theme(icontheme, theme);
	}
	if((list = gtk_icon_theme_list_icons(icontheme, NULL)) == NULL)
		return;
	store = GTK_LIST_STORE(gtk_icon_view_get_model(GTK_ICON_VIEW(
					iconview)));
	gtk_list_store_clear(store);
	for(p = list; p != NULL; p = p->next)
	{
		pixbuf = gtk_icon_theme_load_icon(icontheme, p->data, 48, 0,
				NULL);
		gtk_list_store_insert_with_values(store, &iter, -1,
				COL_NAME, p->data, COL_PIXBUF, pixbuf, -1);
		g_free(p->data);
	}
	g_free(list);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: iconlist\n"
"       iconlist -l [-t theme]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	gtk_init(&argc, &argv);
	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "lt:")) != -1)
		switch(o)
		{
			case 'l':
				prefs.flags |= PREFS_l;
				break;
			case 't':
				prefs.theme = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _iconlist(&prefs) == 0 ? 0 : 2;
}
