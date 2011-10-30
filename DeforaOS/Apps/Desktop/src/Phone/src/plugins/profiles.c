/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
 * - move PulseAudio support in a dedicated plug-in */



#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <pulse/pulseaudio.h>
#include <System.h>
#include "Phone.h"
#include "../../config.h"

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


/* Profiles */
/* private */
/* types */
typedef enum _ProfileType
{
	PROFILE_TYPE_GENERAL = 0,
	PROFILE_TYPE_SILENT,
	PROFILE_TYPE_OFFLINE
} ProfileType;
#define PROFILE_TYPE_LAST PROFILE_TYPE_OFFLINE
#define PROFILE_TYPE_COUNT (PROFILE_TYPE_LAST + 1)

typedef enum _ProfileVolume
{
	PROFILE_VOLUME_SILENT	= 0,
	PROFILE_VOLUME_25	= 25,
	PROFILE_VOLUME_50	= 50,
	PROFILE_VOLUME_75	= 75,
	PROFILE_VOLUME_100	= 100,
	PROFILE_VOLUME_ASC	= -1
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
	GtkWidget * pr_window;
	GtkWidget * pr_combo;
	GtkWidget * pr_online;
	GtkWidget * pr_volume;
	GtkWidget * pr_vibrator;

	/* pulseaudio */
	pa_threaded_mainloop * pam;
	pa_context * pac;
	pa_operation * pao;
} Profiles;

/* variables */
static ProfileDefinition _profiles_definitions[PROFILE_TYPE_COUNT] =
{
	{ "General",	TRUE,	PROFILE_VOLUME_ASC,	TRUE	},
	{ "Silent",	TRUE,	PROFILE_VOLUME_SILENT,	TRUE	},
	{ "Offline",	FALSE,	PROFILE_VOLUME_SILENT,	FALSE	}
};

/* prototypes */
/* plug-in */
static int _profiles_init(PhonePlugin * plugin);
static void _profiles_destroy(PhonePlugin * plugin);
static int _profiles_event(PhonePlugin * plugin, PhoneEvent * event);
static void _profiles_settings(PhonePlugin * plugin);

/* useful */
static void _profiles_apply(PhonePlugin * plugin, ProfileType type);
static void _profiles_switch(PhonePlugin * plugin, ProfileType type);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Profiles",
	"system-config-users",
	_profiles_init,
	_profiles_destroy,
	_profiles_event,
	_profiles_settings,
	NULL
};


/* private */
/* functions */
/* profiles_init */
#if 0
static gboolean _init_idle(gpointer data);
#endif

static int _profiles_init(PhonePlugin * plugin)
{
	Profiles * profiles;
	pa_mainloop_api * mapi = NULL;
	char const * p;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((profiles = object_new(sizeof(*profiles))) == NULL)
		return -1;
	plugin->priv = profiles;
	profiles->source = 0;
	profiles->profiles = _profiles_definitions;
	profiles->profiles_cnt = sizeof(_profiles_definitions)
		/ sizeof(*_profiles_definitions);
	profiles->profiles_cur = 0;
	if((p = plugin->helper->config_get(plugin->helper->phone, "profiles",
					"default")) != NULL)
		for(i = 0; i < profiles->profiles_cnt; i++)
			if(strcmp(profiles->profiles[i].name, p) == 0)
			{
				profiles->profiles_cur = i;
				break;
			}
	profiles->vibrator = 0;
	profiles->pr_window = NULL;
	profiles->pam = pa_threaded_mainloop_new();
	profiles->pac = NULL;
	profiles->pao = NULL;
	if(profiles->pam == NULL)
	{
		_profiles_destroy(plugin);
		return error_set_code(1, "%s",
				"Could not initialize PulseAudio");
	}
	mapi = pa_threaded_mainloop_get_api(profiles->pam);
	/* XXX update the context name */
	if((profiles->pac = pa_context_new(mapi, PACKAGE)) == NULL)
	{
		_profiles_destroy(plugin);
		return error_set_code(1, "%s",
				"Could not initialize PulseAudio");
	}
	pa_context_connect(profiles->pac, NULL, 0, NULL);
	pa_threaded_mainloop_start(profiles->pam);
#if 0
	profiles->source = g_idle_add(_init_idle, plugin);
#endif
	return 0;
}

#if 0
static gboolean _init_idle(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	PhoneEvent event;

	/* FIXME may already be online, may not be desired */
	/* FIXME ask to go online if currently offline */
	event.type = PHONE_EVENT_TYPE_ONLINE;
	plugin->helper->event(plugin->helper->phone, &event);
	profiles->source = 0;
	return FALSE;
}
#endif


/* profiles_destroy */
static void _profiles_destroy(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(profiles->source != 0)
		g_source_remove(profiles->source);
	if(profiles->pr_window != NULL)
		gtk_widget_destroy(profiles->pr_window);
	if(profiles->pao != NULL)
		pa_operation_cancel(profiles->pao);
	if(profiles->pac != NULL)
		pa_context_unref(profiles->pac);
	pa_threaded_mainloop_free(profiles->pam);
	object_delete(profiles);
}


