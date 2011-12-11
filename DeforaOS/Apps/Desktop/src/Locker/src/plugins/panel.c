/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <string.h>
#include <gtk/gtk.h>
#include <System.h>
#include <Desktop/Panel.h>
#include "Locker.h"


/* Panel */
/* private */
/* prototypes */
/* plug-in */
static void _panel_event(LockerPlugin * plugin, LockerEvent event);


/* public */
/* variables */
LockerPlugin plugin =
{
	NULL,
	"Panel",
	"gnome-monitor",
	NULL,
	NULL,
	_panel_event,
	NULL
};


/* private */
/* functions */
/* panel_event */
static void _event_show(gboolean show);

static void _panel_event(LockerPlugin * plugin, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_ACTIVATING:
		case LOCKER_EVENT_LOCKING:
			_event_show(FALSE);
			break;
		case LOCKER_EVENT_UNLOCKING:
			_event_show(TRUE);
			break;
	}
}

static void _event_show(gboolean show)
{
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PANEL_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = PANEL_MESSAGE_SHOW;
	client->data.b[1] = PANEL_MESSAGE_SHOW_PANEL_BOTTOM
		| PANEL_MESSAGE_SHOW_PANEL_TOP;
	client->data.b[2] = show;
	gdk_event_send_clientmessage_toall(&event);
}
