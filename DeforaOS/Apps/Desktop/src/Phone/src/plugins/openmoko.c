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
 * - implement "mute" with the mixer (GSM Line Out)
 * - register a handler for deep sleep (just to avoid unknown errors)
 * - implement notification lights */



#include <fcntl.h>
#include <unistd.h>
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
#include "hayes.h"
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

	/* hardware support */
	GtkWidget * hw_bluetooth;
	GtkWidget * hw_gps;

#ifdef __linux__
	/* alsa support */
	snd_mixer_t * mixer;
	snd_mixer_elem_t * mixer_elem;
	snd_mixer_elem_t * mixer_elem_headphone;
	snd_mixer_elem_t * mixer_elem_speaker;
#endif
} Openmoko;


/* prototypes */
/* plug-in */
static int _openmoko_init(PhonePlugin * plugin);
static void _openmoko_destroy(PhonePlugin * plugin);
static int _openmoko_event(PhonePlugin * plugin, PhoneEvent * event);
static void _openmoko_deepsleep(PhonePlugin * plugin);
static void _openmoko_queue(PhonePlugin * plugin, char const * command);
static void _openmoko_settings(PhonePlugin * plugin);

static int _openmoko_get_state(PhonePlugin * plugin, char const * device,
		gboolean * enabled);
static int _openmoko_set_state(PhonePlugin * plugin, char const * device,
		gboolean enabled);

static int _openmoko_mixer_open(PhonePlugin * plugin);
static int _openmoko_mixer_close(PhonePlugin * plugin);
static int _openmoko_power(PhonePlugin * plugin, gboolean power);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Openmoko",
	"phone-openmoko",
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
	_openmoko_power(plugin, TRUE);
	return 0;
}


/* openmoko_destroy */
static void _openmoko_destroy(PhonePlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;

	_openmoko_mixer_close(plugin);
	if(openmoko->window != NULL)
		gtk_widget_destroy(openmoko->window);
	object_delete(openmoko);
}


/* openmoko_event */
static int _event_mixer_set(PhonePlugin * plugin, char const * filename);
static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event);
static int _event_vibrator(PhonePlugin * plugin, gboolean vibrate);
static int _event_volume_get(PhonePlugin * plugin, gdouble * level);
static int _event_volume_set(PhonePlugin * plugin, gdouble level);

static int _openmoko_event(PhonePlugin * plugin, PhoneEvent * event)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;
#endif

	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			_event_modem_event(plugin, event->modem_event.event);
			break;
		case PHONE_EVENT_TYPE_NOTIFICATION_OFF:
			/* FIXME implement */
			break;
		case PHONE_EVENT_TYPE_NOTIFICATION_ON:
			/* FIXME implement */
			break;
		case PHONE_EVENT_TYPE_STARTED:
			_openmoko_power(plugin, TRUE);
			break;
		case PHONE_EVENT_TYPE_STOPPED:
			_openmoko_power(plugin, FALSE);
			break;
		case PHONE_EVENT_TYPE_RESUME:
			/* FIXME implement in Hayes plug-in if possible */
			_openmoko_queue(plugin, "AT+CTZU=1");
			_openmoko_queue(plugin, "AT+CTZR=1");
			_openmoko_queue(plugin, "AT+CREG=2");
			_openmoko_queue(plugin, "AT+CGEREP=2,1");
#if 0 /* XXX not enabled in the first place */
			_openmoko_queue(plugin, "AT%CSQ=1");
			_openmoko_queue(plugin, "AT%CPRI=1");
			_openmoko_queue(plugin, "AT%CNIV=1");
#endif
			break;
		case PHONE_EVENT_TYPE_SPEAKER_ON:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set(plugin, "gsmspeakerout.state");
#ifdef __linux__
			openmoko->mixer_elem = openmoko->mixer_elem_headphone;
#endif
			break;
		case PHONE_EVENT_TYPE_SPEAKER_OFF:
			/* XXX assumes there's an ongoing call */
			_event_mixer_set(plugin, "gsmhandset.state");
#ifdef __linux__
			openmoko->mixer_elem = openmoko->mixer_elem_speaker;
