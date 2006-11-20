/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
	"Pierre 'khorben' Pronchery",
	NULL
};

/* FIXME */
static char const _license[] = "GPLv2";


/* callbacks */
/* window */
gboolean on_player_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	on_file_close(widget, data);
	return FALSE;
}


gboolean on_player_configure(GtkWidget * window, GdkEventConfigure * event,
		gpointer data)
{
	Player * player = data;
	int width;
	int height;

	width = max(1, event->width);
	height = max(1, event->height - player->view_iheight);
	XResizeWindow(GDK_DISPLAY(), player->view_window, width, height);
	return FALSE;
}


/* file menu */
void on_file_open(GtkWidget * widget, gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
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
	/* FIXME */
}


/* view menu */
void on_view_fullscreen(GtkWidget * widget, gpointer data)
{
	Player * player = data;
	static int full = 0;

	if(!full)
		gtk_window_fullscreen(GTK_WINDOW(player->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(player->window));
	full = !full;
}


/* help menu */
void on_help_about(GtkWidget * widget, gpointer data)
{
	Player * player = data;
	static GtkWidget * window = NULL;
	char const copyright[] = "Copyright (c) 2006 khorben";
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
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
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
