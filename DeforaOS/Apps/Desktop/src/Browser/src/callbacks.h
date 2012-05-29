/* $Id$ */
/* Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef BROWSER_CALLBACKS_H
# define BROWSER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(gpointer data);

/* accelerators */
gboolean on_close(gpointer data);
gboolean on_location(gpointer data);
gboolean on_new_window(gpointer data);
gboolean on_open_file(gpointer data);

/* file menu */
void on_file_new_window(gpointer data);
void on_file_new_folder(gpointer data);
void on_file_new_symlink(gpointer data);
void on_file_close(gpointer data);
void on_file_open_file(gpointer data);

/* edit menu */
void on_edit_copy(gpointer data);
void on_edit_cut(gpointer data);
void on_edit_delete(gpointer data);
void on_edit_paste(gpointer data);
void on_edit_preferences(gpointer data);
void on_edit_select_all(gpointer data);
void on_edit_unselect_all(gpointer data);

/* help menu */
void on_help_about(gpointer data);
void on_help_contents(gpointer data);

/* view menu */
void on_view_home(gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_details(gpointer data);
void on_view_icons(gpointer data);
void on_view_list(gpointer data);
void on_view_thumbnails(gpointer data);
#endif

/* toolbar */
void on_back(gpointer data);
void on_copy(gpointer data);
void on_cut(gpointer data);
void on_forward(gpointer data);
void on_home(gpointer data);
void on_paste(gpointer data);
void on_properties(gpointer data);
void on_refresh(gpointer data);
void on_updir(gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_as(gpointer data);
#endif

/* address bar */
void on_path_activate(gpointer data);

/* view */
void on_detail_default(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_icon_default(GtkIconView * view, GtkTreePath * path, gpointer data);
#endif
void on_filename_edited(GtkCellRendererText * renderer, gchar * arg1,
		gchar * arg2, gpointer data);
#if GTK_CHECK_VERSION(2, 8, 0)
void on_view_drag_data_get(GtkWidget * widget, GdkDragContext * dc,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data);
void on_view_drag_data_received(GtkWidget * widget, GdkDragContext * context,
		gint x, gint y, GtkSelectionData * seldata, guint info,
		guint time, gpointer data);
#endif
gboolean on_view_popup(GtkWidget * widget, gpointer data);
gboolean on_view_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data);

#endif /* !BROWSER_CALLBACKS_H */
