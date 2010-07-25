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
	GtkWidget * widget;
	unsigned int keysym;
	char const * label;
	char const * upper_label;
} KeyboardKey;

struct _Keyboard
{
	KeyboardKey * layout[4];

	GtkWidget * window;
};


/* variables */
static KeyboardKey _1234567890[] =
{
	{ NULL, XK_Escape, "Esc", NULL },
	{ NULL, XK_1, "1", NULL },
	{ NULL, XK_2, "2", NULL },
	{ NULL, XK_3, "3", NULL },
	{ NULL, XK_4, "4", NULL },
	{ NULL, XK_5, "5", NULL },
	{ NULL, XK_6, "6", NULL },
	{ NULL, XK_7, "7", NULL },
	{ NULL, XK_8, "8", NULL },
	{ NULL, XK_9, "9", NULL },
	{ NULL, XK_0, "0", NULL },
	{ NULL, XK_BackSpace, "\xe2\x8c\xab", NULL },
	{ NULL, 0, NULL, NULL }
};

static KeyboardKey _qwertyuiop[] =
{
	{ NULL, XK_Tab, "Tab", NULL },
	{ NULL, XK_Q, "q", "Q" },
	{ NULL, XK_W, "w", "W" },
	{ NULL, XK_E, "e", "E" },
	{ NULL, XK_R, "r", "R" },
	{ NULL, XK_T, "t", "T" },
	{ NULL, XK_Y, "y", "Y" },
	{ NULL, XK_U, "u", "U" },
	{ NULL, XK_I, "i", "I" },
	{ NULL, XK_O, "o", "O" },
	{ NULL, XK_P, "p", "P" },
	{ NULL, XK_Return, "Ret", NULL },
	{ NULL, 0, NULL, NULL }
};
static KeyboardKey _asdfghjkl[] =
{
	{ NULL, XK_Caps_Lock, "Caps", NULL },
	{ NULL, XK_A, "a", "A" },
	{ NULL, XK_S, "s", "S" },
	{ NULL, XK_D, "d", "D" },
	{ NULL, XK_F, "f", "F" },
	{ NULL, XK_G, "g", "G" },
	{ NULL, XK_H, "h", "H" },
	{ NULL, XK_J, "j", "J" },
	{ NULL, XK_K, "k", "K" },
	{ NULL, XK_L, "l", "L" },
	{ NULL, XK_colon, ":", NULL },
	{ NULL, XK_at, "@", NULL },
	{ NULL, 0, NULL, NULL }
};
static KeyboardKey _zxcvbnm[] =
{
	{ NULL, XK_Shift_L, "\xe2\x87\xa7", NULL },
	{ NULL, XK_Z, "z", "Z" },
	{ NULL, XK_X, "x", "X" },
	{ NULL, XK_C, "c", "C" },
	{ NULL, XK_V, "v", "V" },
	{ NULL, XK_B, "b", "B" },
	{ NULL, XK_N, "n", "N" },
	{ NULL, XK_M, "m", "M" },
	{ NULL, XK_space, " ", NULL },
	{ NULL, XK_period, ".", NULL },
	{ NULL, XK_minus, "-", NULL },
	{ NULL, XK_slash, "/", NULL },
	{ NULL, 0, NULL, NULL }
};


/* public */
/* functions */
/* keyboard_new */
Keyboard * keyboard_new(KeyboardPrefs * prefs)
{
	Keyboard * keyboard;
	GdkScreen * screen;
	GdkRectangle rect;
	gint height;
	PangoFontDescription * bold;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * label;
	size_t i;
	size_t j;

	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->layout[0] = _1234567890;
	keyboard->layout[1] = _qwertyuiop;
	keyboard->layout[2] = _asdfghjkl;
	keyboard->layout[3] = _zxcvbnm;
	/* window */
	keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window), FALSE);
	gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window), FALSE);
	screen = gdk_screen_get_default();
	if(prefs != NULL && prefs->monitor > 0
			&& prefs->monitor < gdk_screen_get_n_monitors(screen))
		gdk_screen_get_monitor_geometry(screen, prefs->monitor, &rect);
	else
		gdk_screen_get_monitor_geometry(screen, 0, &rect);
	height = rect.width / 8;
	gtk_widget_set_size_request(keyboard->window, rect.width, height);
	gtk_window_move(GTK_WINDOW(keyboard->window), rect.x,
			rect.y + rect.height - height);
	g_signal_connect_swapped(G_OBJECT(keyboard->window), "delete-event",
			G_CALLBACK(on_keyboard_delete_event), keyboard);
	/* layouts */
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	vbox = gtk_vbox_new(TRUE, 4);
	for(i = 0; i < 4; i++)
	{
		hbox = gtk_hbox_new(TRUE, 4);
		for(j = 0; keyboard->layout[i][j].label != NULL; j++)
		{
			if(keyboard->layout[i][j].keysym == XK_Shift_L)
				widget = gtk_toggle_button_new();
			else
				widget = gtk_button_new();
			label = gtk_label_new(keyboard->layout[i][j].label);
			keyboard->layout[i][j].widget = label;
			gtk_widget_modify_font(label, bold);
			gtk_container_add(GTK_CONTAINER(widget), label);
			g_object_set_data(G_OBJECT(widget), "keysym",
					&keyboard->layout[i][j].keysym);
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
	pango_font_description_free(bold);
	return keyboard;
}


/* keyboard_delete */
void keyboard_delete(Keyboard * keyboard)
{
	gtk_widget_destroy(keyboard->window);
	free(keyboard);
}


/* accessors */
/* keyboard_set_case */
void keyboard_set_case(Keyboard * keyboard, KeyboardCase kcase)
{
	size_t i;
	size_t j;
	GtkWidget * label;

	for(i = 0; i < 4; i++)
		for(j = 0; keyboard->layout[i][j].label != NULL; j++)
		{
			if(keyboard->layout[i][j].upper_label == NULL)
				continue;
			label = keyboard->layout[i][j].widget;
			gtk_label_set_text(GTK_LABEL(label), (kcase == KC_LOWER)
					? keyboard->layout[i][j].label
					: keyboard->layout[i][j].upper_label);
		}
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
