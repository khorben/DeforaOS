/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Editor */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. */



#ifndef EDITOR_CALLBACKS_H
# define EDITOR_CALLBACKS_H

# include <gtk/gtk.h>


/* functions */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
void on_edit_preferences(GtkWidget * widget, gpointer data);
void on_file_close(GtkWidget * widget, gpointer data);
void on_file_new(GtkWidget * widget, gpointer data);
void on_file_open(GtkWidget * widget, gpointer data);
void on_file_save(GtkWidget * widget, gpointer data);
void on_file_save_as(GtkWidget * widget, gpointer data);
void on_help_about(GtkWidget * widget, gpointer data);

/* toolbar */
void on_new(GtkWidget * widget, gpointer data);
void on_open(GtkWidget * widget, gpointer data);
void on_save(GtkWidget * widget, gpointer data);
void on_save_as(GtkWidget * widget, gpointer data);
# ifdef EMBEDDED
void on_preferences(GtkWidget * widget, gpointer data);
# endif

#endif /* !EDITOR_CALLBACKS_H */
