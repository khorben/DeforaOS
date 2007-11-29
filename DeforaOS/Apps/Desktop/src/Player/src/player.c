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
#include <libgen.h>
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
	{ "_Properties", G_CALLBACK(on_file_properties), GTK_STOCK_PROPERTIES },
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
/* private */
/* prototypes */
/* accessors */
static void _player_set_progress(Player * player, unsigned int progress);

/* useful */
static int _player_error(char const * message, int ret);
static void _player_reset(Player * player);

/* callbacks */
static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _command_timeout(gpointer data);
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* functions */
/* accessors */
static void _player_set_progress(Player * player, unsigned int progress)
{
	gdouble fraction;
	static char buf[16];

	fraction = progress <= 100 ? progress : 100;
	fraction /= 100;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(player->progress),
			fraction);
	snprintf(buf, sizeof(buf), "%d%%", progress);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(player->progress), buf);
}


/* useful */
/* player_error */
static int _player_error(char const * message, int ret)
{
	fputs("Player: ", stderr);
	perror(message);
	return ret;
}


/* player_reset */
static void _player_reset(Player * player)
{
	if(player->filename != NULL)
		free(player->filename);
	player->filename = NULL;
	player->width = 0;
	player->height = 0;
	player->audio_bitrate = 0;
	player->audio_channels = 0;
	if(player->audio_codec != NULL)
		free(player->audio_codec);
	player->audio_codec = NULL;
	player->audio_rate = 0;
	player->video_aspect = 0.0;
	player->video_bitrate = 0;
	if(player->video_codec != NULL)
		free(player->video_codec);
	player->video_codec = NULL;
	player->video_fps = 0.0;
	player->video_rate = 0;
	_player_set_progress(player, 0);
}


/* player_command */
static int _player_command(Player * player, char const * cmd, size_t cmd_len)
{
	char * p;

	if(player->pid == -1)
	{
		fputs("Player: mplayer not running\n", stderr);
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s%d%s\"%s\"\n", "DEBUG: pid ", player->pid,
			": write ", cmd);
#endif
	if((p = realloc(player->buf, player->buf_len + cmd_len)) == NULL)
		return _player_error("malloc", 1);
	player->buf = p;
	memcpy(&p[player->buf_len], cmd, cmd_len);
	player->buf_len += cmd_len;
	g_io_add_watch(player->channel[1], G_IO_OUT, _command_write, player);
	return 0;
}


/* callbacks */
/* command_read */
static void _read_parse(Player * player, char const * buf);

static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Player * player = data;
	static char buf[512];
	static size_t buf_len = 0;
	gsize read;
	size_t i;
	size_t j;

	if(condition != G_IO_IN)
	{
		player_error(player, "", 0); /* FIXME */
		gtk_main_quit();
		return FALSE; /* FIXME report error */
	}
	if(g_io_channel_read(source, &buf[buf_len], sizeof(buf) - buf_len,
				&read) != G_IO_ERROR_NONE)
	{
		player_error(player, "", 0); /* FIXME */
		gtk_main_quit();
		return FALSE; /* FIXME report error */
	}
	if(read == 0)
	{
		player->read_id = 0;
		return FALSE; /* FIXME end of file? */
	}
	buf_len += read;
	j = 0;
	for(i = 0; i < buf_len; i++)
	{
		if(buf[i] != '\n')
			continue;
		buf[i] = '\0';
		_read_parse(player, &buf[j]);
		j = i + 1;
	}
	buf_len -= j;
	memmove(buf, &buf[j], buf_len);
	return TRUE;
}

