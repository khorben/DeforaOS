/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
 * - add a pulseaudio version */



#include <sys/ioctl.h>
#if defined(__NetBSD__) || defined(__sun__)
# include <sys/audioio.h>
#elif defined(__linux__)
# include <alsa/asoundlib.h>
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
	"Volume",
	"stock_volume",
	_volume_init,
	_volume_destroy,
	NULL,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* types */
typedef struct _Volume
{
	char const * device;
	char const * control;
#if defined(AUDIO_MIXER_DEVINFO) /* audioio */
	int fd;
	int mix;
	int outputs;
#elif defined(SND_LIB_MAJOR) /* Alsa */
	snd_mixer_t * mixer;
	snd_mixer_elem_t * mixer_elem;
	long mixer_elem_max;
#else /* OSS */
	int fd;
#endif
	guint source;

	/* widgets */
	GtkWidget * widget;
} Volume;


/* prototypes */
static Volume * _volume_new(PanelApplet * applet);
static void _volume_delete(PanelApplet * applet);

static gdouble _volume_get(PanelApplet * applet);
static int _volume_set(PanelApplet * applet, gdouble value);
#ifdef AUDIO_MIXER_DEVINFO
static int _volume_match(PanelApplet * applet, mixer_devinfo_t * md);
#endif

/* callbacks */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data);
static gboolean _on_volume_timeout(gpointer data);


/* functions */
/* volume_init */
static GtkWidget * _volume_init(PanelApplet * applet)
{
#if GTK_CHECK_VERSION(2, 12, 0)
	Volume * volume;
	gdouble value;

	if((volume = _volume_new(applet)) == NULL)
		return NULL;
	applet->priv = volume;
	volume->widget = gtk_volume_button_new();
	g_object_set(G_OBJECT(volume->widget), "size",
			applet->helper->icon_size, NULL);
	if((value = _volume_get(applet)) >= 0.0)
	{
		gtk_scale_button_set_value(GTK_SCALE_BUTTON(volume->widget),
				value);
		_volume_set(applet, value);
	}
	g_signal_connect(G_OBJECT(volume->widget), "value-changed", G_CALLBACK(
				_on_value_changed), applet);
	gtk_widget_show(volume->widget);
	return volume->widget;
#else
	return NULL;
#endif
}


/* volume_destroy */
static void _volume_destroy(PanelApplet * applet)
{
	_volume_delete(applet);
}


/* volume_new */
static Volume * _volume_new(PanelApplet * applet)
{
	Volume * volume;
#if defined(AUDIO_MIXER_DEVINFO)
	int i;
	mixer_devinfo_t md;
#elif defined(SND_LIB_MAJOR)
	int err;
	snd_mixer_elem_t * elem = NULL;
	long min;
#endif

	if((volume = malloc(sizeof(*volume))) == NULL)
	{
		applet->helper->error(applet->helper->panel, "malloc", 0);
		return NULL;
	}
	volume->device = applet->helper->config_get(applet->helper->panel,
					"volume", "device");
	volume->control = applet->helper->config_get(applet->helper->panel,
			"volume", "control");
	volume->source = 0;
#if defined(AUDIO_MIXER_DEVINFO)
	if(volume->device == NULL)
		volume->device = "/dev/mixer";
	volume->mix = -1;
	volume->outputs = -1;
	if((volume->fd = open(volume->device, O_RDWR)) < 0)
	{
		applet->helper->error(applet->helper->panel, volume->device, 0);
		return volume;
	}
	for(i = 0; volume->outputs == -1 || volume->mix == -1; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type != AUDIO_MIXER_CLASS)
			continue;
		if(strcmp(md.label.name, AudioCoutputs) == 0)
			volume->outputs = i;
		else if(strcmp(md.label.name, "mix") == 0)
			volume->mix = i;
	}
	volume->source = g_timeout_add(500, _on_volume_timeout, applet);
#elif defined(SND_LIB_MAJOR)
	if(volume->device == NULL)
		volume->device = "hw:0";
	if(volume->control == NULL)
		volume->control = "Master";
	if((err = snd_mixer_open(&volume->mixer, 0)) != 0
			|| (err = snd_mixer_attach(volume->mixer,
				       	volume->device)) != 0
			|| (err = snd_mixer_selem_register(volume->mixer, NULL,
				       	NULL)) != 0
			|| (err = snd_mixer_load(volume->mixer)) != 0)
		fprintf(stderr, "%s: %s: %s\n", "Panel", volume->device,
			       	snd_strerror(err));
	else
		for(elem = snd_mixer_first_elem(volume->mixer); elem != NULL;
				elem = snd_mixer_elem_next(elem))
			if(strcmp(snd_mixer_selem_get_name(elem),
						volume->control) == 0)
				break;
	if((volume->mixer_elem = elem) != NULL
			&& snd_mixer_selem_get_playback_volume_range(
				volume->mixer_elem, &min,
			       	&volume->mixer_elem_max) == 0)
		volume->source = g_timeout_add(500, _on_volume_timeout, applet);
	else
		volume->mixer_elem = NULL;
#else
	if(volume->device == NULL)
		volume->device = "/dev/mixer";
	if((volume->fd = open(volume->device, O_RDWR)) < 0)
		applet->helper->error(applet->helper->panel, volume->device, 0);
	else
		volume->source = g_timeout_add(500, _on_volume_timeout, applet);
#endif
	return volume;
}


