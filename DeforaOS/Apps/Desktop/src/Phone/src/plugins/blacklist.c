/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include "Phone.h"


/* Blacklist */
/* private */
/* prototypes */
static int _blacklist_event(PhonePlugin * plugin, PhoneEvent event, ...);
static void _blacklist_settings(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Blacklist",
	NULL,
	NULL,
	NULL,
	_blacklist_event,
	_blacklist_settings,
	NULL
};


/* private */
/* functions */
/* blacklist_event */
static int _blacklist_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	va_list ap;
	char const * number = NULL;
	char const * reason;

	va_start(ap, event);
	switch(event)
	{
		case PHONE_EVENT_CALLING:
			number = va_arg(ap, char const *);
			break;
		default:
			break;
	}
	va_end(ap);
	if(number == NULL)
		return 0;
	reason = plugin->helper->config_get(plugin->helper->phone, "blacklist",
			number);
	if(reason == NULL)
		return 0;
	return plugin->helper->error(plugin->helper->phone, reason, 1);
}


/* blacklist_settings */
static void _blacklist_settings(PhonePlugin * plugin)
{
	/* FIXME implement */
}