static void _read_parse(Player * player, char const * buf)
{
	unsigned int u32;
	gdouble db;
	char str[256];

	if(sscanf(buf, "ANS_PERCENT_POSITION=%u\n", &u32) == 1)
		_player_set_progress(player, u32);
	else if(sscanf(buf, "ID_AUDIO_BITRATE=%u\n", &u32) == 1)
		player->audio_bitrate = u32;
	else if(sscanf(buf, "ID_AUDIO_CODEC=%255s", str) == 1)
	{
		if(player->audio_codec != NULL)
			free(player->audio_codec);
		player->audio_codec = strdup(str);
	}
	else if(sscanf(buf, "ID_AUDIO_NCH=%u\n", &u32) == 1)
		player->audio_channels = u32;
	else if(sscanf(buf, "ID_AUDIO_RATE=%u\n", &u32) == 1)
		player->audio_rate = u32;
	else if(sscanf(buf, "ID_LENGTH=%lf\n", &db) == 1)
		player->length = db;
	else if(sscanf(buf, "ID_VIDEO_ASPECT=%lf\n", &db) == 1)
		player->video_aspect = db;
	else if(sscanf(buf, "ID_VIDEO_BITRATE=%u\n", &u32) == 1)
		player->video_bitrate = u32;
	else if(sscanf(buf, "ID_VIDEO_CODEC=%255s", str) == 1)
	{
		if(player->video_codec != NULL)
			free(player->video_codec);
		player->video_codec = strdup(str);
	}
	else if(sscanf(buf, "ID_VIDEO_FPS=%lf\n", &db) == 1)
		player->video_fps = db;
	else if(sscanf(buf, "ID_VIDEO_HEIGHT=%u\n", &u32) == 1)
		player_set_size(player, -1, u32);
	else if(sscanf(buf, "ID_VIDEO_RATE=%u\n", &u32) == 1)
		player->video_rate = u32;
	else if(sscanf(buf, "ID_VIDEO_WIDTH=%u\n", &u32) == 1)
		player_set_size(player, u32, -1);
#ifdef DEBUG
	else
		fprintf(stderr, "DEBUG: unknown output \"%s\"\n", buf);
#endif
}


/* command_timeout */
static gboolean _command_timeout(gpointer data)
{
	Player * player = data;
	static const char cmd[] = "pausing_keep get_percent_pos\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
	return TRUE;
}


/* command_write */
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Player * player = data;
	gsize written;
	char * p;

	if(condition != G_IO_OUT)
	{
		player_error(player, "", 0); /* FIXME */
		gtk_main_quit();
		return FALSE; /* FIXME report error */
	}
	if(g_io_channel_write(source, player->buf, player->buf_len, &written)
			!= G_IO_ERROR_NONE)
	{
		player_error(player, "", 0); /* FIXME */
		gtk_main_quit();
		return FALSE; /* FIXME report error */
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: wrote %zu bytes\n", written);
#endif
	player->buf_len -= written;
	memmove(player->buf, &player->buf[written], player->buf_len);
	if(player->buf_len == 0)
		return FALSE;
	if((p = realloc(player->buf, player->buf_len)) != NULL)
		player->buf = p;
	return TRUE;
}


/* public */
/* player_new */
static int _player_error(char const * message, int ret);
static GtkWidget * _new_menubar(Player * player);
static void _new_mplayer(Player * player);
#if !GTK_CHECK_VERSION(2, 12, 0)
static void gtk_widget_set_tooltip_text(GtkWidget * widget, const char * text);
#endif

