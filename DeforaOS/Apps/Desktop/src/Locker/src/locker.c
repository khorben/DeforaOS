/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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



#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>
#include <System.h>
#include "locker.h"


/* Locker */
/* private */
/* types */
struct _Locker
{
	GdkDisplay * display;
	int screen;
	int event;
};


/* public */
/* functions */
/* locker_new */
static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Locker * locker_new(void)
{
	Locker * locker;
	GdkScreen * screen;
	int error;
	Window root;

	if((locker = object_new(sizeof(*locker))) == NULL)
		return NULL;
	screen = gdk_screen_get_default();
	locker->display = gdk_screen_get_display(screen);
	locker->screen = gdk_x11_get_default_screen();
	if(XScreenSaverQueryExtension(GDK_DISPLAY_XDISPLAY(locker->display),
				&locker->event, &error) == 0
			|| XScreenSaverRegister(
				GDK_DISPLAY_XDISPLAY(locker->display),
				locker->screen, getpid(), XA_INTEGER) == 0)
	{
		object_delete(locker);
		return NULL;
	}
	root = gdk_x11_get_default_root_xwindow();
	XScreenSaverSelectInput(GDK_DISPLAY_XDISPLAY(locker->display), root,
			ScreenSaverNotifyMask);
	gdk_x11_register_standard_event_type(locker->display, locker->event, 1);
	gdk_window_add_filter(gdk_get_default_root_window(), _locker_on_filter,
			locker);
	return locker;
}

static GdkFilterReturn _locker_on_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Locker * locker = data;
	XScreenSaverNotifyEvent * xssne = xevent;

	if(xssne->type != locker->event)
		return GDK_FILTER_CONTINUE;
#if 1 /* def DEBUG */
	fprintf(stderr, "DEBUG: %s() 0x%x\n", __func__, event->type);
#endif
	switch(xssne->state)
	{
		case ScreenSaverOff:
			/* FIXME implement */
			break;
		case ScreenSaverOn:
			/* FIXME implement */
			break;
		case ScreenSaverDisabled:
			/* FIXME implement */
			break;
	}
	return GDK_FILTER_CONTINUE;
}


/* locker_delete */
void locker_delete(Locker * locker)
{
	XScreenSaverUnregister(GDK_DISPLAY_XDISPLAY(locker->display),
			locker->screen);
	object_delete(locker);
}
