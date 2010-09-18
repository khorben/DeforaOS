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



#include <string.h>
#include "keyboard.h"
#include "callbacks.h"


/* public */
/* functions */
/* on_keyboard_delete_event */
gboolean on_keyboard_delete_event(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard_show(keyboard, FALSE);
	gtk_main_quit();
	return TRUE;
}


/* on_keyboard_embedded */
void on_keyboard_embedded(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard_show(keyboard, TRUE);
}


/* on_keyboard_set_layout_keypad */
void on_keyboard_set_layout_keypad(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard_set_layout(keyboard, 1); /* XXX hard-coded */
}


/* on_keyboard_set_layout_letters */
void on_keyboard_set_layout_letters(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard_set_layout(keyboard, 0); /* XXX hard-coded */
}


/* on_keyboard_set_layout_special */
void on_keyboard_set_layout_special(gpointer data)
{
	Keyboard * keyboard = data;

	keyboard_set_layout(keyboard, 2); /* XXX hard-coded */
}
