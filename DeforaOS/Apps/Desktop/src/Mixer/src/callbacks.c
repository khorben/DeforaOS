/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifdef DEBUG
# include <stdio.h>
#endif
#if defined(__NetBSD__) || defined(__sun__)
# include <sys/audioio.h>
#endif
#include <Desktop.h>
#include "mixer.h"
#include "callbacks.h"
#include "../config.h"


/* public */
/* functions */
/* callbacks */
/* on_closex */
gboolean on_closex(gpointer data)
{
	gtk_main_quit();
	return TRUE;
}


/* on_embedded */
void on_embedded(gpointer data)
{
	Mixer * mixer = data;

	mixer_show(mixer);
}


/* file menu */
/* on_file_properties */
void on_file_properties(gpointer data)
{
	Mixer * mixer = data;

	mixer_properties(mixer);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	on_closex(data);
}


/* on_view_all */
void on_view_all(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_all(mixer);
}


#ifdef AUDIO_MIXER_DEVINFO
/* on_view_outputs */
void on_view_outputs(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCoutputs);
}


/* on_view_inputs */
void on_view_inputs(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCinputs);
}


/* on_view_record */
void on_view_record(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCrecord);
}


/* on_view_monitor */
void on_view_monitor(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCmonitor);
}


/* on_view_equalization */
void on_view_equalization(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCequalization);
}


/* on_view_mix */
void on_view_mix(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, "mix");
}


/* on_view_modem */
void on_view_modem(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_class(mixer, AudioCmodem);
}
#endif


/* on_help_about */
void on_help_about(gpointer data)
{
	Mixer * mixer = data;

	mixer_about(mixer);
}


/* controls */
/* on_enum_toggled */
void on_enum_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == TRUE)
		mixer_set_enum(mixer, widget);
}


/* on_mute_toggled */
void on_mute_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

	mixer_set_mute(mixer, widget);
}


/* on_set_toggled */
void on_set_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	mixer_set_set(mixer, widget);
}


/* on_value_changed */
void on_value_changed(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;
	gdouble value;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %lf, %p)\n", __func__, (void*)widget,
			value, (void*)data);
#endif
	value = gtk_range_get_value(GTK_RANGE(widget));
	mixer_set_value(mixer, widget, value);
}
