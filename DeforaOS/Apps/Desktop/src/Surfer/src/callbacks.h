/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef SURFER_CALLBACKS_H
# define SURFER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(gpointer data);

#ifndef EMBEDDED
/* file menu */
void on_file_close(gpointer data);
void on_file_close_tab(gpointer data);
void on_file_new_window(gpointer data);
void on_file_new_tab(gpointer data);
void on_file_open(gpointer data);
void on_file_open_url(gpointer data);
void on_file_print(gpointer data);
void on_file_save_as(gpointer data);

/* edit menu */
void on_edit_find(gpointer data);
void on_edit_preferences(gpointer data);
void on_edit_select_all(gpointer data);
void on_edit_unselect_all(gpointer data);

/* view menu */
void on_view_zoom_in(gpointer data);
void on_view_zoom_out(gpointer data);
void on_view_normal_size(gpointer data);
void on_view_refresh(gpointer data);
void on_view_force_refresh(gpointer data);
void on_view_stop(gpointer data);
void on_view_page_source(gpointer data);

/* help menu */
void on_help_about(gpointer data);
#endif /* !EMBEDDED */

/* toolbar */
void on_back(gpointer data);
void on_close(gpointer data);
void on_forward(gpointer data);
void on_fullscreen(gpointer data);
void on_home(gpointer data);
void on_path_activate(gpointer data);
void on_preferences(gpointer data);
void on_refresh(gpointer data);
void on_stop(gpointer data);

/* notebook */
void on_notebook_close_tab(gpointer data);
void on_notebook_switch_page(GtkWidget * widget, GtkNotebookPage * page,
		guint n, gpointer data);

#endif
