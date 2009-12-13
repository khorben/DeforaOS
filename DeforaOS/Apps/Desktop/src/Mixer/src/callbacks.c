/* $Id$ */
static char _copyright[] =
"Copyright (c) 2009 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Mixer */
static char _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#ifdef DEBUG
# include <stdio.h>
#endif
#ifdef __NetBSD__
# include <sys/audioio.h>
#endif
#include <Desktop.h>
#include "mixer.h"
#include "callbacks.h"
#include "../config.h"


/* private */
/* variables */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* public */
/* functions */
/* callbacks */
/* on_closex */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_main_quit();
	return TRUE;
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
	on_closex(NULL, NULL, data);
}


/* on_view_all */
void on_view_all(gpointer data)
{
	Mixer * mixer = data;

	mixer_show_all(mixer);
}


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


/* on_help_about */
void on_help_about(gpointer data)
{
	GtkWidget * about;

	about = desktop_about_dialog_new();
	/* FIXME make it transient */
	desktop_about_dialog_set_authors(about, _authors);
	desktop_about_dialog_set_copyright(about, _copyright);
	desktop_about_dialog_set_license(about, _license);
	desktop_about_dialog_set_logo_icon_name(about, "gnome-mixer");
	desktop_about_dialog_set_name(about, PACKAGE);
	desktop_about_dialog_set_version(about, VERSION);
	gtk_widget_show(about);
}


/* controls */
/* on_enum_toggled */
void on_enum_toggled(GtkWidget * widget, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* on_set_toggled */
void on_set_toggled(GtkWidget * widget, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
}


/* on_value_changed */
void on_value_changed(GtkWidget * widget, gdouble value, gpointer data)
{
	Mixer * mixer = data;

	mixer_set_value(mixer, widget, value);
}
