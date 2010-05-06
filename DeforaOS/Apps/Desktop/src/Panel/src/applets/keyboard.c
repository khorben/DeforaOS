/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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
/* TODO:
 * - choose a correct size and position for the window
 * - track if xkbd's process ever dies
 * - write own keyboard implementation */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Keyboard */
/* private */
/* types */
typedef struct _Keyboard
{
	PanelAppletHelper * helper;
	GtkWidget * window;
} Keyboard;


/* prototypes */
static GtkWidget * _keyboard_init(PanelApplet * applet);

/* callbacks */
static void _on_keyboard_toggled(GtkWidget * widget, gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_keyboard_init,
	NULL,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* keyboard_init */
static gboolean _init_idle(gpointer data);

static GtkWidget * _keyboard_init(PanelApplet * applet)
{
	GtkWidget * ret;
	GtkWidget * image;
	Keyboard * keyboard;

	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->helper = applet->helper;
	keyboard->window = NULL;
	ret = gtk_toggle_button_new();
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(ret), "toggled", G_CALLBACK(
				_on_keyboard_toggled), keyboard);
	image = gtk_image_new_from_icon_name("input-keyboard",
			applet->helper->icon_size);
	gtk_container_add(GTK_CONTAINER(ret), image);
	gtk_widget_show_all(ret);
	g_idle_add(_init_idle, keyboard);
	return ret;
}

static gboolean _init_idle(gpointer data)
{
	Keyboard * keyboard = data;
	char * argv[] = { "xkbd", "-xid", NULL };
	gint out = -1;
	GError * error = NULL;
	char buf[32];
	ssize_t size;
	unsigned long xid;
	GtkWidget * socket;

	if(keyboard->window != NULL)
		return FALSE;
	if(g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
			NULL, NULL, NULL, NULL, &out, NULL, &error) != TRUE)
		return keyboard->helper->error(keyboard->helper->priv,
				argv[0], FALSE);
	if((size = read(out, buf, sizeof(buf) - 1)) <= 0) /* XXX may block */
		return keyboard->helper->error(keyboard->helper->priv,
				"read", FALSE); /* XXX not very explicit... */
	buf[size] = '\0';
	if(sscanf(buf, "%lu", &xid) != 1)
		return FALSE; /* XXX warn the user */
	keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_default_size(GTK_WINDOW(keyboard->window), 480, 120);
	socket = gtk_socket_new();
	gtk_container_add(GTK_CONTAINER(keyboard->window), socket);
	gtk_socket_add_id(GTK_SOCKET(socket), xid);
	gtk_widget_show(socket);
	return FALSE;
}


/* callbacks */
/* on_keyboard_toggled */
static void _on_keyboard_toggled(GtkWidget * widget, gpointer data)
{
	Keyboard * keyboard = data;

	if(keyboard->window == NULL)
		_init_idle(keyboard);
	if(keyboard->window == NULL)
		return;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		gtk_widget_show(keyboard->window);
	else
		gtk_widget_hide(keyboard->window);
}
