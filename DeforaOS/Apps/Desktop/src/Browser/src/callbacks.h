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



#ifndef BROWSER_CALLBACKS_H
# define BROWSER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

/* file menu */
void on_file_new_window(GtkMenuItem * menuitem, gpointer data);
void on_file_new_folder(GtkMenuItem * menuitem, gpointer data);
void on_file_close(GtkMenuItem * menuitem, gpointer data);

/* edit menu */
void on_edit_copy(GtkMenuItem * menuitem, gpointer data);
void on_edit_cut(GtkMenuItem * menuitem, gpointer data);
void on_edit_delete(GtkMenuItem * menuitem, gpointer data);
void on_edit_paste(GtkMenuItem * menuitem, gpointer data);
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
void on_view_thumbnails(GtkWidget * widget, gpointer data);
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
