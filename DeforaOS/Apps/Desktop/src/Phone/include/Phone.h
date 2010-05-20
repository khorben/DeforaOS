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



#ifndef PHONE_PHONE_H
# define PHONE_PHONE_H

# include <glib.h>


/* Phone */
/* types */
typedef struct _Phone Phone;

typedef enum _PhoneCall
{
	PHONE_CALL_ESTABLISHED = 0,
	PHONE_CALL_INCOMING,
	PHONE_CALL_OUTGOING,
	PHONE_CALL_TERMINATED
} PhoneCall;

typedef enum _PhoneCode
{
	PHONE_CODE_SIM_PIN = 0
} PhoneCode;

typedef enum _PhoneEvent
{
	PHONE_EVENT_CALL_ESTABLISHED = 0,
	PHONE_EVENT_CALL_INCOMING,
	PHONE_EVENT_CALL_OUTGOING,
	PHONE_EVENT_CALL_TERMINATED,
	PHONE_EVENT_NOTIFICATION_OFF,
	PHONE_EVENT_NOTIFICATION_ON,
	PHONE_EVENT_SIM_VALID,
	PHONE_EVENT_SMS_SENT,		/* char * buffer, size_t * len */
	PHONE_EVENT_SMS_RECEIVED,	/* char * buffer, size_t * len */
	PHONE_EVENT_VIBRATOR_OFF,
	PHONE_EVENT_VIBRATOR_ON
} PhoneEvent;

typedef struct _PhonePluginHelper
{
	char const * (*config_get)(Phone * phone, char const * section,
			char const * variable);
	Phone * phone;
} PhonePluginHelper;

typedef struct _PhonePlugin PhonePlugin;

struct _PhonePlugin
{
	PhonePluginHelper * helper;
	int (*init)(PhonePlugin * plugin);
	int (*destroy)(PhonePlugin * plugin);
	int (*event)(PhonePlugin * plugin, PhoneEvent event, ...);
	void * priv;
};


/* functions */
/* useful */
int phone_error(Phone * phone, char const * message, int ret);

/* interface */
void phone_show_call(Phone * phone, gboolean show, ...);	/* PhoneCall */
void phone_show_code(Phone * phone, gboolean show, ...);	/* PhoneCode */
void phone_show_contacts(Phone * phone, gboolean show);
void phone_show_dialer(Phone * phone, gboolean show);
void phone_show_messages(Phone * phone, gboolean show);
void phone_show_read(Phone * phone, gboolean show, ...);
void phone_show_write(Phone * phone, gboolean show);

/* calls */
void phone_call_answer(Phone * phone);
void phone_call_hangup(Phone * phone);
void phone_call_mute(Phone * phone, gboolean mute);
void phone_call_reject(Phone * phone);

/* events */
void phone_event(Phone * phone, PhoneEvent event, ...);

/* plugins */
int phone_load(Phone * phone, char const * plugin);

#endif /* !PHONE_PHONE_H */
