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



#ifndef PHONE_SRC_PHONE_H
# define PHONE_SRC_PHONE_H

# include <Phone.h>


/* Phone */
/* types */
typedef enum _PhoneMessage
{
	PHONE_MESSAGE_SHOW = 0
} PhoneMessage;

typedef enum _PhoneMessageShow
{
	PHONE_MESSAGE_SHOW_CONTACTS = 0,
	PHONE_MESSAGE_SHOW_DIALER,
	PHONE_MESSAGE_SHOW_LOGS,
	PHONE_MESSAGE_SHOW_MESSAGES,
	PHONE_MESSAGE_SHOW_SETTINGS
} PhoneMessageShow;


/* constants */
# define PHONE_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PHONE_CLIENT"
# define PHONE_EMBED_MESSAGE	"DEFORAOS_DESKTOP_PHONE_EMBED"


/* functions */
Phone * phone_new(char const * device, unsigned int baudrate, int retry,
		int hwflow);
void phone_delete(Phone * phone);


/* useful */
int phone_error(Phone * phone, char const * message, int ret);

/* calls */
void phone_call_answer(Phone * phone);
void phone_call_hangup(Phone * phone);
void phone_call_mute(Phone * phone, gboolean mute);
void phone_call_reject(Phone * phone);
void phone_call_speaker(Phone * phone, gboolean speaker);

/* code */
int phone_code_append(Phone * phone, char character);
void phone_code_clear(Phone * phone);
void phone_code_enter(Phone * phone);

/* contacts */
void phone_contacts_call_selected(Phone * phone);
void phone_contacts_delete_selected(Phone * phone);
void phone_contacts_edit_selected(Phone * phone);
void phone_contacts_new(Phone * phone);
void phone_contacts_set(Phone * phone, unsigned int index, char const * name,
		char const * number);
void phone_contacts_write_selected(Phone * phone);

/* dialer */
int phone_dialer_append(Phone * phone, char character);
void phone_dialer_call(Phone * phone, char const * number);
void phone_dialer_hangup(Phone * phone);

/* events */
int phone_event(Phone * phone, PhoneEvent event, ...);

/* interface */
void phone_show_call(Phone * phone, gboolean show, ...);	/* PhoneCall */
void phone_show_code(Phone * phone, gboolean show, ...);	/* PhoneCode */
void phone_show_contacts(Phone * phone, gboolean show);
void phone_show_dialer(Phone * phone, gboolean show);
void phone_show_logs(Phone * phone, gboolean show);
void phone_show_messages(Phone * phone, gboolean show);
void phone_show_plugins(Phone * phone, gboolean show);
void phone_show_read(Phone * phone, gboolean show, ...);
void phone_show_settings(Phone * phone, gboolean show);
void phone_show_system(Phone * phone, gboolean show);
void phone_show_write(Phone * phone, gboolean show, ...);

/* logs */
void phone_logs_call_selected(Phone * phone);
void phone_logs_clear(Phone * phone);
void phone_logs_write_selected(Phone * phone);

/* messages */
void phone_messages_call_selected(Phone * phone);
void phone_messages_delete_selected(Phone * phone);
void phone_messages_read_selected(Phone * phone);
void phone_messages_set(Phone * phone, unsigned int index, char const * number,
		time_t date, char const * content);
void phone_messages_write(Phone * phone, char const * number,
		char const * text);

/* plugins */
int phone_load(Phone * phone, char const * plugin);
int phone_unload(Phone * phone, PhonePlugin * plugin);
void phone_unload_all(Phone * phone);

/* read */
void phone_read_call(Phone * phone);
void phone_read_delete(Phone * phone);
void phone_read_reply(Phone * phone);

/* settings */
void phone_settings_open_selected(Phone * phone);

/* write */
void phone_write_count_buffer(Phone * phone);
void phone_write_send(Phone * phone);

#endif /* !PHONE_SRC_PHONE_H */
