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



#include <stdio.h>
#include "Phone.h"


/* Openmoko */
/* private */
static void _openmoko_event(PhoneEvent event);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	NULL,
	_openmoko_event
};


/* private */
/* functions */
/* openmoko_init */
static void _openmoko_event(PhoneEvent event)
{
	switch(event)
	{
		case PHONE_EVENT_CALL_ESTABLISHED:
		case PHONE_EVENT_CALL_INCOMING:
		case PHONE_EVENT_CALL_OUTGOING:
		case PHONE_EVENT_CALL_TERMINATED:
			/* FIXME implement */
			break;
	}
}
