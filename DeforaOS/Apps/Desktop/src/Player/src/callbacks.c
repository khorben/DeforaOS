/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>";
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
#include "player.h"
#include "callbacks.h"
#include "../config.h"


/* macros */
#define max(a, b) (((a) > (b)) ? (a) : (b))


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* callbacks */
/* window */
gboolean on_player_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	on_file_close(widget, data);
	return FALSE;
}


/* file menu */
void on_file_open(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
}


void on_file_properties(GtkWidget * widget, gpointer data)
{
	Player * player = data;
	GtkWidget * window;
	char buf[256];

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_widget_destroy), NULL);
	snprintf(buf, sizeof(buf), "%s%s", "Properties of ", player->filename);
	gtk_window_set_title(GTK_WINDOW(window), buf);
	/* FIXME implement */
	gtk_widget_show_all(window);
}


void on_file_close(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_stop(player);
	gtk_main_quit();
}


/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data)
{
	Player * player = data;
	static GtkWidget * window = NULL;


	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Preferences");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				player->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				gtk_widget_hide), NULL); /* FIXME cancel */
	/* FIXME implement */
	gtk_widget_show_all(window);
}


/* view menu */
void on_view_fullscreen(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* help menu */
void on_help_about(GtkWidget * widget, gpointer data)
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
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
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
	/* FIXME implement */
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */


/* toolbar */
void on_previous(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_previous(player);
}


void on_rewind(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_rewind(player);
}


void on_play(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_play(player);
}


void on_pause(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_pause(player);
}


void on_stop(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_stop(player);
}


void on_forward(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_forward(player);
}


void on_next(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_next(player);
}


void on_fullscreen(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}
