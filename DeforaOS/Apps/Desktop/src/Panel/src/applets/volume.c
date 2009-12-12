/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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
 * - add a timeout handler monitoring the volume if it changes */



#include <sys/ioctl.h>
#ifdef __NetBSD__
# include <sys/audioio.h>
#else
# include <sys/soundcard.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include "Panel.h"


/* Volume */
/* private */
/* prototypes */
static GtkWidget * _volume_init(PanelApplet * applet);
static void _volume_destroy(PanelApplet * applet);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_volume_init,
	_volume_destroy,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* types */
typedef struct _Volume
{
	PanelAppletHelper * helper;
#ifdef AUDIO_MIXER_DEVINFO
	int fd;
	int outputs;
#else
	int fd;
#endif
} Volume;


/* prototypes */
static Volume * _volume_new(PanelAppletHelper * helper);
static void _volume_delete(Volume * volume);

static gdouble _volume_get(Volume * volume);
static void _volume_set(Volume * volume, gdouble value);

/* callbacks */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data);


/* functions */
/* volume_init */
static GtkWidget * _volume_init(PanelApplet * applet)
{
#if GTK_CHECK_VERSION(2, 12, 0)
	GtkWidget * ret;
	Volume * volume;
	gdouble value;

	if((volume = _volume_new(applet->helper)) == NULL)
		return NULL;
	ret = gtk_volume_button_new();
	value = _volume_get(volume);
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(ret), value);
	_volume_set(volume, value);
	g_signal_connect(G_OBJECT(ret), "value-changed", G_CALLBACK(
				_on_value_changed), volume);
	return ret;
#else
	return NULL;
#endif
}


/* volume_destroy */
static void _volume_destroy(PanelApplet * applet)
{
	Volume * volume = applet->priv;

	_volume_delete(volume);
}


/* volume_new */
static Volume * _volume_new(PanelAppletHelper * helper)
{
	Volume * volume;
#ifdef AUDIO_MIXER_DEVINFO
	int i;
	mixer_devinfo_t md;
#endif

	if((volume = malloc(sizeof(*volume))) == NULL)
	{
		helper->error(helper->priv, "malloc", 0);
		return NULL;
	}
	volume->helper = helper;
#ifdef AUDIO_MIXER_DEVINFO
	volume->outputs = -1;
	if((volume->fd = open("/dev/mixer", O_RDWR)) < 0)
	{
		helper->error(helper->priv, "/dev/mixer", 0);
		return volume;
	}
	for(i = 0; volume->outputs == -1; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type != AUDIO_MIXER_CLASS
				|| strcmp(md.label.name, AudioCoutputs) != 0)
			continue;
		volume->outputs = i;
	}
#else
	if((volume->fd = open("/dev/mixer", O_RDWR)) < 0)
		helper->error(helper->priv, "/dev/mixer", 0);
#endif
	return volume;
}


/* volume_delete */
static void _volume_delete(Volume * volume)
{
#ifdef AUDIO_MIXER_DEVINFO
	if(volume->fd >= 0 && close(volume->fd) != 0)
		volume->helper->error(volume->helper->priv, "/dev/mixer", 0);
#else /* XXX equivalent for now */
	if(volume->fd >= 0 && close(volume->fd) != 0)
		volume->helper->error(volume->helper->priv, "/dev/mixer", 0);
#endif
	free(volume);
}


/* accessors */
/* volume_get */
static gdouble _volume_get(Volume * volume)
{
	gdouble ret = 0.0;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_devinfo_t md;
	mixer_ctrl_t mc;
	int i;

	if(volume->outputs < 0)
		return ret;
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
		{
			volume->helper->error(volume->helper->priv,
					"AUDIO_MIXER_DEVINFO", 0);
			break;
		}
		if(md.mixer_class != volume->outputs
				|| md.type != AUDIO_MIXER_VALUE)
			continue;
		if(strcmp(md.label.name, "lineout") != 0
				&& strcmp(md.label.name, "master") != 0)
			continue;
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = md.un.v.num_channels;
		if(ioctl(volume->fd, AUDIO_MIXER_READ, &mc) < 0)
			volume->helper->error(volume->helper->priv,
					"AUDIO_MIXER_READ", 0);
		else
			ret = mc.un.value.level[0] / 255.0;
		break;
	}
#else
	int value;

	if(ioctl(volume->fd, MIXER_READ(SOUND_MIXER_VOLUME), &value) < 0)
		volume->helper->error(volume->helper->priv, "MIXER_READ", 0);
	else
		ret = ((value & 0xff) + ((value & 0xff00) >> 8)) / 200.0;
#endif
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %lf\n", __func__, ret);
#endif
	return ret;
}


/* volume_set */
void _volume_set(Volume * volume, gdouble value)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_devinfo_t md;
	mixer_ctrl_t mc;
	int i;
	int j;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%lf)\n", __func__, value);
# endif
	if(volume->outputs < 0)
		return;
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.mixer_class != volume->outputs
				|| md.type != AUDIO_MIXER_VALUE)
			continue;
		if(strcmp(md.label.name, "lineout") != 0
				&& strcmp(md.label.name, "master") != 0)
			continue;
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = md.un.v.num_channels;
		mc.un.value.level[0] = value * 255;
		for(j = 1; j < mc.un.value.num_channels; j++) /* XXX overflow */
			mc.un.value.level[j] = mc.un.value.level[0];
		if(ioctl(volume->fd, AUDIO_MIXER_WRITE, &mc) < 0)
			volume->helper->error(volume->helper->priv,
					"AUDIO_MIXER_WRITE", 0);
		break;
	}
#else
	int v = value * 100;

	v |= v << 8;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%lf) 0x%04x\n", __func__, value, v);
# endif
	if(ioctl(volume->fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &v) < 0)
		volume->helper->error(volume->helper->priv, "MIXER_WRITE", 0);
#endif
}


/* callbacks */
/* on_value_changed */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data)
{
	Volume * volume = data;

	_volume_set(volume, value);
}