Player * player_new(void)
{
	Player * player;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;

	if((player = malloc(sizeof(*player))) == NULL)
		return NULL;
	/* view */
	player->paused = 0;
	player->fullscreen = 0;
	/* current file */
	player->filename = NULL;
	player->audio_codec = NULL;
	player->video_codec = NULL;
	/* mplayer */
	player->pid = -1;
	if(pipe(player->fd[0]) != 0
			|| pipe(player->fd[1]) != 0)
	{
		player_error(player, strerror(errno), 0);
		free(player);
		return NULL;
	}
	player->buf = NULL;
	player->buf_len = 0;
	/* callbacks */
	player->read_id = 0;
	player->timeout_id = 0;
	/* widgets */
	player->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(player->window), 512, 384);
	gtk_window_set_title(GTK_WINDOW(player->window), "Player");
	gtk_widget_realize(player->window);
	g_signal_connect(G_OBJECT(player->window), "delete-event", G_CALLBACK(
				on_player_closex), player);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(player->window), vbox);
	player->menubar = _new_menubar(player);
	gtk_box_pack_start(GTK_BOX(vbox), player->menubar, FALSE, FALSE, 0);
	/* view */
	player->view_window = gtk_socket_new();
	gtk_box_pack_start(GTK_BOX(vbox), player->view_window, TRUE, TRUE, 0);
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
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_previous),
			"Previous");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_previous, -1);
	player->tb_rewind = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_REWIND);
	g_signal_connect(player->tb_rewind, "clicked", G_CALLBACK(
				on_rewind), player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_rewind),
			"Rewind");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_rewind, -1);
	player->tb_play = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
	g_signal_connect(player->tb_play, "clicked", G_CALLBACK(on_play),
			player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_play), "Play");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_play, -1);
	player->tb_pause = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_PAUSE);
	g_signal_connect(player->tb_pause, "clicked", G_CALLBACK(on_pause),
			player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_pause), "Pause");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_pause, -1);
	player->tb_stop = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
	g_signal_connect(player->tb_stop, "clicked", G_CALLBACK(on_stop),
			player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_stop), "Stop");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_stop, -1);
	player->tb_forward = gtk_tool_button_new_from_stock(
			GTK_STOCK_MEDIA_FORWARD);
	g_signal_connect(player->tb_forward, "clicked", G_CALLBACK(
				on_forward), player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_forward), "Forward");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_forward, -1);
	player->tb_next = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_NEXT);
	g_signal_connect(player->tb_next, "clicked", G_CALLBACK(
				on_next), player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_next), "Next");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_next, -1);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	player->progress = gtk_progress_bar_new();
	_player_set_progress(player, 0);
	gtk_container_add(GTK_CONTAINER(toolitem), player->progress);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	player->tb_fullscreen = gtk_tool_button_new_from_stock(
			GTK_STOCK_FULLSCREEN);
	g_signal_connect(player->tb_fullscreen, "clicked", G_CALLBACK(
				on_fullscreen), player);
	gtk_widget_set_tooltip_text(GTK_WIDGET(player->tb_fullscreen),
			"Fullscreen");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_fullscreen, -1);
	gtk_box_pack_end(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	gtk_widget_show_all(player->window);
	/* mplayer */
	_new_mplayer(player);
	return player;
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

static void _new_mplayer(Player * player)
{
	char buf[] = "pausing loadfile splash.png 0\nframe_step\n";
	char wid[16];

	_player_reset(player);
	snprintf(wid, sizeof(wid), "%u", gtk_socket_get_id(GTK_SOCKET(
					player->view_window)));
	if((player->pid = fork()) == -1)
	{
		player_error(player, strerror(errno), 0);
		return;
	}
	if(player->pid == 0) /* child */
	{
		close(player->fd[0][0]);
		close(player->fd[1][1]);
		if(dup2(player->fd[1][0], 0) == -1)
			exit(_player_error("dup2", 2));
		if(dup2(player->fd[0][1], 1) == -1)
			exit(_player_error("dup2", 2));
		execlp("mplayer", "mplayer", "-slave", "-wid", wid, "-quiet",
				"-idle", "-softvol", "-identify",
				"-noconsolecontrols", NULL);
		exit(_player_error("mplayer", 2));
	}
	close(player->fd[0][1]);
	close(player->fd[1][0]);
	player->channel[0] = g_io_channel_unix_new(player->fd[0][0]);
	player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
			_command_read, player);
	player->channel[1] = g_io_channel_unix_new(player->fd[1][1]);
	_player_command(player, buf, strlen(buf));
	player->paused = 1;
}

#if !GTK_CHECK_VERSION(2, 12, 0)
static void gtk_widget_set_tooltip_text(GtkWidget * widget, const char * text)
{
	static GtkTooltips * tooltips = NULL;

	if(tooltips == NULL)
	{
		tooltips = gtk_tooltips_new();
		gtk_tooltips_enable(tooltips);
	}
	gtk_tooltips_set_tip(tooltips, widget, text, NULL);
}
#endif


/* player_delete */
void player_delete(Player * player)
{
	char cmd[] = "quit\n";

	if(player->read_id != 0)
		g_source_remove(player->read_id);
	if(player->timeout_id != 0)
		g_source_remove(player->timeout_id);
	_player_command(player, cmd, sizeof(cmd) - 1);
	free(player);
}


/* accessors */
int player_get_fullscreen(Player * player)
{
	return player->fullscreen ? 1 : 0;
}


