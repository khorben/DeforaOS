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



#ifndef KEYBOARD_KEY_H
# define KEYBOARD_KEY_H

# include <gtk/gtk.h>


/* Keyboard */
/* types */
typedef struct _KeyboardKey KeyboardKey;


/* functions */
KeyboardKey * keyboard_key_new(unsigned int keysym, char const * label);
void keyboard_key_delete(KeyboardKey * key);

/* accessors */
unsigned int keyboard_key_get_keysym(KeyboardKey * key);
GtkWidget * keyboard_key_get_label_widget(KeyboardKey * key);
GtkWidget * keyboard_key_get_widget(KeyboardKey * key);
unsigned int keyboard_key_get_width(KeyboardKey * key);

int keyboard_key_set_modifier(KeyboardKey * key, unsigned int modifier,
		unsigned int keysym, char const * label);

/* useful */
void keyboard_key_apply_modifier(KeyboardKey * key, unsigned int modifier);

#endif /* !KEYBOARD_KEY_H */