/* profiles_event */
static int _event_key_tone(PhonePlugin * plugin);
static int _event_starting(PhonePlugin * plugin);
static int _event_stopping(PhonePlugin * plugin);
#if 0
static void _event_call_incoming_do(PhonePlugin * plugin);
static gboolean _event_call_incoming_timeout(gpointer data);
#endif

static int _profiles_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		/* FIXME implement again */
#if 0
		case PHONE_EVENT_TYPE_CALL_INCOMING:
			_event_call_incoming_do(plugin);
			break;
#endif
		case PHONE_EVENT_TYPE_KEY_TONE:
			return _event_key_tone(plugin);
		case PHONE_EVENT_TYPE_STARTING:
			return _event_starting(plugin);
		case PHONE_EVENT_TYPE_STOPPING:
			return _event_stopping(plugin);
#if 0
		case PHONE_EVENT_TYPE_SMS_RECEIVED:
			if(profiles->pao == NULL)
				/* FIXME else queue the notification */
				profiles->pao = pa_context_play_sample(
						profiles->pac, "message", NULL,
						PA_VOLUME_NORM, NULL, NULL);
			break;
		case PHONE_EVENT_TYPE_SIM_PIN_VALID:
		case PHONE_EVENT_TYPE_SMS_SENT:
			/* FIXME beep in general profile? */
			break;
		case PHONE_EVENT_TYPE_CALL_OUTGOING:
		case PHONE_EVENT_TYPE_CALL_TERMINATED:
		case PHONE_EVENT_TYPE_CALL_ESTABLISHED:
			helper->event(helper->phone, PHONE_EVENT_TYPE_VIBRATOR_OFF);
			/* cancel the incoming call notification */
			if(profiles->source != 0)
				g_source_remove(profiles->source);
			profiles->source = 0;
			if(profiles->pao != NULL)
				pa_operation_cancel(profiles->pao);
			profiles->pao = NULL;
			profiles->vibrator = 0;
			break;
#endif
		default: /* not relevant */
			break;
	}
	return 0;
}

static int _event_key_tone(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	ProfileDefinition * definition = &profiles->profiles[
		profiles->profiles_cur];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(definition->volume != PROFILE_VOLUME_SILENT && profiles->pao == NULL)
		profiles->pao = pa_context_play_sample(profiles->pac,
				"keytone", NULL, PA_VOLUME_NORM, NULL, NULL);
	return 0;
}

static int _event_starting(PhonePlugin * plugin)
{
	PhonePluginHelper * helper = plugin->helper;
	Profiles * profiles = plugin->priv;
	ProfileDefinition * definition = &profiles->profiles[
		profiles->profiles_cur];

	if(definition->online)
		return 0;
	if(helper->confirm(helper->phone, "You are currently offline.\n"
				"Do you want to go online?") == 0)
		return 1;
	_profiles_switch(plugin, 0);
	return 0;
}

static int _event_stopping(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	ProfileDefinition * definition = &profiles->profiles[
		profiles->profiles_cur];

	/* prevent stopping the modem except if we're going offline */
	return definition->online ? 1 : 0;
}

#if 0
static void _event_call_incoming_do(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;
	ProfileDefinition * definition = &profiles->profiles[
		profiles->profiles_cur];
	PhoneEvent event;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(definition->volume != PROFILE_VOLUME_SILENT && profiles->pao == NULL)
		profiles->pao = pa_context_play_sample(profiles->pac,
				"ringtone", NULL, PA_VOLUME_NORM, NULL, NULL);
	if(definition->vibrate && profiles->vibrator == 0)
	{
		event.type = PHONE_EVENT_TYPE_VIBRATOR_ON;
		helper->event(helper->phone, &event);
		profiles->vibrator = 1;
	}
	if(profiles->source == 0)
		profiles->source = g_timeout_add(500,
				_event_call_incoming_timeout, plugin);
}
#endif

