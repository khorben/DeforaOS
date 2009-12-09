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



#ifdef __NetBSD__ /* XXX make it more portable */
# include <sys/audioio.h>
#else
# warning Unsupported platform
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "Panel.h"


/* Volume */
/* private */
/* prototypes */
static GtkWidget * _volume_init(PanelApplet * applet);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_volume_init,
	NULL,
	PANEL_APPLET_POSITION_END,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* prototypes */
/* callbacks */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data);


/* functions */
/* volume_init */
static GtkWidget * _volume_init(PanelApplet * applet)
{
	GtkWidget * ret;

	ret = gtk_volume_button_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, "Master volume");
#endif
	/* FIXME set it to the current value first */
	g_signal_connect(G_OBJECT(ret), "value-changed", G_CALLBACK(
				_on_value_changed), NULL);
	return ret;
}


/* callbacks */
/* on_value_changed */
static void _on_value_changed(GtkWidget * widget, gdouble value, gpointer data)
{
#ifdef AUDIO_MIXER_DEVINFO
	int fd;
	mixer_devinfo_t md;
	mixer_ctrl_t mc;
	int i;
	int j;
	int outputs = -1;

	if((fd = open("/dev/mixer", O_RDWR)) < 0)
	{
		perror("/dev/mixer");
		return;
	}
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type == AUDIO_MIXER_CLASS)
		{
			if(strcmp(md.label.name, "outputs") == 0)
				outputs = i;
			continue;
		}
		if(md.mixer_class != outputs || md.type != AUDIO_MIXER_VALUE)
			continue;
		if(strcmp(md.label.name, "lineout") != 0)
			continue;
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = md.un.v.num_channels;
		mc.un.value.level[0] = gtk_scale_button_get_value(
				GTK_SCALE_BUTTON(widget)) * 255;
		for(j = 1; j < mc.un.value.num_channels; j++)
			mc.un.value.level[j] = mc.un.value.level[0];
		mc.un.value.level[1] = mc.un.value.level[0];
		if(ioctl(fd, AUDIO_MIXER_WRITE, &mc) < 0)
			perror("AUDIO_MIXER_WRITE");
		break;
	}
	close(fd);
#endif
}
