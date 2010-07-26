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



#ifndef KEYBOARD_CALLBACKS_H
# define KEYBOARD_CALLBACKS_H

# include <gtk/gtk.h>


/* public */
/* functions */
gboolean on_keyboard_delete_event(gpointer data);
void on_keyboard_key_clicked(gpointer data, GtkWidget * key);
gboolean on_keyboard_key_pressed(GtkWidget * key, GdkEventButton * event,
		gpointer data);
gboolean on_keyboard_key_released(GtkWidget * key, GdkEventButton * event,
		gpointer data);

#endif /* !KEYBOARD_CALLBACKS_H */
