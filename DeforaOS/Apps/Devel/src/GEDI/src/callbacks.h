/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
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



#ifndef GEDI_CALLBACKS_H
# define GEDI_CALLBACKS_H

# include <gtk/gtk.h>


/* callbacks */
void on_exit(GtkWidget * widget, gpointer data);
void on_help_about(GtkWidget * widget, gpointer data);
void on_file_new(GtkWidget * widget, gpointer data);
void on_file_open(GtkWidget * widget, gpointer data);
void on_file_preferences(GtkWidget * widget, gpointer data);
void on_preferences_apply(GtkWidget * widget, gpointer data);
void on_preferences_cancel(GtkWidget * widget, gpointer data);
void on_preferences_ok(GtkWidget * widget, gpointer data);
gboolean on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
void on_project_new(GtkWidget * widget, gpointer data);
void on_project_open(GtkWidget * widget, gpointer data);
void on_project_properties(GtkWidget * widget, gpointer data);
void on_project_save(GtkWidget * widget, gpointer data);
void on_project_save_as(GtkWidget * widget, gpointer data);
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

#endif /* !GEDI_CALLBACKS_H */