/* volume_delete */
static void _volume_delete(PanelApplet * applet)
{
	Volume * volume = applet->priv;

	if(volume->source != 0)
		g_source_remove(volume->source);
#if defined(AUDIO_MIXER_DEVINFO)
	if(volume->fd >= 0 && close(volume->fd) != 0)
		applet->helper->error(applet->helper->panel, volume->device, 0);
#elif defined(SND_LIB_MAJOR)
	if(volume->mixer != NULL)
		snd_mixer_close(volume->mixer);
#else /* XXX equivalent for now */
	if(volume->fd >= 0 && close(volume->fd) != 0)
		applet->helper->error(applet->helper->panel, volume->device, 0);
#endif
	free(volume);
}


/* accessors */
/* volume_get */
static gdouble _volume_get(PanelApplet * applet)
{
	Volume * volume = applet->priv;
	gdouble ret = -1.0;
#if defined(AUDIO_MIXER_DEVINFO)
	mixer_devinfo_t md;
	mixer_ctrl_t mc;
	int i;

	if(volume->fd < 0)
		return ret;
	if(volume->outputs < 0 && volume->mix < 0)
		return ret;
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
		{
			applet->helper->error(NULL, "AUDIO_MIXER_DEVINFO", 0);
			close(volume->fd);
			volume->fd = -1;
			break;
		}
		if(_volume_match(applet, &md) != 1)
			continue;
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = md.un.v.num_channels;
		if(ioctl(volume->fd, AUDIO_MIXER_READ, &mc) < 0)
			applet->helper->error(applet->helper->panel,
					"AUDIO_MIXER_READ", 0);
		else
			ret = mc.un.value.level[0] / 255.0;
		break;
	}
#elif defined(SND_LIB_MAJOR)
	long value;

	if(volume->mixer_elem == NULL)
		return ret;
	if(snd_mixer_selem_get_playback_volume(volume->mixer_elem,
				SND_MIXER_SCHN_FRONT_LEFT, &value) != 0)
		return ret;
	ret = value;
	ret /= volume->mixer_elem_max;
#else
	int value;

	if(volume->fd < 0)
		return ret;
	if(ioctl(volume->fd, MIXER_READ(SOUND_MIXER_VOLUME), &value) < 0)
	{
		applet->helper->error(NULL, "MIXER_READ", 0);
		close(volume->fd);
		volume->fd = -1;
	}
	else
		ret = ((value & 0xff) + ((value & 0xff00) >> 8)) / 200.0;
#endif
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %lf\n", __func__, ret);
#endif
	return ret;
}


/* volume_set */
int _volume_set(PanelApplet * applet, gdouble value)
{
	Volume * volume = applet->priv;
	int ret = 0;
#if defined(AUDIO_MIXER_DEVINFO)
	mixer_devinfo_t md;
	mixer_ctrl_t mc;
	int i;
	int j;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%lf)\n", __func__, value);
# endif
	if(volume->fd < 0)
		return 1;
	if(volume->outputs < 0 && volume->mix < 0)
		return 1;
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(volume->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		/* FIXME use volume->control */
		if(_volume_match(applet, &md) != 1)
			continue;
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = md.un.v.num_channels;
		mc.un.value.level[0] = value * 255;
		for(j = 1; j < mc.un.value.num_channels; j++) /* XXX overflow */
			mc.un.value.level[j] = mc.un.value.level[0];
		if(ioctl(volume->fd, AUDIO_MIXER_WRITE, &mc) < 0)
			ret |= applet->helper->error(applet->helper->panel,
					"AUDIO_MIXER_WRITE", 0);
		break;
	}
#elif defined(SND_LIB_MAJOR)
	long v = value * volume->mixer_elem_max;

	if(volume->mixer_elem == NULL)
		return 0;
	snd_mixer_selem_set_playback_volume_all(volume->mixer_elem, v);
#else
	int v = value * 100;

	if(volume->fd < 0)
		return 1;
	v |= v << 8;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%lf) 0x%04x\n", __func__, value, v);
# endif
	if(ioctl(volume->fd, MIXER_WRITE(SOUND_MIXER_VOLUME), &v) < 0)
		ret |= applet->helper->error(applet->helper->panel,
				"MIXER_WRITE", 0);
#endif
	return ret;
}


#if defined(AUDIO_MIXER_DEVINFO)
/* volume_match */
static int _volume_match(PanelApplet * applet, mixer_devinfo_t * md)
{
	Volume * volume = applet->priv;

	if(md->type != AUDIO_MIXER_VALUE)
		return 0;
	if(md->mixer_class == volume->mix /* mix.master */
			&& strcmp(md->label.name, "master") == 0)
		return 1;
	if(md->mixer_class == volume->outputs /* outputs.lineout */
			&& strcmp(md->label.name, "lineout") == 0)
		return 1;
	if(md->mixer_class == volume->outputs /* outputs.master */
			|| strcmp(md->label.name, "master") == 0)
		return 1;
	return 0;
}
#endif


/* callbacks */
/* on_value_changed */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data)
{
	PanelApplet * applet = data;

	_volume_set(applet, value);
}


/* on_volume_timeout */
static gboolean _on_volume_timeout(gpointer data)
{
	PanelApplet * applet = data;
	Volume * volume = applet->priv;
	gdouble value;

	if((value = _volume_get(applet)) < 0.0)
	{
		volume->source = 0;
		return FALSE;
	}
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(volume->widget), value);
	return TRUE;
}
