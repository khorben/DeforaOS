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

typedef enum _PhoneEncoding
{
	/* XXX must match GSMEncoding from src/gsm.h */
	PHONE_ENCODING_UTF8 = 0,
	PHONE_ENCODING_DATA
} PhoneEncoding;

typedef enum _PhoneEvent
{
	PHONE_EVENT_BATTERY_LEVEL = 0,	/* gdouble */
	PHONE_EVENT_CALL_ESTABLISHED,
	PHONE_EVENT_CALL_INCOMING,
	PHONE_EVENT_CALL_OUTGOING,
	PHONE_EVENT_CALL_TERMINATED,
	PHONE_EVENT_CALLING,		/* char const * number */
	PHONE_EVENT_FUNCTIONAL,
	PHONE_EVENT_KEY_TONE,
	PHONE_EVENT_NOTIFICATION_OFF,
	PHONE_EVENT_NOTIFICATION_ON,
	PHONE_EVENT_OFFLINE,
	PHONE_EVENT_ONLINE,
	PHONE_EVENT_SET_OPERATOR,	/* char const * operator */
	PHONE_EVENT_SET_SIGNAL_LEVEL,	/* gdouble level */
	PHONE_EVENT_SET_VOLUME,		/* gdouble volume */
	PHONE_EVENT_SIM_PIN_VALID,
	PHONE_EVENT_SMS_RECEIVED,
	PHONE_EVENT_SMS_RECEIVING,	/* char const *, GSMEncoding *, char **,
					   size_t * */
	PHONE_EVENT_SMS_SENDING,	/* char const *, GSMEncoding *, char **,
					   size_t * */
	PHONE_EVENT_SMS_SENT,
	PHONE_EVENT_SPEAKER_ON,
	PHONE_EVENT_SPEAKER_OFF,
	PHONE_EVENT_VIBRATOR_OFF,
	PHONE_EVENT_VIBRATOR_ON
} PhoneEvent;

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

typedef struct _PhonePlugin PhonePlugin;

typedef int (PhoneTriggerCallback)(PhonePlugin * plugin, char const * result);
typedef void (PhoneConfigForeachCallback)(char const * variable,
		char const * value, void * priv);

typedef struct _PhonePluginHelper
{
	Phone * phone;
	void (*config_foreach)(Phone * phone, char const * section,
			PhoneConfigForeachCallback callback, void * priv);
	char const * (*config_get)(Phone * phone, char const * section,
			char const * variable);
	int (*config_set)(Phone * phone, char const * section,
			char const * variable, char const * value);
	int (*error)(Phone * phone, char const * message, int ret);
	int (*event)(Phone * phone, PhoneEvent event, ...);
	int (*queue)(Phone * phone, char const * command);
	int (*register_trigger)(Phone * phone, PhonePlugin * plugin,
			char const * trigger, PhoneTriggerCallback callback);
} PhonePluginHelper;

struct _PhonePlugin
{
	PhonePluginHelper * helper;
	char const * name;
	char const * icon;
	int (*init)(PhonePlugin * plugin);
	int (*destroy)(PhonePlugin * plugin);
	int (*event)(PhonePlugin * plugin, PhoneEvent event, ...);
	void (*settings)(PhonePlugin * plugin);
	void * priv;
};


/* constants */
# define PHONE_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PHONE_CLIENT"
# define PHONE_EMBED_MESSAGE	"DEFORAOS_DESKTOP_PHONE_EMBED"

#endif /* !PHONE_PHONE_H */
