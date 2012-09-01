/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Player */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.\n";
/* TODO:
 * - change the progress bar for a slider */



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include <Desktop.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include "callbacks.h"
#include "../config.h"
#include "player.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* Player */
/* private */
/* types */
struct _Player
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
	int album;
	int artist;
	int title;

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
	GtkWidget * menubar;
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
};


/* constants */
#define ICON_NAME "multimedia"

static const DesktopAccel _player_accel[] =
{
	{ G_CALLBACK(on_fullscreen), GDK_CONTROL_MASK, GDK_KEY_F },
	{ G_CALLBACK(on_fullscreen), 0, GDK_KEY_F11 },
#ifdef EMBEDDED
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_KEY_W },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static const DesktopMenu _player_menu_file[] =
{
	{ N_("_Open..."), G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_edit[] =
{
	{ N_("_Preferences"), G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_view[] =
{
	{ N_("_Playlist"), G_CALLBACK(on_view_playlist), NULL, GDK_CONTROL_MASK,
		GDK_KEY_L },
# if GTK_CHECK_VERSION(2, 8, 0)
	{ N_("_Fullscreen"), G_CALLBACK(on_view_fullscreen),
		GTK_STOCK_FULLSCREEN, 0, GDK_KEY_F11 },
# else
	{ N_("_Fullscreen"), G_CALLBACK(on_view_fullscreen), NULL, 0,
		GDK_KEY_F11 },
# endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_help[] =
{
# if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
# else
	{ N_("_About"), G_CALLBACK(on_help_about), NULL, 0, 0 },
# endif
	{ NULL, NULL, NULL, 0 ,0 }
};

static const DesktopMenubar _player_menubar[] =
{
	{ N_("_File"), _player_menu_file },
	{ N_("_Edit"), _player_menu_edit },
	{ N_("_View"), _player_menu_view },
	{ N_("_Help"), _player_menu_help },
	{ NULL, NULL }
};
#endif

#ifdef EMBEDDED
static DesktopToolbar _player_toolbar[] =
{
	{ N_("Open"), G_CALLBACK(on_open), GTK_STOCK_OPEN, GDK_CONTROL_MASK,
		GDK_KEY_O, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Properties"), G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES,
		GDK_MOD1_MASK, GDK_KEY_Return, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Preferences"), G_CALLBACK(on_preferences), GTK_STOCK_PREFERENCES,
		0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};
#endif

static DesktopToolbar _player_playbar[] =
{
	{ N_("Previous"), G_CALLBACK(on_previous), GTK_STOCK_MEDIA_PREVIOUS, 0,
		0, NULL },
	{ N_("Rewind"), G_CALLBACK(on_rewind), GTK_STOCK_MEDIA_REWIND, 0, 0,
		NULL },
	{ N_("Play"), G_CALLBACK(on_play), GTK_STOCK_MEDIA_PLAY, 0, 0, NULL },
	{ N_("Pause"), G_CALLBACK(on_pause), GTK_STOCK_MEDIA_PAUSE, 0, 0,
		NULL },
	{ N_("Stop"), G_CALLBACK(on_stop), GTK_STOCK_MEDIA_STOP, 0, 0, NULL },
	{ N_("Forward"), G_CALLBACK(on_forward), GTK_STOCK_MEDIA_FORWARD, 0, 0,
		NULL },
	{ N_("Next"), G_CALLBACK(on_next), GTK_STOCK_MEDIA_NEXT, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};

static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* prototypes */
/* accessors */
static char * _player_get_filename(Player * player);
static void _player_set_metadata(Player * player, unsigned int column,
		char const * value);
static void _player_set_progress(Player * player, unsigned int progress);

/* useful */
static int _player_command(Player * player, char const * cmd, size_t cmd_len);
static int _player_error(char const * message, int ret);
static void _player_reset(Player * player, char const * filename);

/* callbacks */
static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _command_timeout(gpointer data);
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* functions */
/* player_new */
static int _player_error(char const * message, int ret);
static void _new_mplayer(Player * player);
static void _new_column_text(GtkWidget * view, char const * title, int id);

Player * player_new(void)
{
	Player * player;
	GtkAccelGroup * group;
	GtkWidget * widget;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GdkColor black = { 0, 0, 0, 0 };
	GtkTreeSelection * selection;

	if((player = malloc(sizeof(*player))) == NULL)
		return NULL;
	/* view */
	player->paused = 0;
	player->fullscreen = FALSE;
	/* current file */
	player->current = NULL;
	player->audio_codec = NULL;
	player->video_codec = NULL;
	/* mplayer */
	player->pid = -1;
	if(pipe(player->fd[0]) != 0 || pipe(player->fd[1]) != 0)
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
	group = gtk_accel_group_new();
	player->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(player->window), group);
	gtk_window_set_default_size(GTK_WINDOW(player->window), 512, 384);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(player->window), "multimedia");
#endif
	gtk_window_set_title(GTK_WINDOW(player->window), _("Media player"));
	gtk_widget_realize(player->window);
	g_signal_connect_swapped(G_OBJECT(player->window), "delete-event",
			G_CALLBACK(on_player_closex), player);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(player->window), vbox);
	desktop_accel_create(_player_accel, player, group);
#ifndef EMBEDDED
	/* menubar */
	player->menubar = desktop_menubar_create(_player_menubar, player,
			group);
#else
	/* toolbar */
	player->menubar = desktop_toolbar_create(_player_toolbar, player,
			group);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), player->menubar, FALSE, FALSE, 0);
	/* view */
	player->view_window = gtk_socket_new();
	gtk_widget_modify_bg(player->view_window, GTK_STATE_NORMAL, &black);
	g_signal_connect_swapped(G_OBJECT(player->view_window), "plug-removed",
			G_CALLBACK(on_player_removed), player);
	gtk_box_pack_start(GTK_BOX(vbox), player->view_window, TRUE, TRUE, 0);
#ifndef EMBEDDED
	/* statusbar */
	player->statusbar = gtk_statusbar_new();
	player->statusbar_id = 0;
	gtk_box_pack_end(GTK_BOX(vbox), player->statusbar, FALSE, FALSE, 0);
#endif
	/* playbar */
	toolbar = desktop_toolbar_create(_player_playbar, player, group);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	player->progress = gtk_progress_bar_new();
	_player_set_progress(player, 0);
	gtk_container_add(GTK_CONTAINER(toolitem), player->progress);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	widget = gtk_image_new_from_icon_name("stock_fullscreen",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	player->tb_fullscreen = gtk_tool_button_new(widget, _("Fullscreen"));
	g_signal_connect_swapped(player->tb_fullscreen, "clicked", G_CALLBACK(
				on_fullscreen), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_fullscreen, -1);
	gtk_box_pack_end(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	gtk_widget_show_all(player->window);
	/* playlist */
	/* FIXME make it dockable */
	player->pl_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(player->pl_window), _("Playlist"));
	g_signal_connect_swapped(G_OBJECT(player->pl_window), "delete-event",
			G_CALLBACK(on_playlist_closex), player);
	vbox = gtk_vbox_new(FALSE, 0);
	/* view */
	player->pl_store = gtk_list_store_new(PL_NUM_COLS,
			G_TYPE_BOOLEAN,	/* enabled */
			GDK_TYPE_PIXBUF,/* icon */
			G_TYPE_STRING,	/* filename */
			G_TYPE_UINT,	/* track number */
			G_TYPE_STRING,	/* artist */
			G_TYPE_STRING,	/* album */
			G_TYPE_STRING);	/* name */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	player->pl_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				player->pl_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(player->pl_view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(player->pl_view), TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(player->pl_view),
			gtk_tree_view_column_new_with_attributes("",
				gtk_cell_renderer_pixbuf_new(), "pixbuf",
				PL_COL_ICON, NULL));
	_new_column_text(player->pl_view, _("Artist"), PL_COL_ARTIST);
	_new_column_text(player->pl_view, _("Album"), PL_COL_ALBUM);
	_new_column_text(player->pl_view, _("Title"), PL_COL_TITLE);
	g_signal_connect_swapped(G_OBJECT(player->pl_view), "row-activated",
			G_CALLBACK(on_playlist_activated), player);
	gtk_container_add(GTK_CONTAINER(widget), player->pl_view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_playlist_load), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_playlist_save), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_playlist_add), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				on_playlist_remove), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(player->pl_window), vbox);
	/* about window */
	player->ab_window = NULL;
	/* preferences window */
	player->pr_window = NULL;
