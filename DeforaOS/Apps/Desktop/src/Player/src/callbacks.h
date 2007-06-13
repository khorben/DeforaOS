/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
/* Player is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Player is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Player; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef PLAYER_CALLBACKS_H
# define PLAYER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_player_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
gboolean on_player_configure(GtkWidget * window, GdkEventConfigure * event,
		gpointer data);

/* file menu */
void on_file_open(GtkWidget * widget, gpointer data);
void on_file_close(GtkWidget * widget, gpointer data);

/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data);

/* view menu */
void on_view_fullscreen(GtkWidget * widget, gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);

/* toolbar */
void on_previous(GtkWidget * widget, gpointer data);
void on_rewind(GtkWidget * widget, gpointer data);
void on_play(GtkWidget * widget, gpointer data);
void on_pause(GtkWidget * widget, gpointer data);
void on_stop(GtkWidget * widget, gpointer data);
void on_forward(GtkWidget * widget, gpointer data);
void on_next(GtkWidget * widget, gpointer data);

/* view */

#endif /* !PLAYER_CALLBACKS_H */
