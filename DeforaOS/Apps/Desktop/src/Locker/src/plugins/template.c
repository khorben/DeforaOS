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


/* Template */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
} Template;


/* prototypes */
/* plug-in */
static Template * _template_init(LockerPluginHelper * helper);
static void _template_destroy(Template * template);
static int _template_event(Template * template, LockerEvent event);


/* public */
/* variables */
/* plug-in */
LockerPluginDefinition plugin =
{
	"Template",
	NULL,
	NULL,
	_template_init,
	_template_destroy,
	_template_event
};


/* private */
/* functions */
/* template_init */
static Template * _template_init(LockerPluginHelper * helper)
{
	Template * template;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	return template;
}


/* template_destroy */
static void _template_destroy(Template * template)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	object_delete(template);
}


/* template_event */
static int _template_event(Template * template, LockerEvent event)
{
	switch(event)
	{
		case LOCKER_EVENT_ACTIVATED:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() ACTIVATED\n", __func__);
#endif
			break;
		case LOCKER_EVENT_ACTIVATING:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() ACTIVATING\n", __func__);
#endif
			break;
		case LOCKER_EVENT_DEACTIVATED:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() DEACTIVATED\n", __func__);
#endif
			break;
		case LOCKER_EVENT_DEACTIVATING:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() DEACTIVATING\n", __func__);
#endif
			break;
		case LOCKER_EVENT_LOCKED:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() LOCKED\n", __func__);
#endif
			break;
		case LOCKER_EVENT_LOCKING:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() LOCKING\n", __func__);
#endif
			break;
		case LOCKER_EVENT_SUSPENDING:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() SUSPENDING\n", __func__);
#endif
			break;
		case LOCKER_EVENT_UNLOCKED:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() UNLOCKED\n", __func__);
#endif
			break;
		case LOCKER_EVENT_UNLOCKING:
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() UNLOCKING\n", __func__);
#endif
			break;
#ifdef DEBUG
		default:
			fprintf(stderr, "DEBUG: %s() Unknown event (%u)\n",
					__func__, event);
			break;
#endif
	}
	return 0;
}
