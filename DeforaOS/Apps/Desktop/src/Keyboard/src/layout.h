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



#ifndef KEYBOARD_LAYOUT_H
# define KEYBOARD_LAYOUT_H

# include <gtk/gtk.h>
# include "key.h"


/* Keyboard */
/* types */
typedef struct _KeyboardLayout KeyboardLayout;


/* functions */
KeyboardLayout * keyboard_layout_new(void);
void keyboard_layout_delete(KeyboardLayout * layout);

/* accessors */
GtkWidget * keyboard_layout_get_widget(KeyboardLayout * layout);

/* useful */
KeyboardKey * keyboard_layout_add(KeyboardLayout * layout, unsigned int row,
		unsigned int width, unsigned int keysym, char const * label);
void keyboard_layout_add_widget(KeyboardLayout * layout, unsigned int row,
		unsigned int column, unsigned int width, GtkWidget * widget);
void keyboard_layout_apply_modifier(KeyboardLayout * layout,
		unsigned int modifier);

#endif /* !KEYBOARD_LAYOUT_H */
