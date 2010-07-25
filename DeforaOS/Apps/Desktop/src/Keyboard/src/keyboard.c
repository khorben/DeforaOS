/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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



#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include "callbacks.h"
#include "keyboard.h"


/* Keyboard */
/* private */
/* types */
typedef struct _KeyboardKey
{
	unsigned int keysym;
	char const * label;
} KeyboardKey;

struct _Keyboard
{
	KeyboardKey * layout[3];

	GtkWidget * window;
};


/* variables */
static KeyboardKey _qwertyuiop[] =
{
	{ XK_Tab, "Tab" },
	{ XK_Q, "Q" },
	{ XK_W, "W" },
	{ XK_E, "E" },
	{ XK_R, "R" },
	{ XK_T, "T" },
	{ XK_Y, "Y" },
	{ XK_U, "U" },
	{ XK_I, "I" },
	{ XK_O, "O" },
	{ XK_P, "P" },
	{ XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, NULL }
};
static KeyboardKey _asdfghjkl[] =
{
	{ XK_Caps_Lock, "Caps" },
	{ XK_A, "A" },
	{ XK_S, "S" },
	{ XK_D, "D" },
	{ XK_F, "F" },
	{ XK_G, "G" },
	{ XK_H, "H" },
	{ XK_J, "J" },
	{ XK_K, "K" },
	{ XK_L, "L" },
	{ XK_Return, "Ret" },
	{ 0, NULL }
};
static KeyboardKey _zxcvbnm[] =
{
	{ XK_Shift_L, "\xe2\x87\xa7" },
	{ XK_Z, "Z" },
	{ XK_X, "X" },
	{ XK_C, "C" },
	{ XK_V, "V" },
	{ XK_B, "B" },
	{ XK_N, "N" },
	{ XK_M, "M" },
	{ XK_space, " " },
	{ XK_Shift_R, "\xe2\x87\xa7" },
	{ 0, NULL }
};


/* public */
/* functions */
/* keyboard_new */
Keyboard * keyboard_new(void)
{
	Keyboard * keyboard;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	size_t i;
	size_t j;
	GtkSizeGroup * group;

	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->layout[0] = _qwertyuiop;
	keyboard->layout[1] = _asdfghjkl;
	keyboard->layout[2] = _zxcvbnm;
	keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "delete-event",
			G_CALLBACK(on_keyboard_delete_event), keyboard);
	vbox = gtk_vbox_new(TRUE, 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
	for(i = 0; i < 3; i++)
	{
		hbox = gtk_hbox_new(TRUE, 4);
		for(j = 0; keyboard->layout[i][j].label != NULL; j++)
		{
			widget = gtk_button_new_with_label(
					keyboard->layout[i][j].label);
			g_object_set_data(G_OBJECT(widget), "keysym",
					&keyboard->layout[i][j].keysym);
			gtk_size_group_add_widget(group, widget);
			g_signal_connect_swapped(G_OBJECT(widget), "clicked",
					G_CALLBACK(on_keyboard_key_clicked),
					keyboard);
			gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE,
					0);
		}
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	}
	gtk_container_add(GTK_CONTAINER(keyboard->window), vbox);
	gtk_widget_show_all(keyboard->window);
	return keyboard;
}


/* keyboard_delete */
void keyboard_delete(Keyboard * keyboard)
{
	gtk_widget_destroy(keyboard->window);
	free(keyboard);
}


/* useful */
/* keyboard_show */
void keyboard_show(Keyboard * keyboard, gboolean show)
{
	if(show == TRUE)
		gtk_widget_show(keyboard->window);
	else
		gtk_widget_hide(keyboard->window);
}
