/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef PLAYER_PLAYER_H
# define PLAYER_PLAYER_H

# include <gtk/gtk.h>
# include <gdk/gdkx.h>


/* Player */
typedef struct _Player
{
	char * filename;
	int atstart;

	/* mplayer */
	pid_t pid;
	int fd[2];

	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	Window view_window;
	guint view_yoffset;
	guint view_iheight;
	GtkToolItem * tb_previous;
	GtkToolItem * tb_rewind;
	GtkToolItem * tb_play;
	GtkToolItem * tb_pause;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_next;
	GtkWidget * statusbar;
	gint statusbar_id;
} Player;

Player * player_new(void);
void player_delete(Player * player);

/* useful */
int player_error(Player * player, char const * message, int ret);
int player_sigchld(Player * player);

/* playlist management */
void player_next(Player * player);
void player_previous(Player * player);
void player_open(Player * player, char const * filename);
void player_open_dialog(Player * player);
void player_queue_add(Player * player, char const * filename);
void player_queue_remove(Player * player, char const * filename);
void player_queue_save(Player * player, char const * filename);
void player_queue_save_dialog(Player * player);

void player_play(Player * player);
void player_pause(Player * player);
void player_stop(Player * player);
void player_rewind(Player * player);
void player_forward(Player * player);

#endif