#ifdef DEBUG
	gtk_widget_show_all(player->pl_window);
#else
	gtk_widget_show_all(vbox);
#endif
	/* mplayer */
	_new_mplayer(player);
	return player;
}

static void _new_mplayer(Player * player)
{
	char buf[] = "pausing loadfile splash.png 0\nframe_step\n";
	char wid[16];
	char * argv[] = { "mplayer", "-slave", "-wid", NULL, "-quiet",
		"-idle", "-framedrop", "-softvol", "-identify",
		"-noconsolecontrols", "-nomouseinput", NULL };

	argv[3] = wid;
	_player_reset(player, NULL);
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
		execvp(argv[0], argv);
		exit(_player_error(argv[0], 2));
	}
	close(player->fd[0][1]);
	close(player->fd[1][0]);
	player->channel[0] = g_io_channel_unix_new(player->fd[0][0]);
	player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
			_command_read, player);
	player->channel[1] = g_io_channel_unix_new(player->fd[1][1]);
	_player_command(player, buf, sizeof(buf) - 1);
	player->paused = 1;
}

static void _new_column_text(GtkWidget * view, char const * title, int id)
{
	GtkTreeViewColumn * column;
	GtkCellRenderer * renderer;

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes(title, renderer,
			"text", id, NULL);
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_tree_view_column_set_expand(column, TRUE);
#endif
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, id);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
}


