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



#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
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


/* OSS */
/* private */
/* types */
typedef struct _OSS
{
	GtkWidget * window;
	GtkWidget * mixer;
	int fd;
} OSS;


/* prototypes */
static int _oss_init(PhonePlugin * plugin);
static int _oss_destroy(PhonePlugin * plugin);
static int _oss_event(PhonePlugin * plugin, PhoneEvent * event);
static int _oss_open(PhonePlugin * plugin);
static void _oss_settings(PhonePlugin * plugin);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Audio settings",
	"audio-x-generic",
	_oss_init,
	_oss_destroy,
	_oss_event,
	_oss_settings,
	NULL
};


/* private */
/* functions */
/* oss_init */
static int _oss_init(PhonePlugin * plugin)
{
	OSS * oss;

	if((oss = object_new(sizeof(*oss))) == NULL)
		return 1;
	plugin->priv = oss;
	oss->window = NULL;
	oss->fd = -1;
	_oss_open(plugin);
	return 0;
}


/* oss_destroy */
static int _oss_destroy(PhonePlugin * plugin)
{
	OSS * oss = plugin->priv;

	if(oss->fd >= 0)
		close(oss->fd);
	if(oss->window != NULL)
		gtk_widget_destroy(oss->window);
	free(oss);
	return 0;
}


/* oss_event */
static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event);
static int _event_volume_set(PhonePlugin * plugin, gdouble level);

static int _oss_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			return _event_modem_event(plugin,
					event->modem_event.event);
		case PHONE_EVENT_TYPE_SET_VOLUME:
			return _event_volume_set(plugin,
					event->volume_set.level);
		default: /* not relevant */
			break;
	}
	return 0;
}

static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event)
{
	ModemCallDirection direction;

	switch(event->type)
	{
		case MODEM_EVENT_TYPE_CALL:
			if(event->call.status != MODEM_CALL_STATUS_RINGING)
				break;
			direction = event->call.direction;
			if(direction == MODEM_CALL_DIRECTION_INCOMING)
				/* FIXME ringtone */
				break;
			else if(direction == MODEM_CALL_DIRECTION_OUTGOING)
				/* FIXME tone */
				break;
			break;
		default:
			break;
	}
	return 0;
}

static int _event_volume_set(PhonePlugin * plugin, gdouble level)
{
	int ret = 0;
	OSS * oss = plugin->priv;
	int v = level * 100;
	char buf[256];

	if(oss->fd < 0)
		return 1;
	v |= v << 8;
	if(ioctl(oss->fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &v) < 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", "MIXER_WRITE", strerror(
					errno));
		ret |= plugin->helper->error(plugin->helper->phone, buf, 0);
	}
	return ret;
}


/* oss_open */
static int _oss_open(PhonePlugin * plugin)
{
	OSS * oss = plugin->priv;
	char const * p;
	char buf[256];

	if(oss->fd >= 0)
		close(oss->fd);
	if((p = plugin->helper->config_get(plugin->helper->phone, "oss",
					"mixer")) == NULL)
		p = "/dev/mixer";
	if((oss->fd = open(p, O_RDWR)) < 0)
	{
		snprintf(buf, sizeof(buf), "%s: %s", p, strerror(errno));
		return plugin->helper->error(plugin->helper->phone, buf, 1);
	}
	return 0;
}


/* oss_settings */
static void _on_settings_cancel(gpointer data);
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_ok(gpointer data);

static void _oss_settings(PhonePlugin * plugin)
{
	OSS * oss = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;

	if(oss->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(oss->window));
		return;
	}
	oss->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(oss->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(oss->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(oss->window), "audio-x-generic");
#endif
	gtk_window_set_title(GTK_WINDOW(oss->window), "Sound preferences");
	g_signal_connect_swapped(G_OBJECT(oss->window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* device */
	widget = gtk_label_new("Mixer device:");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	widget = gtk_file_chooser_button_new("Set the mixer device",
			GTK_FILE_CHOOSER_ACTION_OPEN);
	oss->mixer = widget;
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
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
	gtk_container_add(GTK_CONTAINER(oss->window), vbox);
	_on_settings_cancel(plugin);
	gtk_widget_show_all(oss->window);
}

static void _on_settings_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	OSS * oss = plugin->priv;
	char const * p;

	if((p = plugin->helper->config_get(plugin->helper->phone, "oss",
					"mixer")) == NULL)
		p = "/dev/mixer";
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(oss->mixer), p);
	gtk_widget_hide(oss->window);
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
	OSS * oss = plugin->priv;
	char const * p;

	if((p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(oss->mixer)))
			!= NULL)
		plugin->helper->config_set(plugin->helper->phone, "oss",
				"mixer", p);
	gtk_widget_hide(oss->window);
	_oss_open(plugin);
}
