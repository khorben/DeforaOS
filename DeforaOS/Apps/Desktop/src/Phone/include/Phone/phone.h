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



#ifndef DESKTOP_PHONE_PHONE_H
# define DESKTOP_PHONE_PHONE_H

# include "modem.h"


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

typedef enum _PhoneEncoding
{
	/* XXX must match GSMEncoding from src/gsm.h */
	PHONE_ENCODING_UTF8 = 0,
	PHONE_ENCODING_DATA
} PhoneEncoding;

typedef enum _PhoneEventType
{
	PHONE_EVENT_TYPE_KEY_TONE,
	PHONE_EVENT_TYPE_MODEM_EVENT,		/* ModemEvent * event */
	PHONE_EVENT_TYPE_NOTIFICATION_OFF,
	PHONE_EVENT_TYPE_NOTIFICATION_ON,	/* char const * message? */
	PHONE_EVENT_TYPE_OFFLINE,
	PHONE_EVENT_TYPE_ONLINE,
	PHONE_EVENT_TYPE_RESUME,
	PHONE_EVENT_TYPE_SMS_RECEIVING,		/* char const *, GSMEncoding *,
						   char **, size_t * */
	PHONE_EVENT_TYPE_SMS_SENDING,		/* char const *, GSMEncoding *,
						   char **, size_t * */
	PHONE_EVENT_TYPE_SMS_SENT,
	PHONE_EVENT_TYPE_SPEAKER_OFF,
	PHONE_EVENT_TYPE_SPEAKER_ON,
	PHONE_EVENT_TYPE_STARTING,
	PHONE_EVENT_TYPE_STOPPING,
	PHONE_EVENT_TYPE_SUSPEND,
	PHONE_EVENT_TYPE_VIBRATOR_OFF,
	PHONE_EVENT_TYPE_VIBRATOR_ON,
	PHONE_EVENT_TYPE_VOLUME_GET,
	PHONE_EVENT_TYPE_VOLUME_SET
} PhoneEventType;

typedef union _PhoneEvent
{
	PhoneEventType type;

	/* PHONE_EVENT_TYPE_MODEM_EVENT */
	struct
	{
		PhoneEventType type;
		ModemEvent * event;
	} modem_event;

	/* PHONE_EVENT_TYPE_VOLUME_GET, PHONE_EVENT_TYPE_VOLUME_SET */
	struct
	{
		PhoneEventType type;
		double level;
	} volume_get, volume_set;
} PhoneEvent;

typedef enum _PhoneMessage
{
	PHONE_MESSAGE_SHOW = 0,
	PHONE_MESSAGE_POWER_MANAGEMENT
} PhoneMessage;

typedef enum _PhoneMessagePowerManagement
{
	PHONE_MESSAGE_POWER_MANAGEMENT_RESUME = 0,
	PHONE_MESSAGE_POWER_MANAGEMENT_SUSPEND
} PhoneMessagePowerManagement;

typedef enum _PhoneMessageShow
{
	PHONE_MESSAGE_SHOW_CONTACTS = 0,
	PHONE_MESSAGE_SHOW_DIALER,
	PHONE_MESSAGE_SHOW_LOGS,
	PHONE_MESSAGE_SHOW_MESSAGES,
	PHONE_MESSAGE_SHOW_SETTINGS,
	PHONE_MESSAGE_SHOW_WRITE
} PhoneMessageShow;


/* constants */
# define PHONE_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PHONE_CLIENT"
# define PHONE_EMBED_MESSAGE	"DEFORAOS_DESKTOP_PHONE_EMBED"

#endif /* !DESKTOP_PHONE_PHONE_H */