/* player_delete */
void player_delete(Player * player)
{
	char const cmd[] = "\nquit\n";
	gsize written;
	size_t i;
	int status = 0;
	pid_t res;
	struct timespec ts = { 0, 500000 };

	if(player->read_id != 0)
		g_source_remove(player->read_id);
	if(player->timeout_id != 0)
		g_source_remove(player->timeout_id);
	g_io_channel_write(player->channel[1], cmd, sizeof(cmd) - 1, &written);
	g_io_channel_shutdown(player->channel[1], FALSE, NULL);
	for(i = 0; i < 6; i++)
	{
		if((res = waitpid(player->pid, &status, WNOHANG)) == -1)
		{
			_player_error("waitpid", 0);
			break;
		}
		else if(res == 0)
			nanosleep(&ts, NULL);
		else if(WIFEXITED(status) || WIFSIGNALED(status))
				break;
		if(i == 4)
			kill(player->pid, SIGTERM);
	}
	free(player);
}


/* accessors */
/* player_get_fullscreen */
gboolean player_get_fullscreen(Player * player)
{
	return player->fullscreen;
}


/* player_set_fullscreen */
void player_set_fullscreen(Player * player, gboolean fullscreen)
{
	if(player->fullscreen == fullscreen)
		return;
	if(fullscreen)
	{
		gtk_widget_hide(player->menubar);
#ifndef EMBEDDED
		gtk_widget_hide(player->statusbar);
#endif
		gtk_window_fullscreen(GTK_WINDOW(player->window));
	}
	else
	{
		gtk_window_unfullscreen(GTK_WINDOW(player->window));
		gtk_widget_show(player->menubar);
#ifndef EMBEDDED
		gtk_widget_show(player->statusbar);
#endif
	}
	player->fullscreen = fullscreen;
}


/* player_set_size */
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
/* player_about */
static gboolean _about_on_closex(gpointer data);

