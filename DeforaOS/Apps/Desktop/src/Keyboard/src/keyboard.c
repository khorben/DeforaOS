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
#include <stdio.h>
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
struct _Keyboard
{
	KeyboardKey * layout[4];

	PangoFontDescription * bold;
	GtkWidget * window;
	GdkRectangle geometry;
};


/* variables */
static KeyboardKey _1234567890[] =
{
	{ NULL, NULL, XK_Escape, "Esc", NULL },
	{ NULL, NULL, XK_1, "1", "!" },
	{ NULL, NULL, XK_2, "2", "@" },
	{ NULL, NULL, XK_3, "3", "#" },
	{ NULL, NULL, XK_4, "4", "$" },
	{ NULL, NULL, XK_5, "5", "%" },
	{ NULL, NULL, XK_6, "6", "^" },
	{ NULL, NULL, XK_7, "7", "&" },
	{ NULL, NULL, XK_8, "8", "*" },
	{ NULL, NULL, XK_9, "9", "(" },
	{ NULL, NULL, XK_0, "0", ")" },
	{ NULL, NULL, XK_BackSpace, "\xe2\x8c\xab", NULL },
	{ NULL, NULL, 0, NULL, NULL }
};

static KeyboardKey _qwertyuiop[] =
{
	{ NULL, NULL, XK_Tab, "\xe2\x86\x92|", NULL },
	{ NULL, NULL, XK_Q, "q", "Q" },
	{ NULL, NULL, XK_W, "w", "W" },
	{ NULL, NULL, XK_E, "e", "E" },
	{ NULL, NULL, XK_R, "r", "R" },
	{ NULL, NULL, XK_T, "t", "T" },
	{ NULL, NULL, XK_Y, "y", "Y" },
	{ NULL, NULL, XK_U, "u", "U" },
	{ NULL, NULL, XK_I, "i", "I" },
	{ NULL, NULL, XK_O, "o", "O" },
	{ NULL, NULL, XK_P, "p", "P" },
	{ NULL, NULL, XK_Return, "Ret", NULL },
	{ NULL, NULL, 0, NULL, NULL }
};
static KeyboardKey _asdfghjkl[] =
{
	{ NULL, NULL, XK_Caps_Lock, "Caps", NULL },
	{ NULL, NULL, XK_A, "a", "A" },
	{ NULL, NULL, XK_S, "s", "S" },
	{ NULL, NULL, XK_D, "d", "D" },
	{ NULL, NULL, XK_F, "f", "F" },
	{ NULL, NULL, XK_G, "g", "G" },
	{ NULL, NULL, XK_H, "h", "H" },
	{ NULL, NULL, XK_J, "j", "J" },
	{ NULL, NULL, XK_K, "k", "K" },
	{ NULL, NULL, XK_L, "l", "L" },
	{ NULL, NULL, XK_semicolon, ";", ":" },
	{ NULL, NULL, XK_apostrophe, "'", "\"" },
	{ NULL, NULL, 0, NULL, NULL }
};
static KeyboardKey _zxcvbnm[] =
{
	{ NULL, NULL, XK_Shift_L, "\xe2\x87\xa7", NULL },
	{ NULL, NULL, XK_Z, "z", "Z" },
	{ NULL, NULL, XK_X, "x", "X" },
	{ NULL, NULL, XK_C, "c", "C" },
	{ NULL, NULL, XK_V, "v", "V" },
	{ NULL, NULL, XK_B, "b", "B" },
	{ NULL, NULL, XK_N, "n", "N" },
	{ NULL, NULL, XK_M, "m", "M" },
	{ NULL, NULL, XK_space, " ", NULL },
	{ NULL, NULL, XK_comma, ",", "<" },
	{ NULL, NULL, XK_period, ".", ">" },
	{ NULL, NULL, XK_slash, "/", "?" },
	{ NULL, NULL, 0, NULL, NULL }
};


