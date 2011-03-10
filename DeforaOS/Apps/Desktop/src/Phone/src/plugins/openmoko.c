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
 * - add an icon
 * - register a handler for deep sleep (just to avoid unknown errors)
 * - implement notification lights */



#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <System.h>
#ifdef __linux__
# include <alsa/asoundlib.h>
#endif
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
/* types */
typedef struct _Openmoko
{
	GtkWidget * window;
	GtkWidget * deepsleep;
#ifdef __linux__

	/* alsa support */
	snd_mixer_t * mixer;
	snd_mixer_elem_t * mixer_elem;
#endif
} Openmoko;


/* prototypes */
/* plugins */
static int _openmoko_init(PhonePlugin * plugin);
static int _openmoko_destroy(PhonePlugin * plugin);
static int _openmoko_event(PhonePlugin * plugin, PhoneEvent event, ...);
static void _openmoko_deepsleep(PhonePlugin * plugin);
static void _openmoko_settings(PhonePlugin * plugin);

static int _openmoko_mixer_open(PhonePlugin * plugin);
static int _openmoko_mixer_close(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Openmoko",
	"stock_cell-phone",
	_openmoko_init,
	_openmoko_destroy,
	_openmoko_event,
	_openmoko_settings,
	NULL
};


/* private */
/* functions */
/* openmoko_init */
static int _openmoko_init(PhonePlugin * plugin)
{
	Openmoko * openmoko;

	if((openmoko = object_new(sizeof(*openmoko))) == NULL)
		return 1;
	plugin->priv = openmoko;
	openmoko->window = NULL;
	_openmoko_mixer_open(plugin);
	return 0;
}


/* openmoko_destroy */
static int _openmoko_destroy(PhonePlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;

	_openmoko_mixer_close(plugin);
	if(openmoko->window != NULL)
		gtk_widget_destroy(openmoko->window);
	object_delete(openmoko);
	return 0;
}


/* openmoko_event */
static int _event_mixer_set(PhonePlugin * plugin, char const * filename);
static int _event_power_on(PhonePlugin * plugin, gboolean power);
static int _event_resuming(PhonePlugin * plugin);
static int _event_suspend(PhonePlugin * plugin);
static int _event_vibrator(PhonePlugin * plugin, gboolean vibrate);
static int _event_volume_set(PhonePlugin * plugin, gdouble level);

static int _openmoko_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	va_list ap;
	gdouble level;

	switch(event)
	{
		case PHONE_EVENT_CALL_ESTABLISHED:
			/* let us hear the call */
			_event_mixer_set(plugin, "gsmhandset.state");
			/* enable echo cancellation */
			plugin->helper->queue(plugin->helper->phone,
					"AT%N0187");
			break;
		case PHONE_EVENT_CALL_INCOMING:
			/* let us hear the ringtone */
			_event_mixer_set(plugin, "stereoout.state");
			break;
		case PHONE_EVENT_CALL_OUTGOING:
			/* let us hear the connection */
			_event_mixer_set(plugin, "gsmhandset.state");
			break;
		case PHONE_EVENT_CALL_TERMINATED:
			/* restore regular audio */
			_event_mixer_set(plugin, "stereoout.state");
			break;
		case PHONE_EVENT_FUNCTIONAL:
			_openmoko_deepsleep(plugin);
			break;
		case PHONE_EVENT_NOTIFICATION_OFF:
			/* FIXME implement */
			break;
		case PHONE_EVENT_NOTIFICATION_ON:
			/* FIXME implement */
			break;
		case PHONE_EVENT_OFFLINE:
			_event_power_on(plugin, FALSE);
			break;
		case PHONE_EVENT_ONLINE:
			_event_power_on(plugin, TRUE);
			break;
		case PHONE_EVENT_RESUMING:
			_event_resuming(plugin);
			break;
		case PHONE_EVENT_SET_VOLUME:
			va_start(ap, event);
			level = va_arg(ap, gdouble);
			va_end(ap);
			_event_volume_set(plugin, level);
			break;
		case PHONE_EVENT_SPEAKER_ON:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set(plugin, "gsmspeakerout.state");
			break;
		case PHONE_EVENT_SPEAKER_OFF:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set(plugin, "gsmhandset.state");
			break;
		case PHONE_EVENT_SUSPEND:
			_event_suspend(plugin);
			break;
		case PHONE_EVENT_VIBRATOR_OFF:
			_event_vibrator(plugin, FALSE);
			break;
		case PHONE_EVENT_VIBRATOR_ON:
			_event_vibrator(plugin, TRUE);
			break;
		default: /* not relevant */
			break;
	}
	return 0;
}