void player_about(Player * player)
{
	if(player->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(player->ab_window));
		return;
	}
	player->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(player->ab_window), GTK_WINDOW(
				player->window));
	desktop_about_dialog_set_authors(player->ab_window, _authors);
	desktop_about_dialog_set_comments(player->ab_window,
			_("Media player for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(player->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(player->ab_window, ICON_NAME);
	desktop_about_dialog_set_license(player->ab_window, _license);
	desktop_about_dialog_set_name(player->ab_window, PACKAGE);
	desktop_about_dialog_set_translator_credits(player->ab_window,
			_("translator-credits"));
	desktop_about_dialog_set_version(player->ab_window, VERSION);
	g_signal_connect_swapped(G_OBJECT(player->ab_window), "delete-event",
			G_CALLBACK(_about_on_closex), player);
	gtk_widget_show(player->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->ab_window);
	return TRUE;
}


/* player_error */
int player_error(Player * player, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(player->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
	return ret;
}


/* player_forward */
void player_forward(Player * player)
{
	char const cmd[] = "speed_incr 0.5\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
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
	fputs("player: mplayer ", stderr);
	if(WIFEXITED(status))
		fprintf(stderr, "%d%s%u\n", pid, ": exited with code ",
				WEXITSTATUS(status));
	else
		fprintf(stderr, "%d%s", pid, ": Unknown state\n");
	player->pid = -1;
	return 0;
}


/* player_next */
void player_next(Player * player)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreePath * path;
	GtkTreeIter iter;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	if(gtk_tree_model_get_iter(model, &iter, path) != TRUE)
		return;
	if(gtk_tree_model_iter_next(model, &iter) != TRUE)
		return;
	gtk_tree_row_reference_free(player->current);
	path = gtk_tree_model_get_path(model, &iter);
	player->current = gtk_tree_row_reference_new(model, path);
	gtk_tree_path_free(path);
	player_play(player);
}


/* player_open */
int player_open(Player * player, char const * filename)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreeIter iter;
	GtkTreePath * path;
	char cmd[512];
	size_t len;

	player_playlist_clear(player);
	player_playlist_add(player, filename);
	if(gtk_tree_model_get_iter_first(model, &iter) == TRUE)
	{
		path = gtk_tree_model_get_path(model, &iter);
		player->current = gtk_tree_row_reference_new(model, path);
		gtk_tree_path_free(path);
	}
	_player_reset(player, filename);
	len = snprintf(cmd, sizeof(cmd), "%s%s%s", "pausing loadfile \"",
			filename, "\" 0\nframe_step\n");
	if(len >= sizeof(cmd))
	{
		fputs("player: String too long\n", stderr);
		return 1;
	}
	if(_player_command(player, cmd, len) != 0)
		return 1;
	player->paused = 1;
	return 0;
}


