/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
#include <System.h>
#include <Desktop.h>
#include <Desktop/Locker.h>
#include "Phone.h"


/* Locker */
/* private */
/* types */
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;
} LockerPhonePlugin;


/* prototypes */
/* plug-in */
static LockerPhonePlugin * _locker_init(PhonePluginHelper * helper);
static void _locker_destroy(LockerPhonePlugin * locker);
static int _locker_event(LockerPhonePlugin * locker, PhoneEvent * event);

/* useful */
static int _locker_action(LockerAction action);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"Locker",
	NULL,
	NULL,
	_locker_init,
	_locker_destroy,
	_locker_event,
	NULL
};


/* private */
/* functions */
/* locker_init */
static LockerPhonePlugin * _locker_init(PhonePluginHelper * helper)
{
	LockerPhonePlugin * locker;

	if((locker = object_new(sizeof(*locker))) == NULL)
		return NULL;
	locker->helper = helper;
	return locker;
}


/* locker_destroy */
static void _locker_destroy(LockerPhonePlugin * locker)
{
	object_delete(locker);
}


/* locker_event */
static int _event_modem(ModemEvent * event);

static int _locker_event(LockerPhonePlugin * locker, PhoneEvent * event)
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
			/* FIXME this is a security issue */
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
	desktop_message_send(LOCKER_CLIENT_MESSAGE, LOCKER_MESSAGE_ACTION,
			action, TRUE);
	return 0;
}
