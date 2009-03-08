/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* Mailer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Mailer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef MAILER_CALLBACKS_H
# define MAILER_CALLBACKS_H

# include <gtk/gtk.h>


/* mailer window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

/* file menu */
void on_file_new_mail(GtkWidget * widget, gpointer data);
void on_file_send_receive(GtkWidget * widget, gpointer data);
void on_file_print(GtkWidget * widget, gpointer data);
void on_file_print_preview(GtkWidget * widget, gpointer data);
void on_file_quit(GtkWidget * widget, gpointer data);

/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data);

/* message menu */
void on_message_reply(GtkWidget * widget, gpointer data);
void on_message_reply_to_all(GtkWidget * widget, gpointer data);
void on_message_forward(GtkWidget * widget, gpointer data);
void on_message_delete(GtkWidget * widget, gpointer data);
void on_message_view_source(GtkWidget * widget, gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);

/* toolbar */
void on_new_mail(GtkWidget * widget, gpointer data);
void on_stop(GtkWidget * widget, gpointer data);
void on_reply(GtkWidget * widget, gpointer data);
void on_reply_to_all(GtkWidget * widget, gpointer data);
void on_forward(GtkWidget * widget, gpointer data);
void on_delete(GtkWidget * widget, gpointer data);
void on_print(GtkWidget * widget, gpointer data);

/* folder view */
void on_folder_change(GtkTreeSelection * selection, gpointer data);

/* header view */
void on_header_change(GtkTreeSelection * selection, gpointer data);

/* preferences window */
void on_preferences_ok(GtkWidget * widget, gpointer data);
void on_preferences_cancel(GtkWidget * widget, gpointer data);


/* accounts */
void on_account_new(GtkWidget * widget, gpointer data);
void on_account_edit(GtkWidget * widget, gpointer data);
void on_account_delete(GtkWidget * widget, gpointer data);


/* compose window */
gboolean on_compose_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
void on_compose_save(GtkWidget * widget, gpointer data);
void on_compose_send(GtkWidget * widget, gpointer data);
void on_compose_attach(GtkWidget * widget, gpointer data);

/* file menu */
void on_compose_file_new(GtkWidget * widget, gpointer data);
void on_compose_file_save(GtkWidget * widget, gpointer data);
void on_compose_file_save_as(GtkWidget * widget, gpointer data);
void on_compose_file_send(GtkWidget * widget, gpointer data);
void on_compose_file_close(GtkWidget * widget, gpointer data);

/* edit menu */
void on_compose_edit_undo(GtkWidget * widget, gpointer data);
void on_compose_edit_redo(GtkWidget * widget, gpointer data);
void on_compose_edit_cut(GtkWidget * widget, gpointer data);
void on_compose_edit_copy(GtkWidget * widget, gpointer data);
void on_compose_edit_paste(GtkWidget * widget, gpointer data);

/* view menu */
void on_compose_view_cc(GtkWidget * widget, gpointer data);
void on_compose_view_bcc(GtkWidget * widget, gpointer data);

/* help menu */
void on_compose_help_about(GtkWidget * widget, gpointer data);

/* send mail */
gboolean on_send_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
void on_send_cancel(GtkWidget * widget, gpointer data);
gboolean on_send_write(GIOChannel * source, GIOCondition condition,
		gpointer data);

#endif /* !MAILER_CALLBACKS_H */
