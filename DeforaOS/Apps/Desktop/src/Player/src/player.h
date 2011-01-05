/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _Player
{
	/* view */
	int paused;
	gboolean fullscreen;

	/* current file */
	GtkTreeRowReference * current;
	int width;
	int height;
	int audio_bitrate;
	int audio_channels;
	char * audio_codec;
	int audio_rate;
	gdouble length;
	gdouble video_aspect;
	int video_bitrate;
	char * video_codec;
	gdouble video_fps;
	int video_rate;

	/* mplayer */
	pid_t pid;
	int fd[2][2];				/* mplayer pipes	*/
	GIOChannel * channel[2];
	char * buf;				/* pipe write buffer	*/
	size_t buf_len;

	/* callbacks */
	guint read_id;				/* pipe read source id	*/
	guint timeout_id;			/* timeout source id	*/

	/* widgets */
	GtkWidget * window;
#ifndef EMBEDDED
	GtkWidget * menubar;
#endif
	GtkWidget * view;
	GtkWidget * view_window;
	GtkToolItem * tb_previous;
	GtkToolItem * tb_rewind;
	GtkToolItem * tb_play;
	GtkToolItem * tb_pause;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_next;
	GtkWidget * progress;
	GtkToolItem * tb_fullscreen;
#ifndef EMBEDDED
	GtkWidget * statusbar;
	gint statusbar_id;
#endif

	/* about */
	GtkWidget * ab_window;

	/* preferences */
	GtkWidget * pr_window;

	/* playlist */
	GtkWidget * pl_window;
	GtkListStore * pl_store;
	GtkWidget * pl_view;
} Player;

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

/* useful */
void player_about(Player * player);
int player_error(Player * player, char const * message, int ret);
int player_sigchld(Player * player);

/* playlist management */
int player_open(Player * player, char const * filename);
int player_open_dialog(Player * player);
void player_playlist_add(Player * player, char const * filename);
void player_playlist_add_dialog(Player * player);
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
void player_view_playlist(Player * player, gboolean view);
void player_view_preferences(Player * player);
void player_view_properties(Player * player);

#endif /* !PLAYER_PLAYER_H */
