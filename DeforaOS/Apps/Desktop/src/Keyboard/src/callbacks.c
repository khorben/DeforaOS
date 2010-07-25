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



#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>
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


/* on_keyboard_key_clicked */
void on_keyboard_key_clicked(gpointer data, GtkWidget * key)
{
	Keyboard * keyboard = data;
	unsigned int * keysym;
	unsigned int keycode;
	Display * display;
	gboolean upper;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, keyboard,
			gtk_button_get_label(GTK_BUTTON(key)));
#endif
	if((keysym = g_object_get_data(G_OBJECT(key), "keysym")) == NULL)
		return;
	display = gdk_x11_get_default_xdisplay();
	keycode = XKeysymToKeycode(display, *keysym);
	XTestGrabControl(display, True);
	if(*keysym == XK_Shift_L || *keysym == XK_Shift_R)
	{
		upper = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(key));
		XTestFakeKeyEvent(display, keycode, upper ? True : False, 0);
		keyboard_set_case(keyboard, upper ? KC_UPPER : KC_LOWER);
	}
	else
	{
		XTestFakeKeyEvent(display, keycode, True, 0);
		XTestFakeKeyEvent(display, keycode, False, 0);
	}
	XTestGrabControl(display, False);
}
