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



#ifndef KEYBOARD_KEYBOARD_H
# define KEYBOARD_KEYBOARD_H

# include <gtk/gtk.h>
# include "Keyboard.h"
# include "key.h"


/* Keyboard */
/* types */
typedef struct _Keyboard Keyboard;

typedef enum _KeyboardMode
{
	KEYBOARD_MODE_WINDOWED = 0,
	KEYBOARD_MODE_DOCKED,
	KEYBOARD_MODE_EMBEDDED,
	KEYBOARD_MODE_POPUP
} KeyboardMode;

typedef struct _KeyboardPrefs
{
	int monitor;
	char const * font;
	KeyboardMode mode;
} KeyboardPrefs;


/* functions */
Keyboard * keyboard_new(KeyboardPrefs * prefs);
void keyboard_delete(Keyboard * keyboard);

/* accessors */
/* XXX be more explicit */
void keyboard_set_layout(Keyboard * keyboard, unsigned int which);
void keyboard_set_page(Keyboard * keyboard, KeyboardPage page);

/* useful */
void keyboard_show(Keyboard * keyboard, gboolean show);
void keyboard_show_about(Keyboard * keyboard);

void keyboard_key_show(Keyboard * keyboard, KeyboardKey * key, gboolean show,
		GdkEventButton * event);

#endif /* !KEYBOARD_KEYBOARD_H */
