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
	PHONE_EVENT_SPEAKER_ON,
	PHONE_EVENT_SPEAKER_OFF,
	PHONE_EVENT_VIBRATOR_OFF,
	PHONE_EVENT_VIBRATOR_ON
} PhoneEvent;

typedef struct _PhonePluginHelper
{
	Phone * phone;
	char const * (*config_get)(Phone * phone, char const * section,
			char const * variable);
	void (*event)(Phone * phone, PhoneEvent event, ...);
	int (*queue)(Phone * phone, char const * command);
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

#endif /* !PHONE_PHONE_H */
