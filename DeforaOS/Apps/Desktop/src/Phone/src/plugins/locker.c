/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <Desktop/Locker.h>
#include "Phone.h"


/* Locker */
/* private */
/* prototypes */
/* plug-in */
static int _locker_event(PhonePlugin * plugin, PhoneEvent * event);

/* useful */
static int _locker_action(LockerAction action);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Locker",
	NULL,
	NULL,
	NULL,
	_locker_event,
	NULL,
	NULL
};


/* private */
/* functions */
/* locker_event */
static int _event_modem(ModemEvent * event);

static int _locker_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			return _event_modem(event->modem_event.event);
		case PHONE_EVENT_TYPE_SUSPEND:
			return _locker_action(LOCKER_ACTION_LOCK);
		default: /* not relevant */
			break;
	}
	return 0;
}

static int _event_modem(ModemEvent * event)
{
	switch(event->call.direction)
	{
		case MODEM_CALL_DIRECTION_INCOMING:
			return _locker_action(LOCKER_ACTION_UNLOCK);
		default: /* not relevant */
			break;
	}
	return 0;
}


/* useful */
/* locker_action */
static int _locker_action(LockerAction action)
{
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(LOCKER_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = LOCKER_MESSAGE_ACTION;
	client->data.b[1] = action;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
	return 0;
}
