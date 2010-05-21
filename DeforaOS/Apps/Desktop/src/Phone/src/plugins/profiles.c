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
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <pulse/pulseaudio.h>
#include <System.h>
#include "Phone.h"
#include "../../config.h"
#define _(string) gettext(string)

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


/* Profiles */
/* private */
/* types */
typedef struct _Profiles
{
	int event;
	guint source;

	/* vibrator */
	int vibrator;

	/* pulseaudio */
	pa_threaded_mainloop * pam;
	pa_context * pac;
	pa_operation * pao;
} Profiles;

/* prototypes */
static int _profiles_init(PhonePlugin * plugin);
static int _profiles_destroy(PhonePlugin * plugin);
static int _profiles_event(PhonePlugin * plugin, PhoneEvent event, ...);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	_profiles_init,
	_profiles_destroy,
	_profiles_event,
	NULL
};


/* private */
/* functions */
/* profiles_init */
static int _profiles_init(PhonePlugin * plugin)
{
	Profiles * profiles;
	pa_mainloop_api * mapi = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((profiles = malloc(sizeof(*profiles))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	plugin->priv = profiles;
	profiles->event = -1;
	profiles->source = 0;
	profiles->vibrator = 0;
	profiles->pam = pa_threaded_mainloop_new();
	profiles->pac = NULL;
	profiles->pao = NULL;
	if(profiles->pam == NULL)
	{
		_profiles_destroy(plugin);
		return error_set_code(1, "%s",
				_("Could not initialize PulseAudio"));
	}
	mapi = pa_threaded_mainloop_get_api(profiles->pam);
	/* XXX update the context name */
	if((profiles->pac = pa_context_new(mapi, PACKAGE)) == NULL)
	{
		_profiles_destroy(plugin);
		return error_set_code(1, "%s",
				_("Could not initialize PulseAudio"));
	}
	pa_context_connect(profiles->pac, NULL, 0, NULL);
	pa_threaded_mainloop_start(profiles->pam);
	return 0;
}


/* profiles_destroy */
static int _profiles_destroy(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(profiles->source != 0)
		g_source_remove(profiles->source);
	if(profiles->pao != NULL)
		pa_operation_cancel(profiles->pao);
	if(profiles->pac != NULL)
		pa_context_unref(profiles->pac);
	pa_threaded_mainloop_free(profiles->pam);
	free(profiles);
	return 0;
}


/* profiles_event */
static gboolean _event_call_incoming_timeout(gpointer data);

static int _profiles_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;

	if(profiles->event == (int)event)
		/* FIXME this should probably only apply to phone calls */
		return 0; /* already taking care of it */
	if(profiles->source != 0)
		g_source_remove(profiles->source);
	profiles->source = 0;
	if(profiles->pao != NULL)
		pa_operation_cancel(profiles->pao);
	profiles->pao = NULL;
	switch(event)
	{
		case PHONE_EVENT_CALL_INCOMING:
			profiles->pao = pa_context_play_sample(profiles->pac,
					"ringtone", NULL, PA_VOLUME_NORM, NULL,
					NULL);
			profiles->source = g_timeout_add(500,
					_event_call_incoming_timeout, plugin);
			helper->event(helper->phone, PHONE_EVENT_VIBRATOR_ON);
			profiles->vibrator = 1;
			break;
		case PHONE_EVENT_SMS_RECEIVED:
			profiles->pao = pa_context_play_sample(profiles->pac,
					"message", NULL, PA_VOLUME_NORM, NULL,
					NULL);
			break;
		case PHONE_EVENT_SIM_VALID:
		case PHONE_EVENT_SMS_SENT:
			/* FIXME beep in general profile? */
			break;
		case PHONE_EVENT_CALL_OUTGOING:
		case PHONE_EVENT_CALL_TERMINATED:
		case PHONE_EVENT_CALL_ESTABLISHED:
			helper->event(helper->phone, PHONE_EVENT_VIBRATOR_OFF);
			profiles->vibrator = 0;
			break;
		default: /* not relevant */
			break;
	}
	profiles->event = event;
	return 0;
}

static gboolean _event_call_incoming_timeout(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;

	if(profiles->vibrator != 0) /* vibrating with a pause */
	{
		if(profiles->vibrator++ == 1)
			helper->event(helper->phone, PHONE_EVENT_VIBRATOR_ON);
		else if((profiles->vibrator % 5) == 0)
		{
			helper->event(helper->phone, PHONE_EVENT_VIBRATOR_OFF);
			profiles->vibrator = 1;
		}
	}
	if(profiles->pao != NULL) /* playing a sample */
	{
		if(pa_operation_get_state(profiles->pao)
				== PA_OPERATION_RUNNING)
			return TRUE; /* check again later */
		pa_operation_unref(profiles->pao);
		/* ring again */
		profiles->pao = pa_context_play_sample(profiles->pac,
				"ringtone", NULL, PA_VOLUME_NORM, NULL, NULL);
	}
	return TRUE;
}
