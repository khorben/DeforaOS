/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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



#ifndef MAILER_COMPOSE_CALLBACKS_H
# define MAILER_COMPOSE_CALLBACKS_H

# include <gtk/gtk.h>


/* compose window */
void on_compose_attach(gpointer data);
gboolean on_compose_closex(gpointer data);
void on_compose_save(gpointer data);
void on_compose_send(gpointer data);
void on_compose_attach(gpointer data);

/* file menu */
void on_compose_file_new(gpointer data);
void on_compose_file_save(gpointer data);
void on_compose_file_save_as(gpointer data);
void on_compose_file_send(gpointer data);
void on_compose_file_close(gpointer data);

/* edit menu */
void on_compose_edit_undo(gpointer data);
void on_compose_edit_redo(gpointer data);
void on_compose_edit_cut(gpointer data);
void on_compose_edit_copy(gpointer data);
void on_compose_edit_paste(gpointer data);
void on_compose_edit_select_all(gpointer data);

/* view menu */
void on_compose_view_add_field(gpointer data);

/* help menu */
void on_compose_help_about(gpointer data);

/* send mail */
gboolean on_send_closex(gpointer data);
void on_send_cancel(gpointer data);
gboolean on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data);

#endif /* !MAILER_COMPOSE_CALLBACKS_H */
