/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
 * - choose a correct size for the window
 * - move and resize when the root window changes
 * - track if child process ever dies
 * - write own keyboard implementation (dlopen keyboard's binary) */



#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
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
	GPid pid;
	GtkWidget * window;
	/* preferences */
	GtkWidget * pr_box;
	GtkWidget * pr_command;
} Keyboard;


/* prototypes */
static GtkWidget * _keyboard_init(PanelApplet * applet);
static void _keyboard_destroy(PanelApplet * applet);
static GtkWidget * _keyboard_settings(PanelApplet * applet, gboolean apply,
		gboolean reset);

/* callbacks */
static void _on_keyboard_toggled(GtkWidget * widget, gpointer data);


/* constants */
#define PANEL_KEYBOARD_COMMAND_DEFAULT "keyboard -x"


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	"Keyboard",
	"input-keyboard",
	_keyboard_init,
	_keyboard_destroy,
	_keyboard_settings,
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
	applet->priv = keyboard;
	keyboard->helper = applet->helper;
	keyboard->pid = -1;
	keyboard->window = NULL;
	keyboard->pr_box = NULL;
	ret = gtk_toggle_button_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Show keyboard"));
#endif
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
	char * argv[] = { "sh", "-c", PANEL_KEYBOARD_COMMAND_DEFAULT, NULL };
	char const * p;
	char * q = NULL;
	gboolean res;
	gint out = -1;
	GError * error = NULL;
	char buf[32];
	ssize_t size;
	unsigned long xid;
	GtkWidget * socket;

	if(keyboard->window != NULL)
		return FALSE;
	if((p = keyboard->helper->config_get(keyboard->helper->panel,
					"keyboard", "command")) != NULL
			&& (q = strdup(p)) != NULL)
		argv[2] = q;
	res = g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
			NULL, NULL, &keyboard->pid, NULL, &out, NULL, &error);
	free(q);
	if(res != TRUE)
		return keyboard->helper->error(keyboard->helper->panel,
				argv[0], FALSE);
	if((size = read(out, buf, sizeof(buf) - 1)) <= 0) /* XXX may block */
		return keyboard->helper->error(keyboard->helper->panel,
				"read", FALSE); /* XXX not very explicit... */
	buf[size] = '\0';
	if(sscanf(buf, "%lu", &xid) != 1)
		return FALSE; /* XXX warn the user */
	keyboard->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
#endif
	gtk_window_set_type_hint(GTK_WINDOW(keyboard->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	socket = gtk_socket_new();
	gtk_widget_set_size_request(socket, 480, 150);
	gtk_container_add(GTK_CONTAINER(keyboard->window), socket);
	gtk_socket_add_id(GTK_SOCKET(socket), xid);
	gtk_widget_show(socket);
	return FALSE;
}


/* keyboard_destroy */
static void _keyboard_destroy(PanelApplet * applet)
{
	Keyboard * keyboard = applet->priv;

	if(keyboard->pid > 0)
		g_spawn_close_pid(keyboard->pid); /* XXX may be dead already */
	free(keyboard);
}


/* keyboard_settings */
static GtkWidget * _keyboard_settings(PanelApplet * applet, gboolean apply,
		gboolean reset)
{
	Keyboard * keyboard = applet->priv;
	GtkWidget * hbox;
	GtkWidget * widget;
	char const * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %s, %s)\n", __func__, (void*)applet,
			apply ? "TRUE" : "FALSE", reset ? "TRUE" : "FALSE");
#endif
	if(keyboard->pr_box == NULL)
	{
		keyboard->pr_box = gtk_vbox_new(FALSE, 4);
		hbox = gtk_hbox_new(FALSE, 4);
		widget = gtk_label_new("Command:");
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		keyboard->pr_command = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), keyboard->pr_command, TRUE,
				TRUE, 0);
		gtk_box_pack_start(GTK_BOX(keyboard->pr_box), hbox, FALSE, TRUE,
				0);
		gtk_widget_show_all(keyboard->pr_box);
		reset = TRUE;
	}
	if(reset == TRUE)
	{
		if((p = applet->helper->config_get(applet->helper->panel,
						"keyboard", "command")) == NULL)
			p = PANEL_KEYBOARD_COMMAND_DEFAULT;
		gtk_entry_set_text(GTK_ENTRY(keyboard->pr_command), p);
	}
	if(apply == TRUE)
	{
		p = gtk_entry_get_text(GTK_ENTRY(keyboard->pr_command));
		applet->helper->config_set(applet->helper->panel, "keyboard",
				"command", p);
	}
	return keyboard->pr_box;
}


/* callbacks */
/* on_keyboard_toggled */
static void _on_keyboard_toggled(GtkWidget * widget, gpointer data)
{
	Keyboard * keyboard = data;
	gint x = 0;
	gint y = 0;
	gboolean push_in;

	if(keyboard->window == NULL)
		_init_idle(keyboard);
	if(keyboard->window == NULL)
		return;
	keyboard->helper->position_menu(keyboard->helper->panel,
			(GtkMenu*)keyboard->window, &x, &y, &push_in);
	gtk_window_move(GTK_WINDOW(keyboard->window), x, y);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		gtk_widget_show(keyboard->window);
	else
		gtk_widget_hide(keyboard->window);
}
