/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop XMLEditor */
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



#ifndef XMLEDITOR_CALLBACKS_H
# define XMLEDITOR_CALLBACKS_H

# include <gtk/gtk.h>


/* functions */
gboolean on_closex(gpointer data);
void on_edit_preferences(gpointer data);
void on_file_close(gpointer data);
void on_file_new(gpointer data);
void on_file_open(gpointer data);
void on_file_save(gpointer data);
void on_file_save_as(gpointer data);
void on_help_about(gpointer data);
void on_view_collapse_all(gpointer data);
void on_view_expand_all(gpointer data);

/* toolbar */
void on_close(gpointer data);
void on_new(gpointer data);
void on_open(gpointer data);
void on_save(gpointer data);
void on_save_as(gpointer data);
void on_preferences(gpointer data);

/* view */
void on_tag_name_edited(GtkCellRendererText * renderer, gchar * path,
		gchar * name, gpointer data);

#endif /* !XMLEDITOR_CALLBACKS_H */
