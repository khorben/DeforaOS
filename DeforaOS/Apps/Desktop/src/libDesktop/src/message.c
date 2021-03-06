/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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
/* TODO:
 * - proxy for the libApp framework instead */



#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3, 0, 0)
# include <gdk/gdkx.h>
#endif
#include <System.h>
#include "Desktop.h"


/* Message */
/* private */
/* types */
typedef struct _MessageCallback
{
	DesktopMessageCallback callback;
	void * data;
} MessageCallback;


/* prototypes */
/* callbacks */
#if !GTK_CHECK_VERSION(3, 0, 0)
static GdkFilterReturn _desktop_message_on_callback(GdkXEvent * xevent,
		GdkEvent * event, gpointer data);
#endif


/* public */
/* functions */
/* desktop_message_register */
int desktop_message_register(char const * destination,
		DesktopMessageCallback callback, void * data)
{
#if !GTK_CHECK_VERSION(3, 0, 0)
	MessageCallback * mc;

	if((mc = object_new(sizeof(*mc))) == NULL)
		return -1;
	mc->callback = callback;
	mc->data = data;
	gdk_add_client_message_filter(gdk_atom_intern(destination, FALSE),
			_desktop_message_on_callback, mc);
#endif
	return 0;
}


/* desktop_message_send */
int desktop_message_send(char const * destination, uint32_t value1,
		uint32_t value2, uint32_t value3)
{
#if !GTK_CHECK_VERSION(3, 0, 0)
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(destination, FALSE);
	client->data_format = 32;
	client->data.l[0] = value1;
	client->data.l[1] = value2;
	client->data.l[2] = value3;
	gdk_event_send_clientmessage_toall(&event);
#endif
	return 0;
}


/* private */
/* callbacks */
#if !GTK_CHECK_VERSION(3, 0, 0)
/* desktop_message_on_callback */
static GdkFilterReturn _desktop_message_on_callback(GdkXEvent * xevent,
		GdkEvent * event, gpointer data)
{
	MessageCallback * mc = data;
	XEvent * xev = xevent;
	XClientMessageEvent * xcme;
	uint32_t value1;
	uint32_t value2;
	uint32_t value3;

	if(xev->type != ClientMessage)
		return GDK_FILTER_CONTINUE;
	xcme = &xev->xclient;
	value1 = xcme->data.l[0];
	value2 = xcme->data.l[1];
	value3 = xcme->data.l[2];
	if(mc->callback(mc->data, value1, value2, value3) == 0)
		return GDK_FILTER_CONTINUE;
	object_delete(mc);
	return GDK_FILTER_REMOVE;
}
#endif