#if 0
static gboolean _event_call_incoming_timeout(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	PhonePluginHelper * helper = plugin->helper;
	PhoneEvent event;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(profiles->vibrator != 0) /* vibrating with a pause */
	{
		if(profiles->vibrator++ == 1)
		{
			event.type = PHONE_EVENT_TYPE_VIBRATOR_ON;
			helper->event(helper->phone, &event);
		}
		else if((profiles->vibrator % 5) == 0)
		{
			event.type = PHONE_EVENT_TYPE_VIBRATOR_OFF;
			helper->event(helper->phone, &event);
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
#endif


/* profiles_settings */
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_cancel(gpointer data);
static void _on_settings_changed(gpointer data);
static void _on_settings_ok(gpointer data);

static void _profiles_settings(PhonePlugin * plugin)
{
	Profiles * profiles = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * frame;
	GtkWidget * bbox;
	GtkWidget * widget;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, plugin->name);
#endif
	if(profiles->pr_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(profiles->pr_window));
		return;
	}
	profiles->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(profiles->pr_window), 4);
	gtk_window_set_default_size(GTK_WINDOW(profiles->pr_window), 200, 300);
	gtk_window_set_title(GTK_WINDOW(profiles->pr_window), "Profiles");
	g_signal_connect_swapped(G_OBJECT(profiles->pr_window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* combo */
	profiles->pr_combo = gtk_combo_box_new_text();
	for(i = 0; i < profiles->profiles_cnt; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(profiles->pr_combo),
				profiles->profiles[i].name);
	g_signal_connect_swapped(profiles->pr_combo, "changed", G_CALLBACK(
				_on_settings_changed), plugin);
	gtk_box_pack_start(GTK_BOX(vbox), profiles->pr_combo, FALSE, TRUE, 0);
	/* frame */
	frame = gtk_frame_new("Overview");
	widget = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 4);
	profiles->pr_online = gtk_check_button_new_with_label("Online");
	gtk_widget_set_sensitive(profiles->pr_online, FALSE);
	gtk_box_pack_start(GTK_BOX(widget), profiles->pr_online, FALSE, TRUE,
			0);
	bbox = gtk_hbox_new(FALSE, 4);
	profiles->pr_volume = gtk_label_new("Volume: ");
	gtk_widget_set_sensitive(profiles->pr_volume, FALSE);
	gtk_box_pack_start(GTK_BOX(bbox), profiles->pr_volume, FALSE, TRUE, 0);
	profiles->pr_volume = gtk_progress_bar_new();
	gtk_widget_set_sensitive(profiles->pr_volume, FALSE);
	gtk_box_pack_start(GTK_BOX(bbox), profiles->pr_volume, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(widget), bbox, FALSE, TRUE, 0);
	profiles->pr_vibrator = gtk_check_button_new_with_label("Vibrate");
	gtk_widget_set_sensitive(profiles->pr_vibrator, FALSE);
	gtk_box_pack_start(GTK_BOX(widget), profiles->pr_vibrator, FALSE, TRUE,
			0);
	gtk_container_add(GTK_CONTAINER(frame), widget);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	/* dialog */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_cancel), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_ok), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(profiles->pr_window), vbox);
	gtk_widget_show_all(vbox);
	_on_settings_cancel(plugin);
	gtk_window_present(GTK_WINDOW(profiles->pr_window));
}

static gboolean _on_settings_closex(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;

	_on_settings_cancel(profiles);
	return TRUE;
}

static void _on_settings_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;

	gtk_widget_hide(profiles->pr_window);
	gtk_combo_box_set_active(GTK_COMBO_BOX(profiles->pr_combo),
			profiles->profiles_cur);
}

static void _on_settings_changed(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	int i;
	char buf[16];
	double fraction;

	i = gtk_combo_box_get_active(GTK_COMBO_BOX(profiles->pr_combo));
	if(i < 0)
		return;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(profiles->pr_online),
			_profiles_definitions[i].online);
	fraction = _profiles_definitions[i].volume;
	if(_profiles_definitions[i].volume > 0)
		snprintf(buf, sizeof(buf), "%u %%",
				_profiles_definitions[i].volume);
	else if(_profiles_definitions[i].volume == 0)
		snprintf(buf, sizeof(buf), "%s", "Silent");
	else
	{
		snprintf(buf, sizeof(buf), "%s", "Ascending");
		fraction = 0.0;
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(profiles->pr_volume),
			fraction / 100.0);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(profiles->pr_volume), buf);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(profiles->pr_vibrator),
			_profiles_definitions[i].vibrate);
}

static void _on_settings_ok(gpointer data)
{
	PhonePlugin * plugin = data;
	Profiles * profiles = plugin->priv;
	ProfileType type;

	gtk_widget_hide(profiles->pr_window);
	type = gtk_combo_box_get_active(GTK_COMBO_BOX(profiles->pr_combo));
	_profiles_switch(plugin, type);
}


/* profiles_apply */
static void _profiles_apply(PhonePlugin * plugin, ProfileType type)
{
	PhonePluginHelper * helper = plugin->helper;
	Profiles * profiles = plugin->priv;

	if(type > profiles->profiles_cnt)
		/* XXX report error */
		return;
	profiles->profiles_cur = type;
	helper->config_set(helper->phone, "profiles", "default",
			profiles->profiles[profiles->profiles_cur].name);
}


/* profiles_switch */
static void _profiles_switch(PhonePlugin * plugin, ProfileType type)
{
	PhonePluginHelper * helper = plugin->helper;
	Profiles * profiles = plugin->priv;
	ProfileType current = profiles->profiles_cur;
	PhoneEvent pevent;

	if(type == current)
		return;
	if(type > profiles->profiles_cnt)
		/* XXX report error */
		return;
	_profiles_apply(plugin, type);
	memset(&pevent, 0, sizeof(pevent));
	if(profiles->profiles[current].online
			&& !profiles->profiles[type].online)
	{
		/* go offline */
		pevent.type = PHONE_EVENT_TYPE_STOPPING;
		helper->event(helper->phone, &pevent);
	}
	else if(!profiles->profiles[current].online
			&& profiles->profiles[type].online)
	{
		/* go online */
		pevent.type = PHONE_EVENT_TYPE_STARTING;
		helper->event(helper->phone, &pevent);
	}
}