#endif
			break;
		case PHONE_EVENT_TYPE_SUSPEND:
			_openmoko_queue(plugin, "AT+CTZU=0");
			_openmoko_queue(plugin, "AT+CTZR=0");
			_openmoko_queue(plugin, "AT+CREG=0");
			_openmoko_queue(plugin, "AT+CGEREP=0,0");
#if 0 /* XXX not enabled in the first place */
			_openmoko_queue(plugin, "AT%CSQ=0");
			_openmoko_queue(plugin, "AT%CPRI=0");
			_openmoko_queue(plugin, "AT%CNIV=0");
			_openmoko_queue(plugin, "AT%CBHZ=0");
#endif
			break;
		case PHONE_EVENT_TYPE_VIBRATOR_OFF:
			_event_vibrator(plugin, FALSE);
			break;
		case PHONE_EVENT_TYPE_VIBRATOR_ON:
			_event_vibrator(plugin, TRUE);
			break;
		case PHONE_EVENT_TYPE_VOLUME_GET:
			_event_volume_get(plugin, &event->volume_get.level);
			break;
		case PHONE_EVENT_TYPE_VOLUME_SET:
			_event_volume_set(plugin, event->volume_set.level);
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

static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event)
{
	char const * profile = "stereoout.state";

	switch(event->type)
	{
		case MODEM_EVENT_TYPE_CALL:
			if(event->call.status == MODEM_CALL_STATUS_ACTIVE)
				profile = "gsmhandset.state";
			else if(event->call.status == MODEM_CALL_STATUS_RINGING
					&& event->call.direction
					== MODEM_CALL_DIRECTION_OUTGOING)
				profile = "gsmhandset.state";
			_event_mixer_set(plugin, profile);
			/* enable echo cancellation */
			_openmoko_queue(plugin, "AT%N0187");
			break;
		case MODEM_EVENT_TYPE_STATUS:
			if(event->status.status == MODEM_STATUS_ONLINE)
				_openmoko_deepsleep(plugin);
			break;
		default:
			break;
	}
	return 0;
}
static int _event_vibrator(PhonePlugin * plugin, gboolean vibrate)
{
	int ret = 0;
	char const p1[] = "/sys/class/leds/gta02::vibrator/brightness";
	char const p2[] = "/sys/class/leds/neo1973:vibrator/brightness";
	char const * path = p1;
	int fd;
	char buf[256];
	int len;

	if((fd = open(path, O_WRONLY)) < 0)
	{
		path = p2;
		fd = open(path, O_WRONLY);
	}
	if(fd < 0)
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

static int _event_volume_get(PhonePlugin * plugin, gdouble * level)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;
	long min;
	long max;
	long value;

	if(openmoko->mixer_elem == NULL)
		return 0;
	if(snd_mixer_selem_get_playback_volume_range(openmoko->mixer_elem,
				&min, &max) != 0)
		return 0;
	if(snd_mixer_selem_get_playback_volume(openmoko->mixer_elem,
				SND_MIXER_SCHN_FRONT_LEFT, &value) != 0)
		return 0;
	*level = value;
	*level /= max;
#endif
	return 0;
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
	PhonePluginHelper * helper = plugin->helper;
	char const * cmd = "AT%SLEEP=4"; /* allow deep sleep */
	char const * p;

	if((p = helper->config_get(helper->phone, "openmoko", "deepsleep"))
			!= NULL && strtoul(p, NULL, 10) != 0)
		cmd = "AT%SLEEP=2"; /* prevent deep sleep */
	/* XXX may reset the hardware modem */
	_openmoko_queue(plugin, cmd);
	_openmoko_queue(plugin, "AT+CPIN?");
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


/* openmoko_power */
static int _openmoko_power(PhonePlugin * plugin, gboolean power)
{
	int ret = 0;
	char const p1[] = "/sys/bus/platform/devices/gta02-pm-gsm.0/power_on";
	char const p2[] = "/sys/bus/platform/devices/neo1973-pm-gsm.0/power_on";
	char const * path = p1;
	int fd;
	char buf[256];

	if((fd = open(path, O_WRONLY)) < 0)
	{
		path = p2;
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


/* openmoko_get_state */
static int _openmoko_get_state(PhonePlugin * plugin, char const * device,
                gboolean * enabled)
{
	int ret = -1;
	int fd;
	char buf[2];

	if((fd = open(device, O_RDONLY)) < 0)
		return plugin->helper->error(NULL, strerror(errno), 1);
	if(read(fd, &buf, sizeof(buf)) == 2)
	{
		if(buf[0] == '1')
			*enabled = TRUE;
		else if(buf[0] == '0')
			*enabled = FALSE;
		ret = 0;
	}
	close(fd);
	return ret;
}


/* openmoko_set_state */
static int _openmoko_set_state(PhonePlugin * plugin, char const * device,
                gboolean enabled)
{
	int ret = -1;
	int fd;
	char buf[2] = "\0\0";

	if((fd = open(device, O_WRONLY)) < 0)
		return plugin->helper->error(NULL, strerror(errno), 1);
	buf[0] = enabled ? '1' : '0';
	if(write(fd, buf, sizeof(buf)) == sizeof(buf))
		ret = 0;
	close(fd);
	return ret;
}


/* openmoko_mixer_open */
static int _openmoko_mixer_open(PhonePlugin * plugin)
{
#ifdef __linux__
	Openmoko * openmoko = plugin->priv;
	char const * audio_device;
	snd_mixer_elem_t * elem;

	openmoko->mixer_elem = NULL;
	openmoko->mixer_elem_headphone = NULL;
	openmoko->mixer_elem_speaker = NULL;
	if((audio_device = plugin->helper->config_get(plugin->helper->phone,
					"openmoko", "audio_device")) == NULL)
		audio_device = "hw:0";
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
		if(strcmp(snd_mixer_selem_get_name(elem), "Headphone") == 0
				&& snd_mixer_selem_has_playback_volume(elem))
			openmoko->mixer_elem_headphone = elem;
		else if(strcmp(snd_mixer_selem_get_name(elem), "Speaker") == 0
				&& snd_mixer_selem_has_playback_volume(elem))
			openmoko->mixer_elem_speaker = elem;
#endif /* __linux__ */
	return 0;
}


/* openmoko_queue */
static void _openmoko_queue(PhonePlugin * plugin, char const * command)
{
	ModemRequest request;
	HayesRequest hrequest;

	request.type = MODEM_REQUEST_UNSUPPORTED;
	request.unsupported.modem = "Hayes";
	request.unsupported.request_type = HAYES_REQUEST_COMMAND_QUEUE;
	request.unsupported.request = &hrequest;
	request.unsupported.size = sizeof(hrequest);
	hrequest.command_queue.command = command;
	plugin->helper->request(plugin->helper->phone, &request);
}


/* openmoko_settings */
static void _settings_on_apply(gpointer data);
static void _settings_on_cancel(gpointer data);
static gboolean _settings_on_closex(gpointer data);
static void _settings_on_ok(gpointer data);
static void _settings_on_toggled(GtkWidget * widget);

static void _openmoko_settings(PhonePlugin * plugin)
{
	Openmoko * openmoko = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * frame;
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
	g_signal_connect_swapped(openmoko->window, "delete-event", G_CALLBACK(
				_settings_on_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* check button */
	openmoko->deepsleep = gtk_check_button_new_with_label(
			"Prevent deep sleep");
	gtk_box_pack_start(GTK_BOX(vbox), openmoko->deepsleep, FALSE, TRUE, 0);
	/* hardware */
	frame = gtk_frame_new("Hardware");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 4);
	bbox = gtk_vbox_new(TRUE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);
	/* bluetooth */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Bluetooth");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	openmoko->hw_bluetooth = gtk_toggle_button_new_with_label("OFF");
	g_signal_connect(openmoko->hw_bluetooth, "toggled", G_CALLBACK(
				_settings_on_toggled), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), openmoko->hw_bluetooth, FALSE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(bbox), hbox, FALSE, TRUE, 0);
	/* GPS */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("GPS");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	openmoko->hw_gps = gtk_toggle_button_new_with_label("OFF");
	g_signal_connect(openmoko->hw_gps, "toggled", G_CALLBACK(
				_settings_on_toggled), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), openmoko->hw_gps, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), bbox);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	/* button box */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_settings_on_cancel), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_settings_on_apply), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(_settings_on_ok),
			plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(openmoko->window), vbox);
	_settings_on_cancel(plugin);
	gtk_widget_show_all(openmoko->window);
}