void player_set_fullscreen(Player * player, int fullscreen)
{
	if(fullscreen)
	{
		if(player->fullscreen)
			return;
		gtk_widget_hide(player->menubar);
		gtk_widget_hide(player->statusbar);
		gtk_window_fullscreen(GTK_WINDOW(player->window));
		player->fullscreen = !player->fullscreen;
		return;
	}
	if(!player->fullscreen)
		return;
	gtk_window_unfullscreen(GTK_WINDOW(player->window));
	gtk_widget_show(player->menubar);
	gtk_widget_show(player->statusbar);
	player->fullscreen = !player->fullscreen;
}


void player_set_size(Player * player, int width, int height)
{
	if(width < 0)
		width = player->width;
	if(height < 0)
		height = player->height;
	gtk_widget_set_size_request(player->view_window, width, height);
	player->width = width;
	player->height = height;
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
	fputs("Player: mplayer ", stderr);
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


int player_open(Player * player, char const * filename)
{
	char cmd[512];
	size_t len;
	char * p;

	_player_reset(player);
	if((player->filename = strdup(filename)) == NULL)
		return player_error(player, strerror(errno), 1);
	len = snprintf(cmd, sizeof(cmd), "%s%s%s", "pausing loadfile \"",
			player->filename, "\" 0\nframe_step\n");
	if(len >= sizeof(cmd))
	{
		fputs("Player: String too long\n", stderr);
		return 1;
	}
	if(_player_command(player, cmd, len) != 0)
		return 1;
	player->paused = 1;
	p = strdup(filename);
	snprintf(cmd, sizeof(cmd), "%s%s", "Player - ", p != NULL ? basename(p)
			: filename);
	free(p);
	gtk_window_set_title(GTK_WINDOW(player->window), cmd);
	return 0;
}


int player_open_dialog(Player * player)
{
	int ret;
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
		return 1;
	ret = player_open(player, filename);
	g_free(filename);
	return ret;
}


void player_queue_add(Player * player, char const * filename)
{
	char cmd[512];
	size_t len;

	len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"", filename,
			"\" 1\n");
	if(len >= sizeof(cmd))
		fputs("Player: String too long\n", stderr);
	else
		_player_command(player, cmd, len);
}


/* player_play */
int player_play(Player * player)
{
	char cmd[512];
	size_t len;

	if(player->filename == NULL)
		return 0;
	/* FIXME escape double quotes in filename? */
	if(player->paused == 1)
	{
		strcpy(cmd, "pause\n");
		len = 6;
	}
	else if((len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"",
					player->filename, "\" 0\n"))
			>= sizeof(cmd))
	{
		fputs("Player: String too long\n", stderr);
		return 1;
	}
	else
		_player_reset(player);
	if(_player_command(player, cmd, len) != 0)
		return 1;
	player->paused = 0;
	if(player->read_id == 0)
		player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
				_command_read, player);
	if(player->timeout_id == 0)
		player->timeout_id = g_timeout_add(500, _command_timeout,
				player);
	return 0;
}


void player_pause(Player * player)
{
	char cmd[] = "pause\n";

	if(player->filename == NULL)
		return;
	if(player->paused != 0)
	{
		if(player->timeout_id == 0)
			player->timeout_id = g_timeout_add(500,
					_command_timeout, player);
	}
	else if(player->timeout_id != 0)
	{
		g_source_remove(player->timeout_id);
		player->timeout_id = 0;
	}
	_player_command(player, cmd, sizeof(cmd) - 1);
	player->paused = player->paused == 1 ? 0 : 1;
}


void player_stop(Player * player)
{
	char cmd[] = "pausing loadfile splash.png 0\nframe_step\n";

	_player_command(player, cmd, sizeof(cmd)-1);
	_player_set_progress(player, 0);
	player->paused = 0; /* FIXME also needs a stopped state */
	if(player->read_id != 0)
	{
		g_source_remove(player->read_id);
		player->read_id = 0;
	}
	if(player->timeout_id != 0)
	{
		g_source_remove(player->timeout_id);
		player->timeout_id = 0;
	}
}


void player_rewind(Player * player)
{
	char cmd[] = "speed_incr -0.5\n";

	if(player->filename == NULL)
		return;
	_player_command(player, cmd, sizeof(cmd)-1);
}


void player_forward(Player * player)
{
	char cmd[] = "speed_incr 0.5\n";

	if(player->filename == NULL)
		return;
	_player_command(player, cmd, sizeof(cmd)-1);
}