/* player_open_dialog */
int player_open_dialog(Player * player)
{
	int ret;
	GtkWidget * dialog;
	GtkFileFilter * filter;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Video files"));
	gtk_file_filter_add_mime_type(filter, "video/3gpp");
	gtk_file_filter_add_mime_type(filter, "video/3gpp2");
	gtk_file_filter_add_mime_type(filter, "video/mp2t");
	gtk_file_filter_add_mime_type(filter, "video/mp4");
	gtk_file_filter_add_mime_type(filter, "video/mpeg");
	gtk_file_filter_add_mime_type(filter, "video/ogg");
	gtk_file_filter_add_mime_type(filter, "video/quicktime");
	gtk_file_filter_add_mime_type(filter, "video/vivo");
	gtk_file_filter_add_mime_type(filter, "video/vnd.vn-realvideo");
	gtk_file_filter_add_mime_type(filter, "video/webm");
	gtk_file_filter_add_mime_type(filter, "video/x-flv");
	gtk_file_filter_add_mime_type(filter, "video/x-javafx");
	gtk_file_filter_add_mime_type(filter, "video/x-matroska");
	gtk_file_filter_add_mime_type(filter, "video/x-mng");
	gtk_file_filter_add_mime_type(filter, "video/x-ms-asf");
	gtk_file_filter_add_mime_type(filter, "video/x-ms-wmv");
	gtk_file_filter_add_mime_type(filter, "video/x-msvideo");
	gtk_file_filter_add_mime_type(filter, "video/x-ogm+ogg");
	gtk_file_filter_add_mime_type(filter, "video/x-sgi-movie");
	gtk_file_filter_add_mime_type(filter, "video/x-theora+ogg");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Audio files"));
	gtk_file_filter_add_mime_type(filter, "audio/ac3");
	gtk_file_filter_add_mime_type(filter, "audio/mp4");
	gtk_file_filter_add_mime_type(filter, "audio/mpeg");
	gtk_file_filter_add_mime_type(filter, "audio/vorbis");
	gtk_file_filter_add_mime_type(filter, "audio/x-mp2");
	gtk_file_filter_add_mime_type(filter, "audio/x-mp3");
	gtk_file_filter_add_mime_type(filter, "audio/x-ms-wma");
	gtk_file_filter_add_mime_type(filter, "audio/x-vorbis");
	gtk_file_filter_add_mime_type(filter, "audio/x-wav");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
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


/* player_pause */
void player_pause(Player * player)
{
	char const cmd[] = "pause\n";

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


/* player_play */
int player_play(Player * player)
{
	char cmd[512];
	size_t len;
	char * filename;

	if((filename = _player_get_filename(player)) == NULL)
		return 1;
	/* FIXME escape double quotes in filename? */
	if(player->paused == 1)
		len = snprintf(cmd, sizeof(cmd), "%s", "pause\n");
	else if((len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"",
					filename, "\" 0\n"))
			>= sizeof(cmd))
	{
		fputs("player: String too long\n", stderr);
		return 1;
	}
	else
		_player_reset(player, filename);
	free(filename);
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


/* player_playlist_add */
void player_playlist_add(Player * player, char const * filename)
{
	GtkTreeIter iter;

	/* FIXME fetch the actual artists/albums/titles */
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_list_store_insert_with_values(player->pl_store, &iter, -1,
#else
	gtk_list_store_insert_after(player->pl_store, iter, NULL);
	gtk_list_store_set(player->pl_store, iter,
#endif
			PL_COL_FILENAME, filename,
			PL_COL_ALBUM, _("Unknown album"),
			PL_COL_ARTIST, _("Unknown artist"),
			PL_COL_TITLE, _("Unknown title"), -1);
}


/* player_playlist_add_dialog */
void player_playlist_add_dialog(Player * player)
{
	GtkWidget * widget;
	GSList * files = NULL;
	GSList * p;
	char const * filename;

	widget = gtk_file_chooser_dialog_new(_("Add file(s)..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(widget), TRUE);
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_ACCEPT)
		files = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(widget));
	gtk_widget_destroy(widget);
	for(p = files; p != NULL; p = p->next)
	{
		filename = p->data;
		if(strncmp(filename, "file://", 7) == 0)
			player_playlist_add(player, &filename[7]);
	}
	g_slist_foreach(files, (GFunc)g_free, NULL);
	g_slist_free(files);
}


/* player_playlist_clear */
void player_playlist_clear(Player * player)
{
	if(player->current != NULL)
		gtk_tree_row_reference_free(player->current);
	player->current = NULL;
	gtk_list_store_clear(player->pl_store);
}


/* player_playlist_open */
void player_playlist_open(Player * player, char const * filename)
{
	/* FIXME implement */
}


/* player_playlist_open_dialog */
void player_playlist_open_dialog(Player * player)
{
	GtkWidget * widget;
	gchar * filename = NULL;

	widget = gtk_file_chooser_dialog_new(_("Open playlist..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(widget)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					widget));
	gtk_widget_destroy(widget);
	if(filename != NULL)
		player_playlist_open(player, filename);
	g_free(filename);
}


/* player_playlist_play_selected */
void player_playlist_play_selected(Player * player)
{
	GtkTreeSelection * selection;
	GList * rows;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(player->pl_view));
	if((rows = gtk_tree_selection_get_selected_rows(selection, NULL))
			== NULL)
		return;
	if(rows->data != NULL && rows->next == NULL)
	{
		if(player->current != NULL)
			gtk_tree_row_reference_free(player->current);
		player->current = gtk_tree_row_reference_new(GTK_TREE_MODEL(
					player->pl_store), rows->data);
		player_play(player);
	}
	g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(rows);
}


/* player_playlist_save_as_dialog */
void player_playlist_save_as_dialog(Player * player)
{
	/* FIXME implement */
}


/* player_previous */
void player_previous(Player * player)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreePath * path;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	if(gtk_tree_path_prev(path) != TRUE)
		return;
	path = gtk_tree_path_copy(path);
	gtk_tree_row_reference_free(player->current);
	player->current = gtk_tree_row_reference_new(model, path);
	gtk_tree_path_free(path);
	player_play(player);
}


/* player_rewind */
void player_rewind(Player * player)
{
	char const cmd[] = "speed_incr -0.5\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
}


/* player_stop */
void player_stop(Player * player)
{
	char const cmd[] = "pausing loadfile splash.png 0\nframe_step\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
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


/* player_view_playlist */
void player_view_playlist(Player * player, gboolean view)
{
	if(view)
		gtk_window_present(GTK_WINDOW(player->pl_window));
	else
		gtk_widget_hide(player->pl_window);
}


/* player_view_preferences */
static void _preferences_set(Player * player);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void player_view_preferences(Player * player)
{
	GtkWidget * vbox;

	if(player->pr_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(player->pr_window));
		return;
	}
	player->pr_window = gtk_dialog_new_with_buttons(
			_("Media player preferences"),
			GTK_WINDOW(player->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(G_OBJECT(player->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), player);
	g_signal_connect(G_OBJECT(player->pr_window), "response",
			G_CALLBACK(_preferences_on_response), player);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(player->pr_window));
#else
	vbox = GTK_DIALOG(player->pr_window)->vbox;
#endif
	/* FIXME implement */
	_preferences_set(player);
	gtk_widget_show_all(player->pr_window);
}

static void _preferences_set(Player * player)
{
	/* FIXME implement */
}

static gboolean _preferences_on_closex(gpointer data)
{
	Player * player = data;

	_preferences_on_cancel(player);
	return TRUE;
}

static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_OK)
		_preferences_on_ok(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_preferences_on_cancel(data);
}

static void _preferences_on_cancel(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->pr_window);
	_preferences_set(player);
}

static void _preferences_on_ok(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->pr_window);
	/* FIXME implement */
}


/* player_view_properties */
void player_view_properties(Player * player)
{
	GtkWidget * dialog;
	char * filename;
	char buf[256];

	if((filename = _player_get_filename(player)) == NULL)
		return;
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), basename(
				filename));
	free(filename);
	dialog = gtk_dialog_new_with_buttons(buf, GTK_WINDOW(player->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 200);
	/* FIXME implement */
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


/* private */
/* functions */
/* accessors */
/* player_get_filename */
static char * _player_get_filename(Player * player)
{
	char * ret;
	GtkTreePath * path;
	GtkTreeIter iter;

	if(player->current == NULL)
		return NULL;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return NULL;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(player->pl_store), &iter,
				path) != TRUE)
		return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(player->pl_store), &iter,
			PL_COL_FILENAME, &ret, -1);
	return ret;
}


