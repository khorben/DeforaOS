/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
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



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <System.h>
#include <Desktop.h>
#include "callbacks.h"
#include "todo.h"


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
};


/* constants */
enum { TD_COL_DONE, TD_COL_TITLE };
#define TD_COL_LAST TD_COL_TITLE
#define TD_NUM_COLS (TD_COL_LAST + 1)


/* variables */
static DesktopMenu _file_menu[] =
{
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};
static DesktopMenu _edit_menu[] =
{
	{ "_Delete", G_CALLBACK(on_edit_delete), GTK_STOCK_DELETE, 0 },
	{ "", NULL, NULL, 0 },
#if GTK_CHECK_VERSION(2, 10, 0)
	{ "_Select all", G_CALLBACK(on_edit_select_all), GTK_STOCK_SELECT_ALL,
#else
	{ "_Select all", G_CALLBACK(on_edit_select_all), "edit-select-all",
#endif
		GDK_A },
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


/* public */
/* functions */
/* todo_new */
static void _new_view(Todo * todo);

Todo * todo_new(void)
{
	Todo * todo;
	GtkWidget * vbox;
	GtkWidget * widget;

	if((todo = malloc(sizeof(*todo))) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	/* main window */
	todo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(todo->window), 300, 400);
	gtk_window_set_icon_name(GTK_WINDOW(todo->window), "stock_todo");
	gtk_window_set_title(GTK_WINDOW(todo->window), "Todo");
	g_signal_connect_swapped(G_OBJECT(todo->window), "delete-event",
			G_CALLBACK(on_closex), todo);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_menubar, todo, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* toolbar */
	/* view */
	todo->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(todo->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	_new_view(todo);
	gtk_box_pack_start(GTK_BOX(vbox), todo->scrolled, TRUE, TRUE, 0);
	/* statusbar */
	todo->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), todo->statusbar, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(todo->window), vbox);
	gtk_widget_show_all(todo->window);
	return todo;
}

static void _new_view(Todo * todo)
{
	todo->store = gtk_list_store_new(TD_NUM_COLS, G_TYPE_BOOLEAN,
			G_TYPE_STRING);
	todo->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				todo->store));
	gtk_container_add(GTK_CONTAINER(todo->scrolled), todo->view);
}


/* todo_delete */
void todo_delete(Todo * todo)
{
	free(todo);
}


/* useful */
/* todo_delete_selection */
void todo_delete_selection(Todo * todo)
{
	/* FIXME implement */
}


/* todo_select_all */
void todo_select_all(Todo * todo)
{
	/* FIXME implement */
}
