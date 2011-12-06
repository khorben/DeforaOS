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
#include <string.h>
#include <gdk/gdkx.h>
#include "common.h"
#include "key.h"


/* KeyboardKey */
/* private */
/* types */
typedef struct _KeyboardKeyModifier
{
	unsigned int modifier;
	unsigned int keysym;
	char * label;
} KeyboardKeyModifier;

struct _KeyboardKey
{
	GtkWidget * widget;
	GtkWidget * label;
	GtkWidget * popup;
	GtkWidget * button;
	KeyboardKeyModifier key;
	KeyboardKeyModifier * modifiers;
	size_t modifiers_cnt;
	KeyboardKeyModifier * current;
};


/* prototypes */
static void _keyboard_key_create_popup(KeyboardKey * key);

/* callbacks */
static gboolean _on_keyboard_key_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data);
static gboolean _on_keyboard_key_button_release(GtkWidget * widget,
		GdkEventButton * event, gpointer data);


/* public */
/* functions */
/* keyboard_key_new */
KeyboardKey * keyboard_key_new(unsigned int keysym, char const * label)
{
	KeyboardKey * key;

	if((key = malloc(sizeof(*key))) == NULL)
		return NULL;
	if(keysym_is_modifier(keysym))
		key->widget = gtk_toggle_button_new();
	else
		key->widget = gtk_button_new();
	g_signal_connect(G_OBJECT(key->widget), "button-press-event",
			G_CALLBACK(_on_keyboard_key_button_press), key);
	g_signal_connect(G_OBJECT(key->widget), "button-release-event",
			G_CALLBACK(_on_keyboard_key_button_release), key);
	key->label = gtk_label_new(label);
	gtk_container_add(GTK_CONTAINER(key->widget), key->label);
	key->popup = NULL;
	key->button = NULL;
	key->key.modifier = 0;
	key->key.keysym = keysym;
	key->key.label = strdup(label);
	key->modifiers = NULL;
	key->modifiers_cnt = 0;
	key->current = &key->key;
	if(key->key.label == NULL)
	{
		keyboard_key_delete(key);
		return NULL;
	}
	return key;
}


/* keyboard_key_delete */
void keyboard_key_delete(KeyboardKey * key)
{
	size_t i;

	for(i = 0; i < key->modifiers_cnt; i++)
		free(key->modifiers[i].label);
	free(key->modifiers);
	free(key->key.label);
	free(key);
}


/* accessors */
/* keyboard_key_get_keysym */
unsigned int keyboard_key_get_keysym(KeyboardKey * key)
{
	return key->current->keysym;
}


/* keyboard_key_get_label_widget */
GtkWidget * keyboard_key_get_label_widget(KeyboardKey * key)
{
	return key->label;
}


/* keyboard_key_get_widget */
GtkWidget * keyboard_key_get_widget(KeyboardKey * key)
{
	return key->widget;
}


/* keyboard_key_set_background */
void keyboard_key_set_background(KeyboardKey * key, GdkColor * color)
{
	_keyboard_key_create_popup(key);
	gtk_widget_modify_bg(key->widget, GTK_STATE_NORMAL, color);
	gtk_widget_modify_bg(key->popup, GTK_STATE_NORMAL, color);
	gtk_widget_modify_bg(key->button, GTK_STATE_NORMAL, color);
}


/* keyboard_key_set_font */
void keyboard_key_set_font(KeyboardKey * key, PangoFontDescription * font)
{
	_keyboard_key_create_popup(key);
	gtk_widget_modify_font(key->label, font);
	gtk_widget_modify_font(gtk_bin_get_child(GTK_BIN(key->button)), font);
}


/* keyboard_key_set_foreground */
void keyboard_key_set_foreground(KeyboardKey * key, GdkColor * color)
{
	_keyboard_key_create_popup(key);
	gtk_widget_modify_fg(key->label, GTK_STATE_NORMAL, color);
	gtk_widget_modify_fg(key->button, GTK_STATE_NORMAL, color);
}


/* keyboard_key_set_modifier */
int keyboard_key_set_modifier(KeyboardKey * key, unsigned int modifier,
		unsigned int keysym, char const * label)
{
	char * p;
	KeyboardKeyModifier * q;

	if(label == NULL || (p = strdup(label)) == NULL)
		return -1;
	if(modifier == 0)
	{
		key->key.keysym = keysym;
		free(key->key.label);
		key->key.label = p;
		return 0;
	}
	if((q = realloc(key->modifiers, sizeof(*q) * (key->modifiers_cnt + 1)))
			== NULL)
	{
		free(p);
		return -1;
	}
	key->modifiers = q;
	q = &key->modifiers[key->modifiers_cnt++];
	q->modifier = modifier;
	q->keysym = keysym;
	q->label = p;
	return 0;
}


/* useful */
/* keyboard_key_apply_modifier */
void keyboard_key_apply_modifier(KeyboardKey * key, unsigned int modifier)
{
	char const * label = key->key.label;
	size_t i;

	key->current = &key->key;
	if(modifier != 0)
		for(i = 0; i < key->modifiers_cnt; i++)
			if(key->modifiers[i].modifier == modifier)
			{
				label = key->modifiers[i].label;
				key->current = &key->modifiers[i];
				break;
			}
	gtk_label_set_text(GTK_LABEL(key->label), label);
}


/* private */
/* functions */
/* keyboard_key_create_popup */
static void _keyboard_key_create_popup(KeyboardKey * key)
{
	if(key->popup != NULL)
		return;
	key->popup = gtk_window_new(GTK_WINDOW_POPUP);
	key->button = gtk_button_new_with_label(gtk_label_get_text(GTK_LABEL(
					key->label)));
	gtk_button_set_alignment(GTK_BUTTON(key->button), 0.5, 0.1);
	gtk_container_add(GTK_CONTAINER(key->popup), key->button);
}


/* callbacks */
/* on_keyboard_key_button_press */
static gboolean _on_keyboard_key_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data)
{
	KeyboardKey * key = data;
	gint width;
	gint height;

	gdk_drawable_get_size(event->window, &width, &height);
	_keyboard_key_create_popup(key);
	gtk_widget_set_size_request(key->popup, width + 8, height * 2);
	gtk_window_move(GTK_WINDOW(key->popup), event->x_root - event->x - 4,
			event->y_root - event->y - height * 2);
	gtk_widget_show_all(key->popup);
	return FALSE;
}


/* on_keyboard_key_button_release */
static gboolean _on_keyboard_key_button_release(GtkWidget * widget,
		GdkEventButton * event, gpointer data)
{
	KeyboardKey * key = data;

	if(key->popup != NULL)
		gtk_widget_hide(key->popup);
	return FALSE;
}
