/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
