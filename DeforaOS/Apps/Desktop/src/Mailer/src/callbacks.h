/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MAILER_CALLBACKS_H
# define MAILER_CALLBACKS_H

# include <gtk/gtk.h>


/* mailer window */
gboolean on_closex(gpointer data);

/* file menu */
void on_file_new_mail(gpointer data);
void on_file_quit(gpointer data);

/* edit menu */
void on_edit_preferences(gpointer data);
void on_edit_select_all(gpointer data);
void on_edit_unselect_all(gpointer data);

/* message menu */
void on_message_reply(gpointer data);
void on_message_reply_to_all(gpointer data);
void on_message_forward(gpointer data);
void on_message_view_source(gpointer data);

/* help menu */
void on_help_about(gpointer data);

/* toolbar */
void on_new_mail(gpointer data);
void on_reply(gpointer data);
void on_reply_to_all(gpointer data);
void on_forward(gpointer data);
void on_preferences(gpointer data);

/* folder view */
void on_folder_change(GtkTreeSelection * selection, gpointer data);

/* header view */
void on_header_change(GtkTreeSelection * selection, gpointer data);


/* compose window */
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

/* view menu */
void on_compose_view_cc(gpointer data);
void on_compose_view_bcc(gpointer data);

/* help menu */
void on_compose_help_about(gpointer data);

/* send mail */
gboolean on_send_closex(gpointer data);
void on_send_cancel(gpointer data);
gboolean on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data);

#endif /* !MAILER_CALLBACKS_H */
