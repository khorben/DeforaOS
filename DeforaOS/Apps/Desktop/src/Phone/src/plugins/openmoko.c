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
/* TODO:
 * - implement notification light
 * - implement vibrator
 * - prevent deep sleep */



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
#ifndef SBINDIR
# define SBINDIR	PREFIX "/sbin"
#endif


/* Openmoko */
/* private */
static int _openmoko_event(PhonePlugin * plugin, PhoneEvent event, ...);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	NULL,
	NULL,
	_openmoko_event,
	NULL
};


/* private */
/* functions */
/* openmoko_event */
static int _event_mixer_set(char const * filename);

static int _openmoko_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	switch(event)
	{
		case PHONE_EVENT_CALL_ESTABLISHED:
			/* let us hear the call */
			_event_mixer_set("gsmhandset.state");
			/* enable echo cancellation */
			plugin->helper->queue(plugin->helper->phone,
					"AT%N0187");
			break;
		case PHONE_EVENT_CALL_INCOMING:
			/* let us hear the ringtone */
			_event_mixer_set("stereoout.state");
			break;
		case PHONE_EVENT_CALL_OUTGOING:
			/* let us hear the connection */
			_event_mixer_set("gsmhandset.state");
			break;
		case PHONE_EVENT_CALL_TERMINATED:
			/* restore regular audio */
			_event_mixer_set("stereoout.state");
			break;
		case PHONE_EVENT_NOTIFICATION_OFF:
			/* FIXME implement */
			break;
		case PHONE_EVENT_NOTIFICATION_ON:
			/* FIXME implement */
			break;
		case PHONE_EVENT_SPEAKER_ON:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set("gsmspeakerout.state");
			break;
		case PHONE_EVENT_SPEAKER_OFF:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set("gsmhandset.state");
			break;
		case PHONE_EVENT_VIBRATOR_OFF:
			/* FIXME implement */
			break;
		case PHONE_EVENT_VIBRATOR_ON:
			/* FIXME implement */
			break;
		/* not relevant */
		case PHONE_EVENT_SIM_VALID: /* FIXME prevent deep sleep? */
		case PHONE_EVENT_SMS_RECEIVED:
		case PHONE_EVENT_SMS_SENT:
			break;
	}
	return 0;
}

static int _event_mixer_set(char const * filename)
{
	char const scenarios[] = DATADIR "/openmoko/scenarios";
	char * pathname;
	size_t len;
	char * alsactl[] = { SBINDIR "/alsactl", "alsactl", "-f", NULL,
		"restore", NULL };

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
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