static int _event_mixer_set(PhonePlugin * plugin, char const * filename)
{
	int ret = 0;
	char const scenarios[] = DATADIR "/openmoko/scenarios";
	char * pathname;
	size_t len;
	char * alsactl[] = { SBINDIR "/alsactl", "alsactl", "-f", NULL,
		"restore", NULL };
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	len = sizeof(scenarios) + 1 + strlen(filename);
	if((pathname = malloc(len)) == NULL)
		return plugin->helper->error(NULL, strerror(errno), 1);
	snprintf(pathname, len, "%s/%s", scenarios, filename);
	alsactl[3] = pathname;
	if(g_spawn_async(NULL, alsactl, NULL, G_SPAWN_FILE_AND_ARGV_ZERO,
				NULL, NULL, NULL, &error) == FALSE)
		ret = plugin->helper->error(NULL, error->message, 1);
	free(pathname);
	return ret;
}

static int _event_power_on(PhonePlugin * plugin, gboolean power)
{
	int ret = 0;
	char const path1[] = "/sys/bus/platform/drivers/gta02-pm-gsm"
		"/gta02-pm-gsm.0/power_on";
	char const path2[] = "/sys/bus/platform/drivers/neo1973-pm-gsm"
		"/neo1973-pm-gsm.0/power_on";
	char const * path = path1;
	int fd;
	char buf[256];

	if((fd = open(path, O_WRONLY)) < 0)
	{
		path = path2;
		fd = open(path, O_WRONLY);
	}
	if(fd < 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", path, strerror(errno));
		return plugin->helper->error(NULL, buf, 1);
	}
	if(write(fd, power ? "1\n" : "0\n", 2) != 2)
	{
		snprintf(buf, sizeof(buf), "%s: %s", path, strerror(errno));
		ret = plugin->helper->error(NULL, buf, 1);
	}
	if(close(fd) != 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", path, strerror(errno));
		ret = plugin->helper->error(NULL, buf, 1);
	}
	return ret;
}

static int _event_resuming(PhonePlugin * plugin)
{
	plugin->helper->queue(plugin->helper->phone, "AT+CTZU=1");
	plugin->helper->queue(plugin->helper->phone, "AT+CTZR=1");
	plugin->helper->queue(plugin->helper->phone, "AT+CREG=2");
	plugin->helper->queue(plugin->helper->phone, "AT+CGEREP=2,1");
#if 0 /* XXX not enabled in the first place */
	plugin->helper->queue(plugin->helper->phone, "AT%CSQ=1");
	plugin->helper->queue(plugin->helper->phone, "AT%CPRI=1");
	plugin->helper->queue(plugin->helper->phone, "AT%CNIV=1");
#endif
	return 0;
}

static int _event_suspend(PhonePlugin * plugin)
{
	plugin->helper->queue(plugin->helper->phone, "AT+CTZU=0");
	plugin->helper->queue(plugin->helper->phone, "AT+CTZR=0");
	plugin->helper->queue(plugin->helper->phone, "AT+CREG=0");
	plugin->helper->queue(plugin->helper->phone, "AT+CGEREP=0,0");
#if 0 /* XXX not enabled in the first place */
	plugin->helper->queue(plugin->helper->phone, "AT%CSQ=0");
	plugin->helper->queue(plugin->helper->phone, "AT%CPRI=0");
	plugin->helper->queue(plugin->helper->phone, "AT%CNIV=0");
	plugin->helper->queue(plugin->helper->phone, "AT%CBHZ=0");
#endif
	return 0;
}

static int _event_vibrator(PhonePlugin * plugin, gboolean vibrate)
{
	int ret = 0;
	char const path[] = "/sys/class/leds/neo1973:vibrator/brightness";
	int fd;
	char buf[256];
	int len;

	if((fd = open(path, O_WRONLY)) < 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", path, strerror(errno));
		return plugin->helper->error(NULL, buf, 1);
	}
	if((len = snprintf(buf, sizeof(buf), "%d", vibrate ? 255 : 0)) > 0
			&& write(fd, buf, len) != len)
	{
		snprintf(buf, sizeof(buf), "%s: %s", path, strerror(errno));
		ret = plugin->helper->error(NULL, buf, 1);
	}
	close(fd);
	return ret;
}