static void _settings_on_apply(gpointer data)
{
	PhonePlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;
	char const bt1[] = "/sys/bus/platform/devices/gta02-pm-bt.0/power_on";
	char const bt2[] = "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on";
	char const gps1[] = "/sys/bus/platform/devices/gta02-pm-gps.0/power_on";
	char const gps2[] = "/sys/bus/platform/devices/neo1973-pm-gps.0/"
		"power_on";
	char const gps3[] = "/sys/bus/platform/drivers/neo1973-pm-gps/"
		"neo1973-pm-gps.0/pwron";
	gboolean value;

	/* deepsleep */
	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				openmoko->deepsleep));
	plugin->helper->config_set(plugin->helper->phone, "openmoko",
			"deepsleep", value ? "1" : "0");
	_openmoko_deepsleep(plugin);
	/* hardware */
	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				openmoko->hw_bluetooth));
	if(_openmoko_set_state(plugin, bt1, value) != 0)
		_openmoko_set_state(plugin, bt2, value);
	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				openmoko->hw_gps));
	if(_openmoko_set_state(plugin, gps1, value) != 0
			|| _openmoko_set_state(plugin, gps2, value) != 0)
		_openmoko_set_state(plugin, gps3, value);
}

static void _settings_on_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;
	char const bt1[] = "/sys/bus/platform/devices/gta02-pm-bt.0/power_on";
	char const bt2[] = "/sys/bus/platform/devices/neo1973-pm-bt.0/power_on";
	char const gps1[] = "/sys/bus/platform/devices/gta02-pm-gps.0/power_on";
	char const gps2[] = "/sys/bus/platform/devices/neo1973-pm-gps.0/"
		"power_on";
	char const gps3[] = "/sys/bus/platform/drivers/neo1973-pm-gps/"
		"neo1973-pm-gps.0/pwron";
	gboolean enabled;
	char const * p;

	gtk_widget_hide(openmoko->window);
	/* deepsleep */
	if((p = plugin->helper->config_get(plugin->helper->phone, "openmoko",
					"deepsleep")) != NULL
			&& strtoul(p, NULL, 10) != 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->deepsleep), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->deepsleep), FALSE);
	/* hardware */
	if(_openmoko_get_state(plugin, bt1, &enabled) != 0
			&& _openmoko_get_state(plugin, bt2, &enabled) != 0)
		gtk_widget_set_sensitive(openmoko->hw_bluetooth, FALSE);
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->hw_bluetooth), enabled);
		gtk_widget_set_sensitive(openmoko->hw_bluetooth, TRUE);
	}
	if(_openmoko_get_state(plugin, gps1, &enabled) != 0
			&& _openmoko_get_state(plugin, gps2, &enabled) != 0
			&& _openmoko_get_state(plugin, gps3, &enabled) != 0)
		gtk_widget_set_sensitive(openmoko->hw_gps, FALSE);
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
					openmoko->hw_gps), enabled);
		gtk_widget_set_sensitive(openmoko->hw_gps, TRUE);
	}
}

static gboolean _settings_on_closex(gpointer data)
{
	PhonePlugin * plugin = data;

	_settings_on_cancel(plugin);
	return TRUE;
}

static void _settings_on_ok(gpointer data)
{
	PhonePlugin * plugin = data;
	Openmoko * openmoko = plugin->priv;

	gtk_widget_hide(openmoko->window);
	_settings_on_apply(plugin);
}

static void _settings_on_toggled(GtkWidget * widget)
{
	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_button_set_label(GTK_BUTTON(widget), active ? "ON" : "OFF");
}
