/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mixer */
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
#include <sys/audioio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "mixer.h"
#include "../config.h"


/* Mixer */
/* private */
/* types */
struct _Mixer
{
	int fd;

	/* widgets */
	GtkWidget * window;
	GtkWidget * hbox[6];
};


/* variables */
static DesktopMenu _mixer_menu_file[] =
{
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _mixer_menu_view[] =
{
	{ "_All", G_CALLBACK(on_view_all), NULL, 0 },
	{ "_Outputs", G_CALLBACK(on_view_outputs), NULL, 0 },
	{ "_Inputs", G_CALLBACK(on_view_inputs), NULL, 0 },
	{ "_Record", G_CALLBACK(on_view_record), NULL, 0 },
	{ "_Monitor", G_CALLBACK(on_view_monitor), NULL, 0 },
	{ "_Equalization", G_CALLBACK(on_view_equalization), NULL, 0 },
	{ "_Modem", G_CALLBACK(on_view_modem), NULL, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _mixer_menu_help[] =
{
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenubar _mixer_menubar[] =
{
	{ "_File", _mixer_menu_file },
	{ "_View", _mixer_menu_view },
	{ "_Help", _mixer_menu_help },
	{ NULL, NULL },
};


/* public */
/* mixer_new */
static GtkWidget * _new_enum(Mixer * mixer, struct audio_mixer_enum * e);
static GtkWidget * _new_set(Mixer * mixer, struct audio_mixer_set * s);
static GtkWidget * _new_value(Mixer * mixer, struct audio_mixer_value * v);

Mixer * mixer_new(void)
{
	Mixer * mixer;
	GtkWidget * scrolled;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * hbox;
	GtkWidget * control;
	size_t u;
	int i;
	mixer_devinfo_t md;

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	mixer->fd = open("/dev/mixer", O_RDWR);
	mixer->window = NULL;
	if(mixer->fd < 0)
	{
		mixer_delete(mixer);
		return NULL;
	}
	/* widgets */
	mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(mixer->window), 800, 200);
	gtk_window_set_title(GTK_WINDOW(mixer->window), PACKAGE);
	g_signal_connect(G_OBJECT(mixer->window), "delete-event", G_CALLBACK(
				on_closex), mixer);
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
			GTK_SHADOW_NONE);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	widget = desktop_menubar_create(_mixer_menubar, mixer, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* controls */
	hbox = gtk_hbox_new(FALSE, 0);
	for(u = 0; u < sizeof(mixer->hbox) / sizeof(*mixer->hbox); u++)
	{
		mixer->hbox[u] = gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), mixer->hbox[u], TRUE, TRUE,
				0);
	}
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled),
			hbox);
	for(i = 0;; i++)
	{
		md.index = i;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.mixer_class < 0)
			continue;
		u = md.mixer_class;
		if(u >= sizeof(mixer->hbox) / sizeof(*mixer->hbox))
			continue;
		hbox = mixer->hbox[md.mixer_class];
		control = NULL;
		switch(md.type)
		{
			case AUDIO_MIXER_ENUM:
				control = _new_enum(mixer, &md.un.e);
				break;
			case AUDIO_MIXER_SET:
				control = _new_set(mixer, &md.un.s);
				break;
			case AUDIO_MIXER_VALUE:
				control = _new_value(mixer, &md.un.v);
				break;
		}
		if(control == NULL)
			continue;
		widget = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(widget), gtk_label_new(
					md.label.name), FALSE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(widget), control, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	}
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mixer->window), vbox);
	gtk_widget_show_all(vbox);
	mixer_show(mixer, 0);
	gtk_widget_show(mixer->window);
	return mixer;
}

static GtkWidget * _new_enum(Mixer * mixer, struct audio_mixer_enum * e)
{
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	GSList * group = NULL;

	if(e->num_mem <= 0)
		return NULL;
	vbox = gtk_vbox_new(FALSE, 0);
	for(i = 0; i < e->num_mem; i++)
	{
		widget = gtk_radio_button_new_with_label(group,
				e->member[i].label.name);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_set(Mixer * mixer, struct audio_mixer_set * s)
{
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;

	if(s->num_mem <= 0)
		return NULL;
	vbox = gtk_vbox_new(FALSE, 0);
	for(i = 0; i < s->num_mem; i++)
	{
		widget = gtk_check_button_new_with_label(
				s->member[i].label.name);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_value(Mixer * mixer, struct audio_mixer_value * v)
{
	GtkWidget * hbox;
	int i;
	GtkWidget * widget;

	if(v->num_channels <= 0)
		return NULL;
	hbox = gtk_hbox_new(FALSE, 0);
	for(i = 0; i < v->num_channels; i++)
	{
		widget = gtk_vscale_new_with_range(0.0, 1.0, 0.02);
		g_signal_connect(G_OBJECT(widget), "value-changed", G_CALLBACK(
					on_value_changed), mixer);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	}
	return hbox;
}


/* mixer_delete */
void mixer_delete(Mixer * mixer)
{
	if(mixer->fd >= 0)
		close(mixer->fd);
	if(mixer->window != NULL)
		gtk_widget_destroy(mixer->window);
	free(mixer);
}


/* useful */
/* mixer_show */
void mixer_show(Mixer * mixer, int view)
{
	size_t u;

	if(view < 0)
	{
		for(u = 0; u < sizeof(mixer->hbox) / sizeof(*mixer->hbox); u++)
			gtk_widget_show(mixer->hbox[u]);
		return;
	}
	u = view;
	if(u >= sizeof(mixer->hbox) / sizeof(*mixer->hbox))
		return;
	for(u = 0; u < sizeof(mixer->hbox) / sizeof(*mixer->hbox); u++)
		gtk_widget_hide(mixer->hbox[u]);
	gtk_widget_show(mixer->hbox[view]);
}


/* mixer_show_all */
void mixer_show_all(Mixer * mixer)
{
	mixer_show(mixer, -1);
}
