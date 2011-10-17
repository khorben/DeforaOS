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



#include <System.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include "Locker.h"


/* Openmoko */
/* private */
/* prototypes */
/* plug-in */
static int _openmoko_init(LockerPlugin * plugin);
static void _openmoko_destroy(LockerPlugin * plugin);
static void _openmoko_event(LockerPlugin * plugin, LockerEvent event);


/* public */
/* variables */
/* plug-in */
LockerPlugin plugin =
{
	NULL,
	"Openmoko",
	_openmoko_init,
	_openmoko_destroy,
	_openmoko_event,
	NULL
};


/* private */
/* functions */
/* openmoko_init */
static int _openmoko_init(LockerPlugin * plugin)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME really implement */
	return 0;
}


/* openmoko_destroy */
static void _openmoko_destroy(LockerPlugin * plugin)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME really implement */
}


/* openmoko_event */
static void _openmoko_event(LockerPlugin * plugin, LockerEvent event)
{
	switch(event)
	{
		/* FIXME implement */
		case LOCKER_EVENT_ACTIVATING:
		case LOCKER_EVENT_LOCKING:
		case LOCKER_EVENT_UNLOCKING:
			break;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() Unhandled event (%u)\n", __func__, event);
#endif
}
