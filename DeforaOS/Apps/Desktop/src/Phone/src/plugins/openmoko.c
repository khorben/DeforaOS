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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <System.h>
#include "Phone.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


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
static int _event_mixer_set(char const * filename);

static void _openmoko_event(PhoneEvent event)
{
	switch(event)
	{
		case PHONE_EVENT_CALL_ESTABLISHED:
			_event_mixer_set("gsmhandset.xml");
			break;
		case PHONE_EVENT_CALL_INCOMING:
		case PHONE_EVENT_CALL_OUTGOING:
			/* FIXME is there anything to do? */
			break;
		case PHONE_EVENT_CALL_TERMINATED:
			_event_mixer_set("speaker.xml");
			break;
	}
}

static int _event_mixer_set(char const * filename)
{
	char const scenarios[] = DATADIR "/openmoko/scenarios";
	char * pathname;
	size_t len;
	char * alsactl[] = { "/usr/sbin/alsactl", "alsactl", "-f", NULL,
		"restore", NULL };

	len = sizeof(scenarios) + 1 + strlen(filename);
	if((pathname = malloc(len)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	snprintf(pathname, len, "%s/%s", scenarios, filename);
	alsactl[3] = pathname;
	g_spawn_async(NULL, alsactl, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
			NULL, NULL, NULL, NULL);
	free(pathname);
	return 0;
}
