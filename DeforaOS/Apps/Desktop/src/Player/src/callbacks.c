/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2010 Pierre Pronchery <khorben@defora.org>";
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include "player.h"
#include "callbacks.h"
#include "../config.h"
#define _(string) gettext(string)


/* macros */
#define max(a, b) (((a) > (b)) ? (a) : (b))


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* callbacks */
/* player */
/* on_player_closex */
gboolean on_player_closex(gpointer data)
{
	on_file_close(data);
	return TRUE;
}


/* on_player_removed */
void on_player_removed(gpointer data)
{
	/* FIXME implement */
}


/* file menu */
void on_file_open(gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
}


void on_file_properties(gpointer data)
{
	Player * player = data;
	GtkWidget * window;
	char * p;
	char buf[256];

	if(player->filename == NULL)
		return;
	if((p = strdup(player->filename)) == NULL)
	{
		player_error(player, strerror(errno), 0);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_widget_destroy), NULL);
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), basename(p));
	free(p);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	/* FIXME implement */
	gtk_widget_show_all(window);
}


void on_file_close(gpointer data)
{
	Player * player = data;

	player_stop(player);
	gtk_main_quit();
}


/* edit menu */
void on_edit_preferences(gpointer data)
{
	Player * player = data;
	static GtkWidget * window = NULL;


	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("Preferences"));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				player->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_widget_hide), NULL); /* FIXME cancel */
	/* FIXME implement */
	gtk_widget_show_all(window);
}


/* view menu */
/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* on_view_playlist */
void on_view_playlist(gpointer data)
{
	Player * player = data;

	gtk_widget_show(player->pl_window);
}


/* help menu */
static gboolean _on_about_closex(gpointer data);

void on_help_about(gpointer data)
{
	Player * player = data;
	static GtkWidget * window = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		player_error(player, "malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				player->window));
	g_signal_connect_swapped(G_OBJECT(window), "delete-event", G_CALLBACK(
				_on_about_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(window),
			"multimedia");
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	/* FIXME use libDesktop */
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */

static gboolean _on_about_closex(gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
	return TRUE;
}


/* toolbar */
void on_previous(gpointer data)
{
	Player * player = data;

	player_previous(player);
}


void on_rewind(gpointer data)
{
	Player * player = data;

	player_rewind(player);
}


void on_play(gpointer data)
{
	Player * player = data;

	player_play(player);
}


void on_pause(gpointer data)
{
	Player * player = data;

	player_pause(player);
}


void on_stop(gpointer data)
{
	Player * player = data;

	player_stop(player);
}


void on_forward(gpointer data)
{
	Player * player = data;

	player_forward(player);
}


void on_next(gpointer data)
{
	Player * player = data;

	player_next(player);
}


void on_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* view */
/* playlist */
/* on_playlist_add */
void on_playlist_add(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_closex */
gboolean on_playlist_closex(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->pl_window);
	return TRUE;
}


/* on_playlist_load */
void on_playlist_load(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_remove */
void on_playlist_remove(gpointer data)
{
	/* FIXME implement */
}


/* on_playlist_save */
void on_playlist_save(gpointer data)
{
	/* FIXME implement */
}
