/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include "todo.h"
#include "callbacks.h"


/* callbacks */
/* on_closex */
gboolean on_closex(gpointer data)
{
	Todo * todo = data;

	gtk_main_quit();
	return FALSE;
}


/* file menu */
/* on_file_close */
void on_file_close(gpointer data)
{
	Todo * todo = data;

	on_closex(todo);
}


/* on_file_edit */
void on_file_edit(gpointer data)
{
	Todo * todo = data;

	todo_task_edit(todo);
}


/* on_file_new */
void on_file_new(gpointer data)
{
	Todo * todo = data;

	todo_task_add(todo, NULL);
}


/* edit menu */
/* on_edit_delete */
void on_edit_delete(gpointer data)
{
	Todo * todo = data;

	todo_task_delete_selected(todo);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	Todo * todo = data;

	/* FIXME implement */
}


/* on_edit_select_all */
void on_edit_select_all(gpointer data)
{
	Todo * todo = data;

	todo_task_select_all(todo);
}


/* help menu */
/* on_help_about */
void on_help_about(gpointer data)
{
	Todo * todo = data;

	todo_about(todo);
}


/* toolbar */
/* on_delete */
void on_delete(gpointer data)
{
	Todo * todo = data;

	todo_task_delete_selected(todo);
}


/* on_edit */
void on_edit(gpointer data)
{
	Todo * todo = data;

	todo_task_edit(todo);
}


/* on_new */
void on_new(gpointer data)
{
	Todo * todo = data;

	todo_task_add(todo, NULL);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	Todo * todo = data;

	/* FIXME implement */
}


/* on_select_all */
void on_select_all(gpointer data)
{
	Todo * todo = data;

	todo_task_select_all(todo);
}


/* view */
/* on_task_done_toggled */
void on_task_done_toggled(GtkCellRendererToggle * renderer, gchar * path,
		gpointer data)
{
	Todo * todo = data;
	GtkTreePath * treepath;

	treepath = gtk_tree_path_new_from_string(path);
	todo_task_toggle_done(todo, treepath);
	gtk_tree_path_free(treepath);
}


/* on_task_title_edited */
void on_task_title_edited(GtkCellRendererText * renderer, gchar * path,
		gchar * title, gpointer data)
{
	Todo * todo = data;
	GtkTreePath * treepath;

	treepath = gtk_tree_path_new_from_string(path);
	todo_task_set_title(todo, treepath, title);
	gtk_tree_path_free(treepath);
}
