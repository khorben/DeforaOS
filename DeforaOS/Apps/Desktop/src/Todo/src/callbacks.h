/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef TODO_CALLBACKS_H
# define TODO_CALLBACKS_H

# include <gtk/gtk.h>


/* callbacks */
gboolean on_closex(gpointer data);

/* menus */
/* file menu */
void on_file_new(gpointer data);
void on_file_edit(gpointer data);
void on_file_close(gpointer data);

/* edit menu */
void on_edit_select_all(gpointer data);
void on_edit_delete(gpointer data);
void on_edit_preferences(gpointer data);

/* view menu */
void on_view_all_tasks(gpointer data);
void on_view_completed_tasks(gpointer data);
void on_view_remaining_tasks(gpointer data);

/* help menu */
void on_help_about(gpointer data);

/* toolbar */
void on_new(gpointer data);
void on_edit(gpointer data);
void on_select_all(gpointer data);
void on_delete(gpointer data);
void on_preferences(gpointer data);
void on_view_as(gpointer data);

/* view */
void on_task_activated(gpointer data);
void on_task_cursor_changed(gpointer data);
void on_task_done_toggled(GtkCellRendererToggle * renderer, gchar * path,
		gpointer data);
void on_task_priority_edited(GtkCellRendererText * renderer, gchar * path,
		gchar * priority, gpointer data);
void on_task_title_edited(GtkCellRendererText * renderer, gchar * path,
		gchar * title, gpointer data);

#endif /* !TODO_CALLBACKS_H */
