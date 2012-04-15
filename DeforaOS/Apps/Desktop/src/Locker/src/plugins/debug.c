/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
#include <stdio.h>
#include "Locker.h"


/* Debug */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
} Debug;


/* prototypes */
/* plug-in */
static Debug * _debug_init(LockerPluginHelper * helper);
static void _debug_destroy(Debug * debug);
static void _debug_event(Debug * debug, LockerEvent event);


/* public */
/* variables */
/* plug-in */
LockerPluginDefinition plugin =
{
	"Debug",
	"applications-development",
	NULL,
	_debug_init,
	_debug_destroy,
	_debug_event
};


/* private */
/* functions */
/* debug_init */
static Debug * _debug_init(LockerPluginHelper * helper)
{
	Debug * debug;

	if((debug = object_new(sizeof(*debug))) == NULL)
		return NULL;
	debug->helper = helper;
	fprintf(stderr, "DEBUG: %s()\n", __func__);
	return debug;
}


/* debug_destroy */
static void _debug_destroy(Debug * debug)
{
	fprintf(stderr, "DEBUG: %s()\n", __func__);
	object_delete(debug);
}


/* debug_event */
static void _debug_event(Debug * debug, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_ACTIVATING:
			fprintf(stderr, "DEBUG: %s() ACTIVATING\n", __func__);
			return;
		case LOCKER_EVENT_LOCKING:
			fprintf(stderr, "DEBUG: %s() LOCKING\n", __func__);
			return;
		case LOCKER_EVENT_SUSPENDING:
			fprintf(stderr, "DEBUG: %s() SUSPENDING\n", __func__);
			return;
		case LOCKER_EVENT_UNLOCKING:
			fprintf(stderr, "DEBUG: %s() UNLOCKING\n", __func__);
			return;
	}
	fprintf(stderr, "DEBUG: %s() Unknown event (%u)\n", __func__, event);
}
