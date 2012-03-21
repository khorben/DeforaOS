/* $Id$ */
/* Copyright (c) 2009-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MIXER_CALLBACKS_H
# define MIXER_CALLBACKS_H

# include <gtk/gtk.h>


/* callbacks */
gboolean on_closex(gpointer data);
void on_embedded(gpointer data);

/* menubar */
void on_file_properties(gpointer data);
void on_file_close(gpointer data);
void on_view_all(gpointer data);
void on_view_outputs(gpointer data);
void on_view_inputs(gpointer data);
void on_view_record(gpointer data);
void on_view_monitor(gpointer data);
void on_view_equalization(gpointer data);
void on_view_mix(gpointer data);
void on_view_modem(gpointer data);
void on_help_about(gpointer data);

/* controls */
void on_enum_toggled(GtkWidget * widget, gpointer data);
void on_mute_toggled(GtkWidget * widget, gpointer data);
void on_set_toggled(GtkWidget * widget, gpointer data);
void on_value_changed(GtkWidget * widget, gpointer data);

#endif /* !MIXER_CALLBACKS_H */
