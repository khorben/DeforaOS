/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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



#ifndef PHONE_CALLBACKS_H
# define PHONE_CALLBACKS_H

# include <gtk/gtk.h>


/* callbacks */
gboolean on_phone_closex(gpointer data);
GdkFilterReturn on_phone_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

/* calls */
void on_phone_call_answer(gpointer data);
void on_phone_call_close(gpointer data);
void on_phone_call_hangup(gpointer data);
void on_phone_call_mute(GtkWidget * widget, gpointer data);
void on_phone_call_reject(gpointer data);
void on_phone_call_speaker(GtkWidget * widget, gpointer data);
void on_phone_call_volume(GtkWidget * widget, gpointer data);

/* code */
void on_phone_code_clear(gpointer data);
void on_phone_code_clicked(GtkWidget * widget, gpointer data);
void on_phone_code_enter(gpointer data);
void on_phone_code_leave(gpointer data);

/* contacts */
void on_phone_contacts_call(gpointer data);
void on_phone_contacts_delete(gpointer data);
void on_phone_contacts_edit(gpointer data);
void on_phone_contacts_new(gpointer data);
void on_phone_contacts_show(gpointer data);
void on_phone_contacts_write(gpointer data);

/* dialer */
void on_phone_dialer_call(gpointer data);
void on_phone_dialer_clicked(GtkWidget * widget, gpointer data);
void on_phone_dialer_hangup(gpointer data);

/* logs */
void on_phone_logs_activated(gpointer data);
void on_phone_logs_call(gpointer data);
void on_phone_logs_clear(gpointer data);
void on_phone_logs_write(gpointer data);

/* messages */
void on_phone_messages_activated(gpointer data);
void on_phone_messages_call(gpointer data);
void on_phone_messages_delete(gpointer data);
void on_phone_messages_reply(gpointer data);
void on_phone_messages_write(gpointer data);

/* read */
void on_phone_read_call(gpointer data);
void on_phone_read_delete(gpointer data);
void on_phone_read_forward(gpointer data);
void on_phone_read_reply(gpointer data);

/* write */
void on_phone_write_attach(gpointer data);
void on_phone_write_changed(gpointer data);
void on_phone_write_copy(gpointer data);
void on_phone_write_cut(gpointer data);
void on_phone_write_paste(gpointer data);
void on_phone_write_send(gpointer data);

#endif /* !PHONE_CALLBACKS_H */
