/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
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



#ifndef TODO_CALLBACKS_H
# define TODO_CALLBACKS_H

# include <gtk/gtk.h>


/* callbacks */
gboolean on_closex(gpointer data);

/* file menu */
void on_file_close(gpointer data);

/* edit menu */
void on_edit_select_all(gpointer data);
void on_edit_delete(gpointer data);

/* help menu */
void on_help_about(gpointer data);

#endif /* !TODO_CALLBACKS_H */
