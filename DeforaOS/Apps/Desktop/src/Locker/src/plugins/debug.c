/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
/* prototypes */
/* plug-in */
static int _debug_init(LockerPlugin * plugin);
static void _debug_destroy(LockerPlugin * plugin);
static void _debug_event(LockerPlugin * plugin, LockerEvent event);


/* public */
/* variables */
/* plug-in */
LockerPlugin plugin =
{
	NULL,
	"Debug",
	"applications-development",
	_debug_init,
	_debug_destroy,
	_debug_event,
	NULL
};


/* private */
/* functions */
/* debug_init */
static int _debug_init(LockerPlugin * plugin)
{
	fprintf(stderr, "DEBUG: %s()\n", __func__);
	return 0;
}


/* debug_destroy */
static void _debug_destroy(LockerPlugin * plugin)
{
	fprintf(stderr, "DEBUG: %s()\n", __func__);
}


/* debug_event */
static void _debug_event(LockerPlugin * plugin, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_ACTIVATING:
			fprintf(stderr, "DEBUG: %s() ACTIVATING\n", __func__);
			return;
		case LOCKER_EVENT_LOCKING:
			fprintf(stderr, "DEBUG: %s() LOCKING\n", __func__);
			return;
		case LOCKER_EVENT_UNLOCKING:
			fprintf(stderr, "DEBUG: %s() UNLOCKING\n", __func__);
			return;
	}
	fprintf(stderr, "DEBUG: %s() Unknown event (%u)\n", __func__, event);
}
