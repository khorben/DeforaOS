/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <string.h>
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
static GdkFilterReturn _filter_message_power_management(Phone * phone,
		PhoneMessagePowerManagement what);
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
		case PHONE_MESSAGE_POWER_MANAGEMENT:
			return _filter_message_power_management(phone,
					xclient->data.b[1]);
		case PHONE_MESSAGE_SHOW:
			return _filter_message_show(phone, xclient->data.b[1],
					xclient->data.b[2]);
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _filter_message_power_management(Phone * phone,
		PhoneMessagePowerManagement what)
{
	PhoneEvent event;

	memset(&event, 0, sizeof(event));
	switch(what)
	{
		case PHONE_MESSAGE_POWER_MANAGEMENT_RESUME:
			event.type = PHONE_EVENT_TYPE_RESUME;
			phone_event(phone, &event);
			break;
		case PHONE_MESSAGE_POWER_MANAGEMENT_SUSPEND:
			event.type = PHONE_EVENT_TYPE_SUSPEND;
			phone_event(phone, &event);
			break;
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
		case PHONE_MESSAGE_SHOW_LOGS:
			phone_show_logs(phone, show);
			break;
		case PHONE_MESSAGE_SHOW_MESSAGES:
			phone_show_messages(phone, show,
					MODEM_MESSAGE_FOLDER_INBOX);
			break;
		case PHONE_MESSAGE_SHOW_SETTINGS:
			phone_show_settings(phone, show);
			break;
		case PHONE_MESSAGE_SHOW_WRITE:
			phone_show_write(phone, show, NULL, NULL);
			break;
	}
	return GDK_FILTER_CONTINUE;
}


/* calls */
/* on_phone_call_answer */
void on_phone_call_answer(gpointer data)
{
	Phone * phone = data;

	phone_call_answer(phone);
}


/* on_phone_call_close */
void on_phone_call_close(gpointer data)
{
	Phone * phone = data;

	phone_show_call(phone, FALSE);
}


/* on_phone_call_hangup */
void on_phone_call_hangup(gpointer data)
{
	Phone * phone = data;

	phone_call_hangup(phone);
}


/* on_phone_call_mute */
void on_phone_call_mute(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	gboolean mute;

	mute = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	phone_call_mute(phone, mute);
}


/* on_phone_call_reject */
void on_phone_call_reject(gpointer data)
{
	Phone * phone = data;

	phone_call_reject(phone);
}


/* on_phone_call_speaker */
void on_phone_call_speaker(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	gboolean speaker;

	speaker = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	phone_call_speaker(phone, speaker);
}


/* on_phone_call_volume */
void on_phone_call_volume(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	gdouble volume;

	volume = gtk_range_get_value(GTK_RANGE(widget));
	phone_call_set_volume(phone, volume);
}


/* code */
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

	phone_contacts_delete_selected(phone);
}


/* on_phone_contacts_edit */
void on_phone_contacts_edit(gpointer data)
{
	Phone * phone = data;

	phone_contacts_edit_selected(phone);
}


/* on_phone_contacts_new */
void on_phone_contacts_new(gpointer data)
{
	Phone * phone = data;

	phone_contacts_new(phone);
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
	PhoneEvent event;

	event.type = PHONE_EVENT_TYPE_KEY_TONE;
	phone_event(phone, &event);
	phone_dialer_call(phone, NULL);
}


/* on_phone_dialer_clicked */
void on_phone_dialer_clicked(GtkWidget * widget, gpointer data)
{
	Phone * phone = data;
	char const * character;
	PhoneEvent event;

	character = g_object_get_data(G_OBJECT(widget), "character");
	event.type = PHONE_EVENT_TYPE_KEY_TONE;
	phone_event(phone, &event);
	phone_dialer_append(phone, *character);
}


/* on_phone_dialer_hangup */
void on_phone_dialer_hangup(gpointer data)
{
	Phone * phone = data;
	PhoneEvent event;

	event.type = PHONE_EVENT_TYPE_KEY_TONE;
	phone_event(phone, &event);
	phone_dialer_hangup(phone);
}


/* logs */
/* on_phone_logs_activated */
void on_phone_logs_activated(gpointer data)
{
	Phone * phone = data;

	/* XXX may not be the most appropriate action to take */
	phone_logs_call_selected(phone);
}


/* on_phone_logs_call */
void on_phone_logs_call(gpointer data)
{
	Phone * phone = data;

	phone_logs_call_selected(phone);
}


/* on_phone_logs_clear */
void on_phone_logs_clear(gpointer data)
{
	Phone * phone = data;

	phone_logs_clear(phone);
}


/* on_phone_logs_write */
void on_phone_logs_write(gpointer data)
{
	Phone * phone = data;

	phone_logs_write_selected(phone);
}


/* messages */
/* on_phone_messages_activated */
void on_phone_messages_activated(gpointer data)
{
	Phone * phone = data;

	phone_messages_read_selected(phone);
}


/* on_phone_messages_call */
void on_phone_messages_call(gpointer data)
{
	Phone * phone = data;

	phone_messages_call_selected(phone);
}


/* on_phone_messages_delete */
void on_phone_messages_delete(gpointer data)
{
	Phone * phone = data;

	phone_messages_delete_selected(phone);
}


/* on_phone_messages_reply */
void on_phone_messages_reply(gpointer data)
{
	Phone * phone = data;

	/* FIXME implement */
}


/* on_phone_messages_write */
void on_phone_messages_write(gpointer data)
{
	Phone * phone = data;

	phone_messages_write(phone, NULL, NULL);
}


/* read */
/* on_phone_read_call */
void on_phone_read_call(gpointer data)
{
	Phone * phone = data;

	phone_read_call(phone);
}


/* on_phone_read_delete */
void on_phone_read_delete(gpointer data)
{
	Phone * phone = data;

	phone_read_delete(phone);
}


/* on_phone_read_forward */
void on_phone_read_forward(gpointer data)
{
	Phone * phone = data;

	phone_read_forward(phone);
}


/* on_phone_read_reply */
void on_phone_read_reply(gpointer data)
{
	Phone * phone = data;

	phone_read_reply(phone);
}


/* write */
/* on_phone_write_attach */
void on_phone_write_attach(gpointer data)
{
	Phone * phone = data;

	phone_write_attach_dialog(phone);
}


/* on_phone_write_changed */
void on_phone_write_changed(gpointer data)
{
	Phone * phone = data;

	phone_write_count_buffer(phone);
}


/* on_phone_write_copy */
void on_phone_write_copy(gpointer data)
{
	Phone * phone = data;

	phone_write_copy(phone);
}


/* on_phone_write_cut */
void on_phone_write_cut(gpointer data)
{
	Phone * phone = data;

	phone_write_cut(phone);
}


/* on_phone_write_paste */
void on_phone_write_paste(gpointer data)
{
	Phone * phone = data;

	phone_write_paste(phone);
}


/* on_phone_write_send */
void on_phone_write_send(gpointer data)
{
	Phone * phone = data;

	phone_write_send(phone);
}