static int _event_volume_set(PhonePlugin * plugin, gdouble level)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;

	if(openmoko->mixer_elem == NULL)
		return 0;
	snd_mixer_selem_set_playback_volume_all(openmoko->mixer_elem, level);
#endif
	return 0;
}


/* openmoko_deepsleep */
static void _openmoko_deepsleep(PhonePlugin * plugin)
{
	char const * cmd = "AT%SLEEP=4"; /* allow deep sleep */
	char const * p;

	if((p = plugin->helper->config_get(plugin->helper->phone, "openmoko",
					"deepsleep")) != NULL
			&& strtoul(p, NULL, 10) != 0)
		cmd = "AT%SLEEP=2"; /* prevent deep sleep */
	/* XXX may reset the hardware modem */
	plugin->helper->queue(plugin->helper->phone, cmd);
	plugin->helper->queue(plugin->helper->phone, "AT+CPIN?");
}


/* openmoko_mixer_close */
static int _openmoko_mixer_close(PhonePlugin * plugin)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;

	openmoko->mixer_elem = NULL;
	if(openmoko->mixer != NULL)
		snd_mixer_close(openmoko->mixer);
	openmoko->mixer = NULL;
#endif /* __linux__ */
	return 0;
}


/* openmoko_mixer_open */
static int _openmoko_mixer_open(PhonePlugin * plugin)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;
	char const * audio_device;
	char const * audio_control;
	snd_mixer_elem_t * elem;

	openmoko->mixer_elem = NULL;
	if((audio_device = plugin->helper->config_get(plugin->helper->phone,
					"openmoko", "audio_device")) == NULL)
		audio_device = "neo1973-gta02";
	if((audio_control = plugin->helper->config_get(plugin->helper->phone,
					"openmoko", "audio_control")) == NULL)
		audio_control = "Speaker";
	if(snd_mixer_open(&openmoko->mixer, 0) != 0)
	{
		openmoko->mixer = NULL;
		return -1;
	}
	if(snd_mixer_attach(openmoko->mixer, audio_device) != 0
			|| snd_mixer_selem_register(openmoko->mixer, NULL, NULL)
			|| snd_mixer_load(openmoko->mixer) != 0)
	{
		_openmoko_mixer_close(plugin);
		return -1;
	}
	for(elem = snd_mixer_first_elem(openmoko->mixer); elem != NULL;
			elem = snd_mixer_elem_next(elem))
		if(strcmp(snd_mixer_selem_get_name(elem), audio_control) == 0)
			break;
	if(elem == NULL)
	{
		_openmoko_mixer_close(plugin);
		return -1;
	}
	openmoko->mixer_elem = elem;
#endif /* __linux__ */
	return 0;
}


/* openmoko_settings */
static void _on_settings_cancel(gpointer data);
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_ok(gpointer data);

static void _openmoko_settings(PhonePlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;

	if(openmoko->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(openmoko->window));
		return;
	}
	openmoko->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(openmoko->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(openmoko->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(openmoko->window),
			"gnome-settings");
#endif
	gtk_window_set_title(GTK_WINDOW(openmoko->window),
			"Openmoko preferences");
	g_signal_connect_swapped(G_OBJECT(openmoko->window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* check button */
	openmoko->deepsleep = gtk_check_button_new_with_label(
			"Prevent deep sleep");
	gtk_box_pack_start(GTK_BOX(vbox), openmoko->deepsleep, FALSE, TRUE, 0);
	/* button box */
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
	gtk_container_add(GTK_CONTAINER(openmoko->window), vbox);
	_on_settings_cancel(plugin);
	gtk_widget_show_all(openmoko->window);
}

static void _on_settings_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;
	char const * p;

	if((p = plugin->helper->config_get(plugin->helper->phone, "openmoko",
					"deepsleep")) != NULL
			&& strtoul(p, NULL, 10) != 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->deepsleep), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->deepsleep), FALSE);
	gtk_widget_hide(openmoko->window);
}

static gboolean _on_settings_closex(gpointer data)
{
	PhonePlugin * plugin = data;

	_on_settings_cancel(plugin);
	return TRUE;
}

static void _on_settings_ok(gpointer data)
{
	PhonePlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;
	gboolean value;

	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				openmoko->deepsleep));
	plugin->helper->config_set(plugin->helper->phone, "openmoko",
			"deepsleep", value ? "1" : "0");
	_openmoko_deepsleep(plugin);
	gtk_widget_hide(openmoko->window);
}
