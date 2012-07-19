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



#include "Locker.h"
#include <stdio.h>
#include <System.h>


/* Suspend */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
	gint source;
} Suspend;


/* prototypes */
/* plug-in */
static Suspend * _suspend_init(LockerPluginHelper * helper);
static void _suspend_destroy(Suspend * suspend);
static int _suspend_event(Suspend * suspend, LockerEvent event);

/* callbacks */
static gboolean _suspend_on_timeout(gpointer data);


/* public */
/* variables */
/* plug-in */
LockerPluginDefinition plugin =
{
	"Suspend",
	NULL,
	NULL,
	_suspend_init,
	_suspend_destroy,
	_suspend_event
};


/* private */
/* functions */
/* suspend_init */
static Suspend * _suspend_init(LockerPluginHelper * helper)
{
	Suspend * suspend;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((suspend = object_new(sizeof(*suspend))) == NULL)
		return NULL;
	suspend->helper = helper;
	suspend->source = 0;
	return suspend;
}


/* suspend_destroy */
static void _suspend_destroy(Suspend * suspend)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(suspend->source != 0)
		g_source_remove(suspend->source);
	object_delete(suspend);
}


/* suspend_event */
static int _suspend_event(Suspend * suspend, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_DEACTIVATED:
		case LOCKER_EVENT_UNLOCKED:
			/* cancel any pending suspend */
			if(suspend->source != 0)
				g_source_remove(suspend->source);
			suspend->source = 0;
			break;
		case LOCKER_EVENT_ACTIVATED:
		case LOCKER_EVENT_LOCKED:
			/* queue a suspend if not already */
			if(suspend->source != 0)
				break;
			/* XXX let the delay be configurable */
			suspend->source = g_timeout_add(10000,
					_suspend_on_timeout, suspend);
			break;
		default:
			/* ignore the other events */
			break;
	}
	return 0;
}


/* callbacks */
/* suspend_on_timeout */
static gboolean _suspend_on_timeout(gpointer data)
{
	Suspend * suspend = data;
	LockerPluginHelper * helper = suspend->helper;

	suspend->source = 0;
	helper->action(helper->locker, LOCKER_ACTION_SUSPEND);
	return FALSE;
}
