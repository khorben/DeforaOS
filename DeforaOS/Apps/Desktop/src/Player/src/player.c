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



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "callbacks.h"
#include "player.h"


/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};


/* constants */
struct _menu _menu_file[] =
{
	{ "_Open", G_CALLBACK(on_file_open), GTK_STOCK_OPEN },
	{ "", NULL, NULL },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE },
	{ NULL, NULL, NULL }
};

struct _menu _menu_edit[] =
{
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES },
	{ NULL, NULL, NULL }
};

struct _menu _menu_view[] =
{
#if GTK_CHECK_VERSION(2, 8, 0)
	{ "_Fullscreen", G_CALLBACK(on_view_fullscreen), GTK_STOCK_FULLSCREEN },
#else
	{ "_Fullscreen", G_CALLBACK(on_view_fullscreen), NULL },
#endif
	{ NULL, NULL, NULL }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL },
#endif
	{ NULL, NULL, NULL }
};

static struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* Player */
static int _player_error(char const * message, int ret);
static GtkWidget * _new_menubar(Player * player);
static void _new_mplayer(Player * player);
Player * player_new(void)
{
	Player * player;
	GtkWidget * vbox;
	GtkWidget * tb_menubar;
	GtkWidget * toolbar;
	GtkRequisition req;
	unsigned long black;

	if((player = malloc(sizeof(*player))) == NULL)
		return NULL;
	player->filename = NULL;
	player->atstart = 0;
	player->pid = -1;
	player->window = NULL;
	if(pipe(player->fd) != 0)
	{
		player_error(player, strerror(errno), 0);
		free(player);
		return NULL;
	}
	player->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(player->window), 300, 300);
	gtk_window_set_title(GTK_WINDOW(player->window), "Player");
	gtk_widget_realize(player->window);
	g_signal_connect(G_OBJECT(player->window), "configure_event",
			G_CALLBACK(on_player_configure), player);
	g_signal_connect(G_OBJECT(player->window), "delete_event", G_CALLBACK(
				on_player_closex), player);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(player->window), vbox);
	tb_menubar = _new_menubar(player);
	gtk_box_pack_start(GTK_BOX(vbox), tb_menubar, FALSE, FALSE, 0);
	/* statusbar */
	player->statusbar = gtk_statusbar_new();
	player->statusbar_id = 0;
	gtk_box_pack_end(GTK_BOX(vbox), player->statusbar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	player->tb_previous = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_PREVIOUS);
	g_signal_connect(player->tb_previous, "clicked", G_CALLBACK(
				on_previous), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_previous, -1);
	player->tb_rewind = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_REWIND);
	g_signal_connect(player->tb_rewind, "clicked", G_CALLBACK(
				on_rewind), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_rewind, -1);
	player->tb_play = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	g_signal_connect(player->tb_play, "clicked", G_CALLBACK(on_play),
			player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_play, -1);
	player->tb_pause = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_PAUSE);
	g_signal_connect(player->tb_pause, "clicked", G_CALLBACK(on_pause),
			player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_pause, -1);
	player->tb_stop = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	g_signal_connect(player->tb_stop, "clicked", G_CALLBACK(on_stop),
			player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_stop, -1);
	player->tb_forward = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_FORWARD);
	g_signal_connect(player->tb_forward, "clicked", G_CALLBACK(
				on_forward), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_forward, -1);
	player->tb_next = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	g_signal_connect(player->tb_next, "clicked", G_CALLBACK(
				on_next), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_next, -1);
	gtk_box_pack_end(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	gtk_widget_show_all(player->window);
	/* view */
	player->view_iheight = 0;
	gtk_widget_size_request(tb_menubar, &req);
	player->view_yoffset = req.height;
	player->view_iheight+=req.height;
	gtk_widget_size_request(toolbar, &req);
	player->view_iheight+=req.height;
	gtk_widget_size_request(player->statusbar, &req);
	player->view_iheight+=req.height;
	black = BlackPixel(GDK_DISPLAY(), DefaultScreen(GDK_DISPLAY()));
	player->view_window = XCreateSimpleWindow(GDK_DISPLAY(),
			GDK_WINDOW_XWINDOW(player->window->window), 0,
			player->view_yoffset, 400, 300 - player->view_iheight,
			0, 0, black);
	XMapWindow(GDK_DISPLAY(), player->view_window);
	/* mplayer */
	_new_mplayer(player);
	return player;
}

static int _player_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "Player: ");
	perror(message);
	return ret;
}