/* public */
/* functions */
/* keyboard_new */
Keyboard * keyboard_new(KeyboardPrefs * prefs)
{
	Keyboard * keyboard;
	KeyboardKey * kk;
	GdkScreen * screen;
	gint height;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * label;
	size_t i;
	size_t j;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->layout[0] = _1234567890;
	keyboard->layout[1] = _qwertyuiop;
	keyboard->layout[2] = _asdfghjkl;
	keyboard->layout[3] = _zxcvbnm;
	screen = gdk_screen_get_default();
	if(prefs != NULL && prefs->monitor > 0
			&& prefs->monitor < gdk_screen_get_n_monitors(screen))
		gdk_screen_get_monitor_geometry(screen, prefs->monitor,
				&keyboard->geometry);
	else
		gdk_screen_get_monitor_geometry(screen, 0, &keyboard->geometry);
	height = (keyboard->geometry.width / 12) * 3;
	/* window */
	if(prefs->embedded != 0)
	{
		keyboard->window = gtk_plug_new(0);
		g_signal_connect_swapped(G_OBJECT(keyboard->window), "embedded",
				G_CALLBACK(on_keyboard_embedded), keyboard);
	}
	else
	{
		keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window),
				FALSE);
		gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window),
				FALSE);
		gtk_window_move(GTK_WINDOW(keyboard->window),
				keyboard->geometry.x,
				keyboard->geometry.y + keyboard->geometry.height
				- height);
		g_signal_connect_swapped(G_OBJECT(keyboard->window),
				"delete-event",
				G_CALLBACK(on_keyboard_delete_event), keyboard);
	}
	gtk_widget_set_size_request(keyboard->window, keyboard->geometry.width,
			height);
	/* layouts */
	if(prefs->font != NULL)
		keyboard->bold = pango_font_description_from_string(
				prefs->font);
	else
		keyboard->bold = pango_font_description_new();
	pango_font_description_set_weight(keyboard->bold, PANGO_WEIGHT_BOLD);
	vbox = gtk_vbox_new(TRUE, 4);
	for(i = 0; i < 4; i++)
	{
		hbox = gtk_hbox_new(TRUE, 4);
		for(j = 0; keyboard->layout[i][j].label != NULL; j++)
		{
			kk = &keyboard->layout[i][j];
			if(kk->keysym == XK_Shift_L || kk->keysym == XK_Shift_R)
				widget = gtk_toggle_button_new();
			else
				widget = gtk_button_new();
			label = gtk_label_new(kk->label);
			kk->widget = label;
			gtk_widget_modify_font(label, keyboard->bold);
			gtk_container_add(GTK_CONTAINER(widget), label);
			g_object_set_data(G_OBJECT(widget), "key", kk);
			g_signal_connect(G_OBJECT(widget), "button-press-event",
					G_CALLBACK(on_keyboard_key_pressed),
					keyboard);
			g_signal_connect(G_OBJECT(widget),
					"button-release-event", G_CALLBACK(
						on_keyboard_key_released),
					keyboard);
			g_signal_connect_swapped(G_OBJECT(widget), "clicked",
					G_CALLBACK(on_keyboard_key_clicked),
					keyboard);
			gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE,
					0);
		}
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	}
	gtk_container_add(GTK_CONTAINER(keyboard->window), vbox);
	gtk_widget_show_all(vbox);
	if(prefs->embedded == 0)
		gtk_widget_show(keyboard->window);
	else
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() id=%u\n", __func__,
				gtk_plug_get_id(GTK_PLUG(keyboard->window)));
#endif
		printf("%u\n", gtk_plug_get_id(GTK_PLUG(keyboard->window)));
		fclose(stdout);
	}
	return keyboard;
}


/* keyboard_delete */
void keyboard_delete(Keyboard * keyboard)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_widget_destroy(keyboard->window);
	pango_font_description_free(keyboard->bold);
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
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, show ? "TRUE" : "FALSE");
#endif
	if(show == TRUE)
		gtk_widget_show(keyboard->window);
	else
		gtk_widget_hide(keyboard->window);
}


/* keyboard_key_show */
void keyboard_key_show(Keyboard * keyboard, KeyboardKey * key, gboolean show)
{
	GtkWidget * widget;
	unsigned int bwidth = keyboard->geometry.width / 12;
	unsigned int bheight = (bwidth / 4) * 3;
	size_t i;
	size_t j;

	if(show == FALSE)
	{
		if(key->popup != NULL)
			gtk_widget_destroy(key->popup);
		key->popup = NULL;
		return;
	}
	if(key->popup == NULL)
	{
		key->popup = gtk_window_new(GTK_WINDOW_POPUP);
		widget = gtk_button_new_with_label(key->upper_label != NULL?
				key->upper_label : key->label);
		gtk_button_set_alignment(GTK_BUTTON(widget), 0.5, 0.1);
		gtk_widget_modify_font(gtk_bin_get_child(GTK_BIN(widget)),
				keyboard->bold);
		gtk_widget_set_size_request(key->popup, bwidth, bheight * 2);
		gtk_container_add(GTK_CONTAINER(key->popup), widget);
	}
	for(i = 0; i < 4; i++)
	{
		for(j = 0; keyboard->layout[i][j].label != NULL; j++)
			if(&keyboard->layout[i][j] == key)
				break;
		if(keyboard->layout[i][j].label != NULL)
			break;
	}
	if(i == 4 && j == 12) /* XXX hard-coded */
	{
		i = 0;
		j = 5;
	}
	gtk_window_move(GTK_WINDOW(key->popup), bwidth * j,
			keyboard->geometry.height - (bheight * 6)
			+ (bheight * i));
	gtk_widget_show_all(key->popup);
}