/* player_set_metadata */
static void _player_set_metadata(Player * player, unsigned int column,
		char const * value)
{
	GtkTreePath * path;
	GtkTreeIter iter;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(player->pl_store), &iter,
				path) != TRUE)
		return;
	gtk_list_store_set(player->pl_store, &iter, column, value, -1);
}


/* player_set_progress */
static void _player_set_progress(Player * player, unsigned int progress)
{
	gdouble fraction;
	char buf[16];

	fraction = (progress <= 100) ? progress : 100;
	fraction /= 100;
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(player->progress),
			fraction);
	snprintf(buf, sizeof(buf), "%u%%", progress);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(player->progress), buf);
}


/* useful */
/* player_error */
static int _player_error(char const * message, int ret)
{
	fputs("player: ", stderr);
	perror(message);
	return ret;
}


/* player_reset */
static void _player_reset(Player * player, char const * filename)
{
	char * p = NULL;
	char buf[256];

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
	player->album = -1;
	player->artist = -1;
	player->title = -1;
	_player_set_progress(player, 0);
	if(filename != NULL)
		p = strdup(filename);
	snprintf(buf, sizeof(buf), "%s%s%s", _("Media player"),
			(filename != NULL) ? " - " : "", (filename != NULL)
			? ((p != NULL) ? basename(p) : filename) : "");
	free(p);
	gtk_window_set_title(GTK_WINDOW(player->window), buf);
}


/* player_command */
static int _player_command(Player * player, char const * cmd, size_t cmd_len)
{
	char * p;

	if(player->pid == -1)
	{
		fputs("player: mplayer not running\n", stderr);
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
	else if(sscanf(buf, "ID_CLIP_INFO_NAME%u=%255s", &u32, str) == 2)
	{
		if(strcmp(str, "Album") == 0)
			player->album = u32;
		else if(strcmp(str, "Artist") == 0)
			player->artist = u32;
		else if(strcmp(str, "Title") == 0)
			player->title = u32;
	}
	else if(sscanf(buf, "ID_CLIP_INFO_VALUE%u=%255[^\n]", &u32, str) == 2)
	{
		if(player->album >= 0 && (unsigned)player->album == u32)
			_player_set_metadata(player, PL_COL_ALBUM, str);
		else if(player->artist >= 0 && (unsigned)player->artist == u32)
			_player_set_metadata(player, PL_COL_ARTIST, str);
		else if(player->title >= 0 && (unsigned)player->title == u32)
			_player_set_metadata(player, PL_COL_TITLE, str);
	}
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
