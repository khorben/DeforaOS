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

/* code */
void on_phone_code_clear(gpointer data);
void on_phone_code_clicked(GtkWidget * widget, gpointer data);
void on_phone_code_enter(gpointer data);
void on_phone_code_leave(gpointer data);

/* contacts */
void on_phone_contacts_call(gpointer data);
void on_phone_contacts_compose(gpointer data);
void on_phone_contacts_delete(gpointer data);
void on_phone_contacts_edit(gpointer data);
void on_phone_contacts_show(gpointer data);

/* dialer */
void on_phone_dialer_call(gpointer data);
void on_phone_dialer_clicked(GtkWidget * widget, gpointer data);
void on_phone_dialer_hangup(gpointer data);

/* messages */
void on_phone_messages_call(gpointer data);
void on_phone_messages_compose(gpointer data);
void on_phone_messages_delete(gpointer data);
void on_phone_messages_inbox(gpointer data);
void on_phone_messages_outbox(gpointer data);
void on_phone_messages_reply(gpointer data);

#endif /* !PHONE_CALLBACKS_H */
