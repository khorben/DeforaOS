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
#include <gtk/gtk.h>
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
typedef enum _ProfileVolume
{
	PROFILE_VOLUME_SILENT = 0,
	PROFILE_VOLUME_25,
	PROFILE_VOLUME_50,
	PROFILE_VOLUME_75,
	PROFILE_VOLUME_100,
	PROFILE_VOLUME_ASC
} ProfileVolume;

typedef struct _ProfileDefinition
{
	char const * name;
	gboolean online;
	ProfileVolume volume;
	gboolean vibrate;
} ProfileDefinition;

typedef struct _Profiles
{
	guint source;

	/* profiles */
	ProfileDefinition * profiles;
	size_t profiles_cnt;
	size_t profiles_cur;

	/* vibrator */
	int vibrator;

	/* settings */
	GtkWidget * window;
	GtkWidget * combo;

	/* pulseaudio */
	pa_threaded_mainloop * pam;
	pa_context * pac;
	pa_operation * pao;
} Profiles;

/* variables */
static ProfileDefinition _profiles_definitions[] =
{
	{ "General",	TRUE,	PROFILE_VOLUME_ASC,	TRUE	},
	{ "Silent",	TRUE,	PROFILE_VOLUME_SILENT,	TRUE	},
	{ "Offline",	FALSE,	PROFILE_VOLUME_SILENT,	FALSE	}
};

/* prototypes */
static int _profiles_init(PhonePlugin * plugin);
static int _profiles_destroy(PhonePlugin * plugin);
static int _profiles_event(PhonePlugin * plugin, PhoneEvent event, ...);
static void _profiles_settings(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Profiles",
	"gnome-settings",
	_profiles_init,
	_profiles_destroy,
	_profiles_event,
	_profiles_settings,
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
	profiles->source = 0;
	profiles->profiles = _profiles_definitions;
	profiles->profiles_cnt = 3;
	profiles->profiles_cur = 0;
	profiles->vibrator = 0;
	profiles->window = NULL;
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
	/* XXX may already be online, may not be desired */
	plugin->helper->event(plugin->helper->phone, PHONE_EVENT_ONLINE);
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
static void _event_call_incoming_do(PhonePlugin * plugin);
static gboolean _event_call_incoming_timeout(gpointer data);

static int _profiles_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;

	switch(event)
	{
		case PHONE_EVENT_CALL_INCOMING:
			_event_call_incoming_do(plugin);
			break;
		case PHONE_EVENT_SMS_RECEIVED:
			if(profiles->pao == NULL)
				/* FIXME else queue the notification */
				profiles->pao = pa_context_play_sample(
						profiles->pac, "message", NULL,
						PA_VOLUME_NORM, NULL, NULL);
			break;
		case PHONE_EVENT_SIM_VALID:
		case PHONE_EVENT_SMS_SENT:
			/* FIXME beep in general profile? */
			break;
		case PHONE_EVENT_CALL_OUTGOING:
		case PHONE_EVENT_CALL_TERMINATED:
		case PHONE_EVENT_CALL_ESTABLISHED:
			helper->event(helper->phone, PHONE_EVENT_VIBRATOR_OFF);
			/* cancel the incoming call notification */
			if(profiles->source != 0)
				g_source_remove(profiles->source);
			profiles->source = 0;
			if(profiles->pao != NULL)
				pa_operation_cancel(profiles->pao);
			profiles->pao = NULL;
			profiles->vibrator = 0;
			break;
		default: /* not relevant */
			break;
	}
	return 0;
}

static void _event_call_incoming_do(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(profiles->pao == NULL)
		profiles->pao = pa_context_play_sample(profiles->pac,
				"ringtone", NULL, PA_VOLUME_NORM, NULL, NULL);
	if(profiles->vibrator == 0)
	{
		helper->event(helper->phone, PHONE_EVENT_VIBRATOR_ON);
		profiles->vibrator = 1;
	}
	if(profiles->source == 0)
		profiles->source = g_timeout_add(500,
				_event_call_incoming_timeout, plugin);
}

static gboolean _event_call_incoming_timeout(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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
	if(profiles->pao != NULL && pa_operation_get_state(profiles->pao)
			!= PA_OPERATION_RUNNING)
	{
		pa_operation_unref(profiles->pao);
		/* ring again */
		profiles->pao = pa_context_play_sample(profiles->pac,
				"ringtone", NULL, PA_VOLUME_NORM, NULL, NULL);
	}
	return TRUE;
}


/* profiles_settings */
static gboolean _on_profiles_closex(gpointer data);

static void _profiles_settings(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	GtkWidget * vbox;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, plugin->name);
#endif
	if(profiles->window == NULL)
	{
		profiles->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(profiles->window), 200,
				300);
		gtk_window_set_title(GTK_WINDOW(profiles->window), "Profiles");
		g_signal_connect_swapped(G_OBJECT(profiles->window),
				"delete-event", G_CALLBACK(_on_profiles_closex),
				profiles);
		vbox = gtk_vbox_new(FALSE, 0);
		/* entry */
		profiles->combo = gtk_combo_box_new_text();
		for(i = 0; i < profiles->profiles_cnt; i++)
			gtk_combo_box_append_text(GTK_COMBO_BOX(
						profiles->combo),
					profiles->profiles[i].name);
		gtk_combo_box_set_active(GTK_COMBO_BOX(profiles->combo),
				profiles->profiles_cur);
		gtk_box_pack_start(GTK_BOX(vbox), profiles->combo, FALSE, TRUE,
				0);
		gtk_container_add(GTK_CONTAINER(profiles->window), vbox);
		gtk_widget_show_all(vbox);
	}
	gtk_window_present(GTK_WINDOW(profiles->window));
}

static gboolean _on_profiles_closex(gpointer data)
{
	Profiles * profiles = data;

	gtk_widget_hide(profiles->window);
	return TRUE;
}