static GtkWidget * _new_menubar(Player * player)
{
	GtkWidget * tb_menubar;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			p = &_menubar[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == NULL)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback),
						player);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	return tb_menubar;
}

void _player_command(Player * player, char const * cmd, size_t cmd_len);
static void _new_mplayer(Player * player)
{
	char buf[] = "pausing loadfile splash.png 0\nframe_step\n";

	if((player->pid = fork()) == -1)
	{
		player_error(player, strerror(errno), 0);
		return;
	}
	if(player->pid == 0)
	{
		close(0);
		if(dup2(player->fd[0], 0) == -1)
			exit(_player_error("dup2", 2));
		snprintf(buf, sizeof(buf), "%u", (unsigned)player->view_window);
		execlp("mplayer", "mplayer", "-slave", "-wid", buf,
				"-quiet", "-idle", NULL);
		exit(_player_error("mplayer", 2));
	}
	_player_command(player, buf, strlen(buf));
}

void player_delete(Player * player)
{
	char cmd[] = "quit\n";

	_player_command(player, cmd, sizeof(cmd)-1);
	free(player);
}


/* private */
void _player_command(Player * player, char const * cmd, size_t cmd_len)
{
	if(player->pid == -1)
	{
		fprintf(stderr, "%s", "Player: mplayer not running\n");
		return;
	}
	player->atstart = 0;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s\n", "Player: mplayer ", player->pid,
			": Sending command:\n", cmd);
#endif
	if(write(player->fd[1], cmd, cmd_len) != cmd_len)
		_player_error("write", 0);
}


/* useful */
int player_error(Player * player, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(player->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
	return ret;
}


int player_sigchld(Player * player)
{
	pid_t pid;
	int status;

	if(player->pid == -1)
		return 1;
	if((pid = waitpid(player->pid, &status, WNOHANG)) == -1)
		return player_error(player, "waitpid", 1);
	if(pid == 0)
		return 1;
	fprintf(stderr, "%s", "Player: mplayer ");
	if(WIFEXITED(status))
		fprintf(stderr, "%d%s%u\n", pid, ": exited with code ",
				WEXITSTATUS(status));
	else
		fprintf(stderr, "%d%s", pid, ": Unknown state\n");
	player->pid = -1;
	return 0;
}


/* playlist management */
void player_next(Player * player)
{
	char cmd[] = "pt_step 1\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_previous(Player * player)
{
	char cmd[] = "pt_step -1\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_open(Player * player, char const * filename)
{
	char buf[512];
	size_t len;

	if(player->filename != NULL)
		free(player->filename);
	if((player->filename = strdup(filename)) == NULL)
	{
		player_error(player, strerror(errno), 0);
		return;
	}
	snprintf(buf, sizeof(buf), "%s%s", "Player - ", filename);
	gtk_window_set_title(GTK_WINDOW(player->window), buf);
	len = snprintf(buf, sizeof(buf), "%s%s%s", "pausing loadfile \"",
			player->filename, "\" 0\nframe_step\n");
	if(len >= sizeof(buf))
		fprintf(stderr, "%s", "Player: String too long\n");
	else
	{
		_player_command(player, buf, len);
		player->atstart = 1;
	}
}


void player_open_dialog(Player * player)
{
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new("Open file...",
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	player_open(player, filename);
	g_free(filename);
}


void player_queue_add(Player * player, char const * filename)
{
	char cmd[512];
	size_t len;

	len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"", filename,
			"\" 1\n");
	if(len >= sizeof(cmd))
		fprintf(stderr, "%s", "Player: String too long\n");
	else
		_player_command(player, cmd, len);
}


void player_play(Player * player)
{
	char cmd[512];
	size_t len;

	if(player->filename == NULL)
		return;
	if(player->atstart)
		len = snprintf(cmd, sizeof(cmd), "pause\n");
	else
		/* FIXME escape double quotes in filename? */
		len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"",
				player->filename, "\" 0\n");
	if(len >= sizeof(cmd))
		fprintf(stderr, "%s", "Player: String too long\n");
	else
		_player_command(player, cmd, len);
}


void player_pause(Player * player)
{
	char cmd[] = "pause\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_stop(Player * player)
{
	char cmd[] = "pausing loadfile splash.png 0\nframe_step\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_rewind(Player * player)
{
	char cmd[] = "speed_incr -0.5\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_forward(Player * player)
{
	char cmd[] = "speed_incr 0.5\n";

	_player_command(player, cmd, sizeof(cmd)-1);
}
