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



#include <gdk/gdkx.h>
#include "phone.h"
#include "callbacks.h"


/* callbacks */
/* on_phone_closex */
gboolean on_phone_closex(gpointer data)
{
	GtkWidget * widget = data;

	gtk_widget_hide(widget);
	return TRUE;
}


/* on_phone_filter */
static GdkFilterReturn _filter_message_show(Phone * phone,
		PhoneMessageShow what, gboolean show);

GdkFilterReturn on_phone_filter(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Phone * phone = data;
	XEvent * xev = xevent;
	XClientMessageEvent * xclient = &xev->xclient;
	PhoneMessage message;

	if(xev->type != ClientMessage)
		return GDK_FILTER_CONTINUE;
	if(xclient->message_type != gdk_x11_get_xatom_by_name(
				PHONE_CLIENT_MESSAGE))
		return GDK_FILTER_CONTINUE;
	message = xclient->data.b[0];
	switch(message)
	{
		case PHONE_MESSAGE_SHOW:
			return _filter_message_show(phone, xclient->data.b[1],
					xclient->data.b[2]);
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_message_show(Phone * phone,
		PhoneMessageShow what, gboolean show)
{
	switch(what)
	{
		case PHONE_MESSAGE_SHOW_CONTACTS:
			phone_show_contacts(phone, show);
			break;
		case PHONE_MESSAGE_SHOW_DIALER:
			phone_show_dialer(phone, show);
			break;
		case PHONE_MESSAGE_SHOW_MESSAGES:
			phone_show_messages(phone, show);
			break;
	}
	return GDK_FILTER_CONTINUE;
}


/* on_phone_code_clear */
void on_phone_code_clear(gpointer data)
{
	Phone * phone = data;

	phone_code_clear(phone);
}


/* on_phone_code_clicked */
void on_phone_code_clicked(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	char const * character;

	character = g_object_get_data(G_OBJECT(widget), "character");
	phone_code_append(phone, *character);
}


/* code */
void on_phone_code_enter(gpointer data)
{
	Phone * phone = data;

	phone_code_enter(phone);
}


/* on_phone_code_leave */
void on_phone_code_leave(gpointer data)
{
	Phone * phone = data;

	phone_show_code(phone, FALSE);
}


/* contacts */
/* on_phone_contacts_call */
void on_phone_contacts_call(gpointer data)
{
	Phone * phone = data;

	phone_contacts_call_selected(phone);
}


/* on_phone_contacts_delete */
void on_phone_contacts_delete(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_contacts_edit */
void on_phone_contacts_edit(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_contacts_show */
void on_phone_contacts_show(gpointer data)
{
	Phone * phone = data;

	phone_show_contacts(phone, TRUE);
}


/* on_phone_contacts_write */
void on_phone_contacts_write(gpointer data)
{
	Phone * phone = data;

	phone_contacts_write_selected(phone);
}


/* dialer */
/* on_phone_dialer_call */
void on_phone_dialer_call(gpointer data)
{
	Phone * phone = data;

	phone_call(phone, NULL);
}


/* on_phone_dialer_clicked */
void on_phone_dialer_clicked(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	char const * character;

	character = g_object_get_data(G_OBJECT(widget), "character");
	phone_dialer_append(phone, *character);
}


/* on_phone_dialer_hangup */
void on_phone_dialer_hangup(gpointer data)
{
	Phone * phone = data;

	phone_hangup(phone);
}


/* messages */
/* on_phone_messages_call */
void on_phone_messages_call(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_changed */
void on_phone_messages_changed(gpointer data)
{
	Phone * phone = data;

	phone_messages_count_buffer(phone);
}


/* on_phone_messages_delete */
void on_phone_messages_delete(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_inbox */
void on_phone_messages_inbox(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_outbox */
void on_phone_messages_outbox(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_reply */
void on_phone_messages_reply(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_send */
void on_phone_messages_send(gpointer data)
{
	Phone * phone = data;

	phone_messages_send(phone);
}


/* on_phone_messages_write */
void on_phone_messages_write(gpointer data)
{
	Phone * phone = data;

	phone_messages_write(phone, NULL, NULL);
}
