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


/* on_keyboard_embedded */
void on_keyboard_embedded(gpointer data)
{
	Keyboard * keyboard = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	keyboard_show(keyboard, TRUE);
}


/* on_keyboard_key_clicked */
void on_keyboard_key_clicked(gpointer data, GtkWidget * key)
{
	Keyboard * keyboard = data;
	KeyboardKey * kk;
	unsigned int keycode;
	Display * display;
	gboolean upper;

	if((kk = g_object_get_data(G_OBJECT(key), "key")) == NULL)
		return;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, keyboard,
			kk->label);
#endif
	display = gdk_x11_get_default_xdisplay();
	keycode = XKeysymToKeycode(display, kk->keysym);
	XTestGrabControl(display, True);
	if(kk->keysym == XK_Shift_L || kk->keysym == XK_Shift_R)
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


/* on_keyboard_key_pressed */
gboolean on_keyboard_key_pressed(GtkWidget * key, GdkEventButton * event,
		gpointer data)
{
	Keyboard * keyboard = data;
	KeyboardKey * kk;

	if((kk = g_object_get_data(G_OBJECT(key), "key")) != NULL)
		keyboard_key_show(keyboard, kk, TRUE);
	return FALSE;
}


/* on_keyboard_key_released */
gboolean on_keyboard_key_released(GtkWidget * key, GdkEventButton * event,
		gpointer data)
{
	Keyboard * keyboard = data;
	KeyboardKey * kk;

	if((kk = g_object_get_data(G_OBJECT(key), "key")) != NULL)
		keyboard_key_show(keyboard, kk, FALSE);
	return FALSE;
}
