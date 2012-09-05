/* $Id$ */
/* Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PLAYER_PLAYER_H
# define PLAYER_PLAYER_H

# include <gtk/gtk.h>


/* Player */
/* types */
typedef struct _Player Player;

enum
{
	PL_COL_ENABLED = 0,
	PL_COL_ICON,
	PL_COL_FILENAME,
	PL_COL_TRACK,
	PL_COL_ARTIST,
	PL_COL_ALBUM,
	PL_COL_TITLE,
	PL_NUM_COLS
};
# define PL_LAST PL_NUM_COLS


/* functions */
Player * player_new(void);
void player_delete(Player * player);

/* accessors */
gboolean player_get_fullscreen(Player * player);
void player_set_fullscreen(Player * player, gboolean fullscreen);
void player_set_size(Player * player, int width, int height);
void player_set_volume(Player * player, gdouble volume);

/* useful */
void player_about(Player * player);
int player_error(Player * player, char const * message, int ret);
int player_sigchld(Player * player);

/* playlist management */
int player_open(Player * player, char const * filename);
int player_open_dialog(Player * player);
int player_open_url(Player * player, char const * url);
int player_open_url_dialog(Player * player);
void player_playlist_add(Player * player, char const * filename);
void player_playlist_add_dialog(Player * player);
void player_playlist_add_url(Player * player, char const * url);
void player_playlist_clear(Player * player);
void player_playlist_open(Player * player, char const * filename);
void player_playlist_open_dialog(Player * player);
void player_playlist_play_selected(Player * player);
void player_playlist_save_as_dialog(Player * player);

/* playback */
void player_next(Player * player);
int player_play(Player * player);
void player_pause(Player * player);
void player_previous(Player * player);
void player_stop(Player * player);
void player_rewind(Player * player);
void player_forward(Player * player);

/* user interface */
void player_show_playlist(Player * player, gboolean show);
void player_show_preferences(Player * player, gboolean show);
void player_show_properties(Player * player, gboolean show);

#endif /* !PLAYER_PLAYER_H */
