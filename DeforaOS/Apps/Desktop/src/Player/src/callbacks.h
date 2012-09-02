/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
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



#ifndef PLAYER_CALLBACKS_H
# define PLAYER_CALLBACKS_H

# include <gtk/gtk.h>


/* player */
gboolean on_player_closex(gpointer data);
void on_player_removed(gpointer data);

/* file menu */
void on_file_open(gpointer data);
void on_file_properties(gpointer data);
void on_file_close(gpointer data);

/* edit menu */
void on_edit_preferences(gpointer data);

/* view menu */
void on_view_fullscreen(gpointer data);
void on_view_playlist(gpointer data);

/* help menu */
void on_help_about(gpointer data);

/* toolbar */
void on_close(gpointer data);
void on_open(gpointer data);
void on_properties(gpointer data);
void on_preferences(gpointer data);

/* playbar */
void on_previous(gpointer data);
void on_rewind(gpointer data);
void on_play(gpointer data);
void on_pause(gpointer data);
void on_stop(gpointer data);
void on_forward(gpointer data);
void on_next(gpointer data);
void on_volume_changed(gpointer data);
void on_fullscreen(gpointer data);

/* view */
/* playlist */
void on_playlist_activated(gpointer data);
void on_playlist_add(gpointer data);
gboolean on_playlist_closex(gpointer data);
void on_playlist_load(gpointer data);
void on_playlist_remove(gpointer data);
void on_playlist_save(gpointer data);

#endif /* !PLAYER_CALLBACKS_H */
