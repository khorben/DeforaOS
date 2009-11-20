/* $Id$ */
static char _copyright[] =
"Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Todo */
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
"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n";



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <System.h>
#include <Desktop.h>
#include "callbacks.h"
#include "todo.h"
#include "../config.h"


/* Todo */
/* private */
/* types */
struct _Todo
{
	GtkWidget * window;
	GtkWidget * scrolled;
	GtkListStore * store;
	GtkWidget * view;
	GtkWidget * statusbar;
	GtkWidget * about;
};


/* constants */
enum { TD_COL_DONE, TD_COL_TITLE, TD_COL_START, TD_COL_DISPLAY_START, TD_COL_END,
	TD_COL_DISPLAY_END, TD_COL_PRIORITY, TD_COL_CATEGORY, TD_COL_COMMENT };
#define TD_COL_LAST TD_COL_COMMENT
#define TD_NUM_COLS (TD_COL_LAST + 1)

static struct
{
	int col;
	char const * title;
	int sort;
	GCallback callback;
} _todo_columns[] = {
	{ TD_COL_DONE, "Done", TD_COL_DONE, G_CALLBACK(on_task_done_toggled) },
	{ TD_COL_TITLE, "Title", TD_COL_TITLE, G_CALLBACK(on_task_title_edited) },
	{ TD_COL_DISPLAY_START, "Beginning", TD_COL_START, NULL },
	{ TD_COL_DISPLAY_END, "Completion", TD_COL_END, NULL },
	{ 0, NULL, 0, NULL }
};


/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

#ifndef EMBEDDED
/* menubar */
static DesktopMenu _file_menu[] =
{
	{ "_New", G_CALLBACK(on_file_new), GTK_STOCK_NEW, GDK_N },
	{ "_Edit", G_CALLBACK(on_file_edit), GTK_STOCK_EDIT, GDK_E },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};
static DesktopMenu _edit_menu[] =
{
#if GTK_CHECK_VERSION(2, 10, 0)
	{ "_Select all", G_CALLBACK(on_edit_select_all), GTK_STOCK_SELECT_ALL,
#else
	{ "_Select all", G_CALLBACK(on_edit_select_all), "edit-select-all",
#endif
		GDK_A },
	{ "", NULL, NULL, 0 },
	{ "_Delete", G_CALLBACK(on_edit_delete), GTK_STOCK_DELETE, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Preferences", G_CALLBACK(on_edit_preferences), GTK_STOCK_PREFERENCES,
		GDK_P },
	{ NULL, NULL, NULL, 0 }
};
static DesktopMenu _help_menu[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};
static DesktopMenubar _menubar[] =
{
	{ "_File", _file_menu },
	{ "_Edit", _edit_menu },
	{ "_Help", _help_menu },
	{ NULL, NULL },
};
#endif

/* toolbar */
static DesktopToolbar _toolbar[] =
{
	{ GTK_STOCK_NEW, G_CALLBACK(on_new), 0, NULL },
	{ GTK_STOCK_EDIT, G_CALLBACK(on_edit), 0, NULL },
	{ "", NULL, 0, NULL },
#if GTK_CHECK_VERSION(2, 10, 0)
	{ GTK_STOCK_SELECT_ALL, G_CALLBACK(on_select_all), 0, NULL },
#else
	{ "edit-select-all", G_CALLBACK(on_select_all), 0, NULL },
#endif
	{ GTK_STOCK_DELETE, G_CALLBACK(on_delete), 0, NULL },
#ifdef EMBEDDED
	{ "", NULL, 0, NULL },
	{ GTK_STOCK_PREFERENCES, G_CALLBACK(on_preferences), 0, NULL },
#endif
	{ NULL, NULL, 0, NULL }
};


/* public */
/* functions */
/* todo_new */
static void _new_view(Todo * todo);

Todo * todo_new(void)
{
	Todo * todo;
	GtkAccelGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((todo = malloc(sizeof(*todo))) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	/* main window */
	group = gtk_accel_group_new();
	todo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(todo->window), group);
	gtk_window_set_default_size(GTK_WINDOW(todo->window), 300, 400);
	gtk_window_set_icon_name(GTK_WINDOW(todo->window), "stock_todo");
	gtk_window_set_title(GTK_WINDOW(todo->window), "Todo");
	g_signal_connect_swapped(G_OBJECT(todo->window), "delete-event",
			G_CALLBACK(on_closex), todo);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef EMBEDDED
	/* menubar */
	widget = desktop_menubar_create(_menubar, todo, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
#endif
	/* toolbar */
	widget = desktop_toolbar_create(_toolbar, todo, group);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* view */
	todo->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(todo->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	_new_view(todo);
	gtk_box_pack_start(GTK_BOX(vbox), todo->scrolled, TRUE, TRUE, 0);
	/* statusbar */
	todo->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), todo->statusbar, FALSE, TRUE, 0);
	todo->about = NULL;
	gtk_container_add(GTK_CONTAINER(todo->window), vbox);
	gtk_widget_show_all(todo->window);
	return todo;
}

static void _new_view(Todo * todo)
{
	GtkTreeSelection * sel;
	size_t i;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	todo->store = gtk_list_store_new(TD_NUM_COLS, G_TYPE_BOOLEAN, G_TYPE_STRING,
			G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING,
			G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING);
	todo->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(todo->store));
	if((sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view))) != NULL)
		gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
	/* done column */
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(G_OBJECT(renderer), "toggled", G_CALLBACK(
				_todo_columns[0].callback), todo);
	column = gtk_tree_view_column_new_with_attributes(_todo_columns[0].title,
			renderer, "active", _todo_columns[0].col, NULL);
	gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
			GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 50);
	gtk_tree_view_column_set_sort_column_id(column, TD_COL_DONE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(todo->view), column);
	/* other columns */
	for(i = 1; _todo_columns[i].title != NULL; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		if(_todo_columns[i].callback != NULL)
		{
			g_object_set(G_OBJECT(renderer), "editable", TRUE, NULL);
			g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
						_todo_columns[i].callback), todo);
		}
		column = gtk_tree_view_column_new_with_attributes(
				_todo_columns[i].title, renderer, "text",
				_todo_columns[i].col, NULL);
		gtk_tree_view_column_set_sort_column_id(column,
				_todo_columns[i].sort);
		gtk_tree_view_append_column(GTK_TREE_VIEW(todo->view), column);
	}
	gtk_container_add(GTK_CONTAINER(todo->scrolled), todo->view);
}


