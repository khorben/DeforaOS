/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include "callbacks.h"
#include "layout.h"
#include "keyboard.h"


/* Keyboard */
/* private */
/* types */
struct _Keyboard
{
	KeyboardLayout ** layouts;
	size_t layouts_cnt;

	PangoFontDescription * font;
	GtkWidget * window;
	GdkRectangle geometry;
	int width;
	int height;
	int x;
	int y;
};

typedef struct _KeyboardKeys
{
	unsigned int row;
	unsigned int width;
	unsigned int modifier;
	unsigned int keysym;
	char const * label;
} KeyboardKeys;

typedef enum _KeyboardLayoutSection
{
	KLS_LETTERS = 0,
	KLS_KEYPAD,
	KLS_SPECIAL
} KeyboardLayoutSection;
#define KLS_LAST KLS_SPECIAL
#define KLS_COUNT (KLS_LAST + 1)


/* variables */
static const KeyboardKeys _keyboard_layout_letters[] =
{
	{ 0, 2, 0, XK_q, "q" },
	{ 0, 0, XK_Shift_L, XK_Q, "Q" },
	{ 0, 2, 0, XK_w, "w" },
	{ 0, 0, XK_Shift_L, XK_W, "W" },
	{ 0, 2, 0, XK_e, "e" },
	{ 0, 0, XK_Shift_L, XK_E, "E" },
	{ 0, 2, 0, XK_r, "r" },
	{ 0, 0, XK_Shift_L, XK_R, "R" },
	{ 0, 2, 0, XK_t, "t" },
	{ 0, 0, XK_Shift_L, XK_T, "T" },
	{ 0, 2, 0, XK_y, "y" },
	{ 0, 0, XK_Shift_L, XK_Y, "Y" },
	{ 0, 2, 0, XK_u, "u" },
	{ 0, 0, XK_Shift_L, XK_U, "U" },
	{ 0, 2, 0, XK_i, "i" },
	{ 0, 0, XK_Shift_L, XK_I, "I" },
	{ 0, 2, 0, XK_o, "o" },
	{ 0, 0, XK_Shift_L, XK_O, "O" },
	{ 0, 2, 0, XK_p, "p" },
	{ 0, 0, XK_Shift_L, XK_P, "P" },
	{ 1, 1, 0, 0, NULL },
	{ 1, 2, 0, XK_a, "a" },
	{ 1, 0, XK_Shift_L, XK_A, "A" },
	{ 1, 2, 0, XK_s, "s" },
	{ 1, 0, XK_Shift_L, XK_S, "S" },
	{ 1, 2, 0, XK_d, "d" },
	{ 1, 0, XK_Shift_L, XK_D, "D" },
	{ 1, 2, 0, XK_f, "f" },
	{ 1, 0, XK_Shift_L, XK_F, "F" },
	{ 1, 2, 0, XK_g, "g" },
	{ 1, 0, XK_Shift_L, XK_G, "G" },
	{ 1, 2, 0, XK_h, "h" },
	{ 1, 0, XK_Shift_L, XK_H, "H" },
	{ 1, 2, 0, XK_j, "j" },
	{ 1, 0, XK_Shift_L, XK_J, "J" },
	{ 1, 2, 0, XK_k, "k" },
	{ 1, 0, XK_Shift_L, XK_K, "K" },
	{ 1, 2, 0, XK_l, "l" },
	{ 1, 0, XK_Shift_L, XK_L, "L" },
	{ 2, 2, 0, XK_Shift_L, "\xe2\x87\xa7" },
	{ 2, 2, 0, XK_z, "z" },
	{ 2, 0, XK_Shift_L, XK_Z, "Z" },
	{ 2, 2, 0, XK_x, "x" },
	{ 2, 0, XK_Shift_L, XK_X, "X" },
	{ 2, 2, 0, XK_c, "c" },
	{ 2, 0, XK_Shift_L, XK_C, "C" },
	{ 2, 2, 0, XK_v, "v" },
	{ 2, 0, XK_Shift_L, XK_V, "V" },
	{ 2, 2, 0, XK_b, "b" },
	{ 2, 0, XK_Shift_L, XK_B, "B" },
	{ 2, 2, 0, XK_n, "n" },
	{ 2, 0, XK_Shift_L, XK_N, "N" },
	{ 2, 2, 0, XK_m, "m" },
	{ 2, 0, XK_Shift_L, XK_M, "M" },
	{ 2, 2, 0, XK_comma, "," },
	{ 2, 0, XK_Shift_L, XK_comma, "<" },
	{ 2, 2, 0, XK_period, "." },
	{ 2, 0, XK_Shift_L, XK_period, ">" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 3, 0, XK_Control_L, "Ctrl" },
	{ 3, 3, 0, XK_Alt_L, "Alt" },
	{ 3, 5, 0, XK_space, " " },
	{ 3, 3, 0, XK_Return, "\xe2\x86\xb2" },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static const KeyboardKeys _keyboard_layout_keypad[] =
{
	{ 0, 3, 0, XK_Num_Lock, "Num" },
	{ 0, 3, 0, XK_KP_Home, "\xe2\x86\x96" },
	{ 0, 0, XK_Num_Lock, XK_7, "7" },
	{ 0, 3, 0, XK_KP_Up, "\xe2\x86\x91" },
	{ 0, 0, XK_Num_Lock, XK_8, "8" },
	{ 0, 3, 0, XK_KP_Page_Up, "\xe2\x87\x9e" },
	{ 0, 0, XK_Num_Lock, XK_9, "9" },
	{ 0, 3, 0, XK_KP_Subtract, "-" },
	{ 1, 3, 0, XK_KP_Divide, "/" },
	{ 1, 3, 0, XK_KP_Left, "\xe2\x86\x90" },
	{ 1, 0, XK_Num_Lock, XK_4, "4" },
	{ 1, 3, 0, XK_5, "5" },
	{ 1, 3, 0, XK_KP_Right, "\xe2\x86\x92" },
	{ 1, 0, XK_Num_Lock, XK_6, "6" },
	{ 1, 3, 0, XK_KP_Add, "+" },
	{ 2, 3, 0, XK_KP_Multiply, "*" },
	{ 2, 3, 0, XK_KP_End, "\xe2\x86\x99" },
	{ 2, 0, XK_Num_Lock, XK_1, "1" },
	{ 2, 3, 0, XK_KP_Down, "\xe2\x86\x93" },
	{ 2, 0, XK_Num_Lock, XK_2, "2" },
	{ 2, 3, 0, XK_KP_Page_Down, "\xe2\x87\x9f" },
	{ 2, 0, XK_Num_Lock, XK_3, "3" },
	{ 2, 3, 0, XK_KP_Enter, "\xe2\x86\xb2" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 6, 0, XK_KP_Insert, "Ins" },
	{ 3, 0, XK_Num_Lock, XK_0, "0" },
	{ 3, 3, 0, XK_KP_Delete, "Del" },
	{ 3, 0, XK_Num_Lock, XK_KP_Decimal, "." },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static const KeyboardKeys _keyboard_layout_special[] =
{
	{ 0, 3, 0, XK_Escape, "Esc" },
	{ 0, 2, 0, XK_F1, "F1" },
	{ 0, 2, 0, XK_F2, "F2" },
	{ 0, 2, 0, XK_F3, "F3" },
	{ 0, 2, 0, XK_F4, "F4" },
	{ 0, 1, 0, 0, NULL },
	{ 0, 2, 0, XK_F5, "F5" },
	{ 0, 2, 0, XK_F6, "F6" },
	{ 0, 2, 0, XK_F7, "F7" },
	{ 0, 2, 0, XK_F8, "F8" },
	{ 1, 2, 0, XK_1, "1" },
	{ 1, 0, XK_Shift_L, XK_1, "!" },
	{ 1, 2, 0, XK_2, "2" },
	{ 1, 0, XK_Shift_L, XK_2, "@" },
	{ 1, 2, 0, XK_3, "3" },
	{ 1, 0, XK_Shift_L, XK_3, "#" },
	{ 1, 2, 0, XK_4, "4" },
	{ 1, 0, XK_Shift_L, XK_4, "$" },
	{ 1, 2, 0, XK_5, "5" },
	{ 1, 0, XK_Shift_L, XK_5, "%" },
	{ 1, 2, 0, XK_6, "6" },
	{ 1, 0, XK_Shift_L, XK_6, "^" },
	{ 1, 2, 0, XK_7, "7" },
	{ 1, 0, XK_Shift_L, XK_7, "&" },
	{ 1, 2, 0, XK_8, "8" },
	{ 1, 0, XK_Shift_L, XK_8, "*" },
	{ 1, 2, 0, XK_9, "9" },
	{ 1, 0, XK_Shift_L, XK_9, "(" },
	{ 1, 2, 0, XK_0, "0" },
	{ 1, 0, XK_Shift_L, XK_0, ")" },
	{ 2, 3, 0, XK_Tab, "\xe2\x86\xb9" },
	{ 2, 2, 0, XK_grave, "`" },
	{ 2, 0, XK_Shift_L, XK_grave, "~" },
	{ 2, 2, 0, XK_minus, "-" },
	{ 2, 0, XK_Shift_L, XK_minus, "_" },
	{ 2, 2, 0, XK_equal, "=" },
	{ 2, 0, XK_Shift_L, XK_equal, "+" },
	{ 2, 2, 0, XK_backslash, "\\" },
	{ 2, 0, XK_Shift_L, XK_backslash, "|" },
	{ 2, 2, 0, XK_bracketleft, "[" },
	{ 2, 0, XK_Shift_L, XK_bracketleft, "{" },
	{ 2, 2, 0, XK_bracketright, "]" },
	{ 2, 0, XK_Shift_L, XK_bracketright, "}" },
	{ 2, 2, 0, XK_semicolon, ";" },
	{ 2, 0, XK_Shift_L, XK_semicolon, ":" },
	{ 2, 2, 0, XK_apostrophe, "'" },
	{ 2, 0, XK_Shift_L, XK_apostrophe, "\"" },
	{ 3, 3, 0, 0, NULL },
	{ 3, 2, 0, XK_Shift_L, "\xe2\x87\xa7" },
	{ 3, 3, 0, XK_space, " " },
	{ 3, 2, 0, XK_comma, "," },
	{ 3, 0, XK_Shift_L, XK_comma, "<" },
	{ 3, 2, 0, XK_period, "." },
	{ 2, 0, XK_Shift_L, XK_period, ">" },
	{ 3, 2, 0, XK_slash, "/" },
	{ 3, 0, XK_Shift_L, XK_slash, "?" },
	{ 3, 3, 0, XK_Return, "\xe2\x86\xb2" },
	{ 3, 3, 0, XK_BackSpace, "\xe2\x8c\xab" },
	{ 0, 0, 0, 0, NULL }
};

static const KeyboardKeys * _keyboard_layout[KLS_COUNT] =
{
	_keyboard_layout_letters,
	_keyboard_layout_keypad,
	_keyboard_layout_special
};


/* prototypes */
static GtkWidget * _keyboard_add_layout(Keyboard * keyboard,
		KeyboardKeys const * keys, unsigned int n);


/* public */
/* functions */
/* keyboard_new */
Keyboard * keyboard_new(KeyboardPrefs * prefs)
{
	Keyboard * keyboard;
	GdkScreen * screen;
	GtkWidget * vbox;
	GtkWidget * widget;
	PangoFontDescription * bold;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((keyboard = malloc(sizeof(*keyboard))) == NULL)
		return NULL;
	keyboard->layouts = NULL;
	keyboard->layouts_cnt = 0;
	screen = gdk_screen_get_default();
	if(prefs != NULL && prefs->monitor > 0
			&& prefs->monitor < gdk_screen_get_n_monitors(screen))
		gdk_screen_get_monitor_geometry(screen, prefs->monitor,
				&keyboard->geometry);
	else
		gdk_screen_get_monitor_geometry(screen, 0, &keyboard->geometry);
	/* window */
	if(prefs->embedded != 0)
	{
		keyboard->window = gtk_plug_new(0);
		keyboard->width = 0;
		keyboard->height = 0;
		keyboard->x = 0;
		keyboard->y = 0;
		g_signal_connect_swapped(G_OBJECT(keyboard->window), "embedded",
				G_CALLBACK(on_keyboard_embedded), keyboard);
	}
	else
	{
		keyboard->window = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_container_set_border_width(GTK_CONTAINER(keyboard->window),
				4);
		gtk_window_set_accept_focus(GTK_WINDOW(keyboard->window),
				FALSE);
		gtk_window_set_focus_on_map(GTK_WINDOW(keyboard->window),
				FALSE);
		keyboard->width = keyboard->geometry.width;
		keyboard->height = (keyboard->geometry.width / 11) * 3;
		keyboard->x = keyboard->geometry.x;
		keyboard->y = keyboard->geometry.y + keyboard->geometry.height
			- keyboard->height;
		gtk_window_move(GTK_WINDOW(keyboard->window), keyboard->x,
				keyboard->y);
		gtk_widget_set_size_request(keyboard->window, keyboard->width,
				keyboard->height);
		g_signal_connect_swapped(G_OBJECT(keyboard->window),
				"delete-event",
				G_CALLBACK(on_keyboard_delete_event), keyboard);
	}
	/* fonts */
	if(prefs->font != NULL)
		keyboard->font = pango_font_description_from_string(
				prefs->font);
	else
		keyboard->font = pango_font_description_new();
	pango_font_description_set_weight(keyboard->font, PANGO_WEIGHT_BOLD);
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	/* layouts */
	vbox = gtk_vbox_new(TRUE, 4);
	gtk_widget_show(vbox);
	if((widget = _keyboard_add_layout(keyboard,
					_keyboard_layout[KLS_LETTERS],
					KLS_LETTERS)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	if((widget = _keyboard_add_layout(keyboard,
					_keyboard_layout[KLS_KEYPAD],
					KLS_KEYPAD)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	if((widget = _keyboard_add_layout(keyboard,
					_keyboard_layout[KLS_SPECIAL],
					KLS_SPECIAL)) != NULL)
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(keyboard->window), vbox);
	if(prefs->embedded == 0)
		gtk_widget_show(keyboard->window);
	else
	{
		printf("%u\n", gtk_plug_get_id(GTK_PLUG(keyboard->window)));
		fclose(stdout);
	}
	keyboard_set_layout(keyboard, 0);
	pango_font_description_free(bold);
	return keyboard;
}


/* keyboard_delete */
void keyboard_delete(Keyboard * keyboard)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_widget_destroy(keyboard->window);
	pango_font_description_free(keyboard->font);
	free(keyboard);
}


/* accessors */
/* keyboard_set_modifier */
void keyboard_set_modifier(Keyboard * keyboard, unsigned int modifier)
{
	size_t i;

	for(i = 0; i < keyboard->layouts_cnt; i++)
		keyboard_layout_apply_modifier(keyboard->layouts[i], modifier);
}


/* keyboard_set_layout */
void keyboard_set_layout(Keyboard * keyboard, unsigned int which)
{
	size_t i;
	GtkWidget * widget;

	for(i = 0; i < keyboard->layouts_cnt; i++)
		if((widget = keyboard_layout_get_widget(keyboard->layouts[i]))
				== NULL)
			continue;
		else if(i == which)
			gtk_widget_show_all(widget);
		else
			gtk_widget_hide(widget);
}


/* useful */
/* keyboard_show */
void keyboard_show(Keyboard * keyboard, gboolean show)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, show ? "TRUE" : "FALSE");
#endif
	if(show == TRUE)
	{
		gtk_window_get_size(GTK_WINDOW(keyboard->window),
				&keyboard->width, &keyboard->height);
		gtk_widget_show(keyboard->window);
		gtk_window_get_position(GTK_WINDOW(keyboard->window),
				&keyboard->x, &keyboard->y);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() width=%d, height=%d\n", __func__,
				keyboard->width, keyboard->height);
		fprintf(stderr, "DEBUG: %s() x=%d, y=%d\n", __func__,
				keyboard->x, keyboard->y);
#endif
	}
	else
		gtk_widget_hide(keyboard->window);
}


/* private */
/* keyboard_add_layout */
static void _layout_changed(GtkWidget * widget, gpointer data);

static GtkWidget * _keyboard_add_layout(Keyboard * keyboard,
		KeyboardKeys const * keys, unsigned int n)
{
	KeyboardLayout ** p;
	KeyboardLayout * layout;
	size_t i;
	KeyboardKey * key;
	GtkWidget * widget;
	char const * labels[] = { "Abc", "123", ",./", NULL };
	unsigned long l;

	if((p = realloc(keyboard->layouts, sizeof(*p) * (keyboard->layouts_cnt
						+ 1))) == NULL)
		return NULL;
	keyboard->layouts = p;
	if((layout = keyboard_layout_new()) == NULL)
		return NULL;
	keyboard->layouts[keyboard->layouts_cnt++] = layout;
	for(i = 0; keys[i].width != 0; i++)
	{
		key = keyboard_layout_add(layout, keys[i].row, keys[i].width,
				keys[i].keysym, keys[i].label);
		if(key == NULL)
			continue;
		widget = keyboard_key_get_label_widget(key);
		gtk_widget_modify_font(widget, keyboard->font);
		for(; keys[i + 1].width == 0 && keys[i + 1].modifier != 0; i++)
			keyboard_key_set_modifier(key, keys[i + 1].modifier,
					keys[i + 1].keysym, keys[i + 1].label);
	}
	widget = gtk_combo_box_new_text();
	gtk_widget_modify_font(gtk_bin_get_child(GTK_BIN(widget)),
			keyboard->font);
	for(l = 0; labels[l] != NULL; l++)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(widget), labels[l]);
		if(l == n)
		{
			g_object_set_data(G_OBJECT(widget), "layout",
					(void *)l);
			gtk_combo_box_set_active(GTK_COMBO_BOX(widget), l);
		}
	}
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(
				_layout_changed), keyboard);
	keyboard_layout_add_widget(layout, 3, 0, 3, widget);
	return keyboard_layout_get_widget(layout);
}

static void _layout_changed(GtkWidget * widget, gpointer data)
{
	Keyboard * keyboard = data;
	unsigned long d;
	int n;

	d = (unsigned long)g_object_get_data(G_OBJECT(widget), "layout");
	n = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), d);
	switch(n)
	{
		case 0:
			on_keyboard_set_layout_letters(keyboard);
			break;
		case 1:
			on_keyboard_set_layout_keypad(keyboard);
			break;
		case 2:
			on_keyboard_set_layout_special(keyboard);
			break;
	}
}
