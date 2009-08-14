/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

#ifndef EMBEDDED
/* file menu */
void on_file_close(GtkWidget * widget, gpointer data);
void on_file_new_window(GtkWidget * widget, gpointer data);
void on_file_open(GtkWidget * widget, gpointer data);
void on_file_open_url(GtkWidget * widget, gpointer data);

/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data);
void on_edit_select_all(GtkWidget * widget, gpointer data);
void on_edit_unselect_all(GtkWidget * widget, gpointer data);

/* view menu */
void on_view_zoom_in(GtkWidget * widget, gpointer data);
void on_view_zoom_out(GtkWidget * widget, gpointer data);
void on_view_normal_size(GtkWidget * widget, gpointer data);
void on_view_refresh(GtkWidget * widget, gpointer data);
void on_view_force_refresh(GtkWidget * widget, gpointer data);
void on_view_stop(GtkWidget * widget, gpointer data);
void on_view_page_source(GtkWidget * widget, gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);
#endif /* !EMBEDDED */

/* toolbar */
void on_back(GtkWidget * widget, gpointer data);
void on_forward(GtkWidget * widget, gpointer data);
void on_fullscreen(GtkToggleToolButton * button, gpointer data);
void on_home(GtkWidget * widget, gpointer data);
void on_path_activate(GtkWidget * widget, gpointer data);
void on_refresh(GtkWidget * widget, gpointer data);
void on_stop(GtkWidget * widget, gpointer data);

#endif