/* todo_delete */
void todo_delete(Todo * todo)
{
	free(todo);
}


/* useful */
/* todo_about */
void todo_about(Todo * todo)
{
	if(todo->about != NULL)
	{
		gtk_widget_show(todo->about);
		return;
	}
	todo->about = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(todo->about),
			GTK_WINDOW(todo->window));
	desktop_about_dialog_set_authors(todo->about, _authors);
	desktop_about_dialog_set_copyright(todo->about, _copyright);
	desktop_about_dialog_set_logo_icon_name(todo->about, "stock_todo");
	desktop_about_dialog_set_license(todo->about, _license);
	desktop_about_dialog_set_name(todo->about, PACKAGE);
	desktop_about_dialog_set_version(todo->about, VERSION);
	gtk_widget_show(todo->about);
}


/* tasks */
/* todo_task_add */
void todo_task_add(Todo * todo)
{
	GtkTreeIter iter;

	gtk_list_store_insert(todo->store, &iter, 0);
}


/* todo_task_edit */
void todo_task_edit(Todo * todo)
{
	/* FIXME implement */
}


/* todo_task_remove */
void todo_task_remove(Todo * todo)
{
	/* FIXME implement */
}


/* todo_task_select_all */
void todo_task_select_all(Todo * todo)
{
	GtkTreeSelection * sel;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(todo->view));
	gtk_tree_selection_select_all(sel);
}


/* todo_task_set_title */
void todo_task_set_title(Todo * todo, GtkTreePath * path, char const * title)
{
	GtkTreeIter iter;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(todo->store), &iter, path);
	gtk_list_store_set(todo->store, &iter, TD_COL_TITLE, title, -1);
}


/* todo_task_toggle_done */
void todo_task_toggle_done(Todo * todo, GtkTreePath * path)
{
	GtkTreeIter iter;
	gboolean done;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(todo->store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(todo->store), &iter, TD_COL_DONE, &done,
			-1);
	done = !done;
	gtk_list_store_set(todo->store, &iter, TD_COL_DONE, done, -1);
}
