/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */
/* This file is part of Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef BROWSER_CALLBACKS_H
# define BROWSER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

/* file menu */
void on_file_new_window(GtkMenuItem * menuitem, gpointer data);
void on_file_close(GtkMenuItem * menuitem, gpointer data);

/* edit menu */
void on_edit_copy(GtkMenuItem * menuitem, gpointer data);
void on_edit_cut(GtkMenuItem * menuitem, gpointer data);
void on_edit_delete(GtkMenuItem * menuitem, gpointer data);
void on_edit_preferences(GtkMenuItem * menuitem, gpointer data);
void on_edit_select_all(GtkMenuItem * menuitem, gpointer data);
void on_edit_unselect_all(GtkMenuItem * menuitem, gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);

/* view menu */
void on_view_home(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_details(GtkWidget * widget, gpointer data);
void on_view_icons(GtkWidget * widget, gpointer data);
void on_view_list(GtkWidget * widget, gpointer data);
#endif

/* toolbar */
void on_back(GtkWidget * widget, gpointer data);
void on_forward(GtkWidget * widget, gpointer data);
void on_home(GtkWidget * widget, gpointer data);
void on_properties(GtkWidget * widget, gpointer data);
void on_refresh(GtkWidget * widget, gpointer data);
void on_updir(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_as(GtkWidget * widget, gpointer data);
#endif

/* address bar */
void on_path_activate(GtkWidget * widget, gpointer data);

/* view */
void on_detail_default(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_icon_default(GtkIconView * view, GtkTreePath * path, gpointer data);
#endif
void on_filename_edited(GtkCellRendererText * renderer, gchar * arg1,
		gchar * arg2, gpointer data);
gboolean on_view_popup(GtkWidget * widget, gpointer data);
gboolean on_view_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data);

#endif /* !BROWSER_CALLBACKS_H */
