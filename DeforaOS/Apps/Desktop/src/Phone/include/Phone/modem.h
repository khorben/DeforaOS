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



#ifndef DESKTOP_PHONE_MODEM_H
# define DESKTOP_PHONE_MODEM_H

# include <sys/types.h>


/* Modem */
/* types */
typedef struct _Modem Modem;

typedef enum _ModemAuthenticationMethod
{
	MODEM_AUTHENTICATION_METHOD_NONE = 0,
	MODEM_AUTHENTICATION_METHOD_PIN
} ModemAuthenticationMethod;

typedef enum _ModemAuthenticationStatus
{
	MODEM_AUTHENTICATION_STATUS_OK = 0,
	MODEM_AUTHENTICATION_STATUS_REQUIRED,
	MODEM_AUTHENTICATION_STATUS_ERROR
} ModemAuthenticationStatus;

typedef enum _ModemBatteryStatus
{
	MODEM_BATTERY_STATUS_UNKNOWN = 0,
	MODEM_BATTERY_STATUS_NONE,
	MODEM_BATTERY_STATUS_CONNECTED,
	MODEM_BATTERY_STATUS_CHARGING,
	MODEM_BATTERY_STATUS_ERROR
} ModemBatteryStatus;

typedef enum _ModemCallDirection
{
	MODEM_CALL_DIRECTION_NONE = 0,
	MODEM_CALL_DIRECTION_INCOMING,
	MODEM_CALL_DIRECTION_OUTGOING
} ModemCallDirection;

typedef enum _ModemCallStatus
{
	MODEM_CALL_STATUS_NONE = 0,
	MODEM_CALL_STATUS_RINGING,
	MODEM_CALL_STATUS_ACTIVE
} ModemCallStatus;

typedef enum _ModemCallType
{
	MODEM_CALL_TYPE_VOICE = 0,
	MODEM_CALL_TYPE_DATA
} ModemCallType;

typedef enum _ModemContactStatus
{
	MODEM_CONTACT_STATUS_OFFLINE = 0,
	MODEM_CONTACT_STATUS_AWAY,
	MODEM_CONTACT_STATUS_IDLE,
	MODEM_CONTACT_STATUS_ONLINE
} ModemContactStatus;
# define MODEM_CONTACT_STATUS_LAST	MODEM_CONTACT_STATUS_ONLINE
# define MODEM_CONTACT_STATUS_COUNT	(MODEM_CONTACT_STATUS_LAST + 1)

typedef enum _ModemMessageEncoding
{
	MODEM_MESSAGE_ENCODING_NONE = 0,
	MODEM_MESSAGE_ENCODING_DATA,
	MODEM_MESSAGE_ENCODING_ASCII,
	MODEM_MESSAGE_ENCODING_UTF8
} ModemMessageEncoding;

typedef enum _ModemMessageFolder
{
	MODEM_MESSAGE_FOLDER_UNKNOWN = 0,
	MODEM_MESSAGE_FOLDER_INBOX,
	MODEM_MESSAGE_FOLDER_OUTBOX,
	MODEM_MESSAGE_FOLDER_DRAFTS,
	MODEM_MESSAGE_FOLDER_TRASH,
	MODEM_MESSAGE_FOLDER_OTHER
} ModemMessageFolder;

typedef enum _ModemMessageStatus
{
	MODEM_MESSAGE_STATUS_NEW = 0,
	MODEM_MESSAGE_STATUS_UNREAD,
	MODEM_MESSAGE_STATUS_READ
} ModemMessageStatus;

typedef enum _ModemRegistrationMode
{
	MODEM_REGISTRATION_MODE_UNKNOWN = 0,
	MODEM_REGISTRATION_MODE_AUTOMATIC,
	MODEM_REGISTRATION_MODE_MANUAL,
	MODEM_REGISTRATION_MODE_DISABLED
} ModemRegistrationMode;

typedef enum _ModemRegistrationStatus
{
	MODEM_REGISTRATION_STATUS_UNKNOWN = 0,
	MODEM_REGISTRATION_STATUS_NOT_SEARCHING,
	MODEM_REGISTRATION_STATUS_SEARCHING,
	MODEM_REGISTRATION_STATUS_REGISTERED,
	MODEM_REGISTRATION_STATUS_DENIED
} ModemRegistrationStatus;

/* ModemConfig */
typedef enum _ModemConfigType
{
	MCT_NONE = 0,
	MCT_BOOLEAN,
	MCT_FILENAME,
	MCT_STRING,
	MCT_UINT32
} ModemConfigType;

typedef struct _ModemConfig
{
	char const * name;
	char const * title;
	ModemConfigType type;
	void * value;
} ModemConfig;

/* ModemEvent */
typedef enum _ModemEventType
{
	MODEM_EVENT_TYPE_ERROR = 0,
	MODEM_EVENT_TYPE_AUTHENTICATION,
	MODEM_EVENT_TYPE_BATTERY_LEVEL,
	MODEM_EVENT_TYPE_CALL,
	MODEM_EVENT_TYPE_CONNECTION,
	MODEM_EVENT_TYPE_CONTACT,
	MODEM_EVENT_TYPE_CONTACT_DELETED,
	MODEM_EVENT_TYPE_MESSAGE,
	MODEM_EVENT_TYPE_MESSAGE_DELETED,
	MODEM_EVENT_TYPE_MESSAGE_SENT,
	MODEM_EVENT_TYPE_MODEL,
	MODEM_EVENT_TYPE_REGISTRATION,
	MODEM_EVENT_TYPE_STATUS
} ModemEventType;
# define MODEM_EVENT_TYPE_LAST MODEM_EVENT_TYPE_STATUS
# define MODEM_EVENT_TYPE_COUNT (MODEM_EVENT_TYPE_LAST + 1)

typedef union _ModemEvent
{
	ModemEventType type;

	/* MODEM_EVENT_TYPE_ERROR */
	struct
	{
		ModemEventType type;
		char const * message;
	} error;

	/* MODEM_EVENT_TYPE_AUTHENTICATION */
	struct
	{
		ModemEventType type;
		char const * name;
		ModemAuthenticationMethod method;
		ModemAuthenticationStatus status;
		int retries;
	} authentication;

	/* MODEM_EVENT_TYPE_BATTERY_LEVEL */
	struct
	{
		ModemEventType type;
		ModemBatteryStatus status;
		double level;
	} battery_level;

	/* MODEM_EVENT_TYPE_CALL */
	struct
	{
		ModemEventType type;
		ModemCallType call_type;
		ModemCallDirection direction;
		ModemCallStatus status;
		char const * number;
	} call;

	/* MODEM_EVENT_TYPE_CONNECTION */
	struct
	{
		ModemEventType type;
		int connected;
		size_t in;
		size_t out;
	} connection;

	/* MODEM_EVENT_TYPE_CONTACT */
	struct
	{
		ModemEventType type;
		unsigned int id;
		ModemContactStatus status;
		char const * name;
		char const * number;
	} contact;

	/* MODEM_EVENT_TYPE_CONTACT_DELETED */
	struct
	{
		ModemEventType type;
		unsigned int id;
	} contact_deleted;

	/* MODEM_EVENT_TYPE_MESSAGE */
	struct
	{
		ModemEventType type;
		unsigned int id;
		time_t date;
		char const * number;
		ModemMessageFolder folder;
		ModemMessageStatus status;
		ModemMessageEncoding encoding;
		size_t length;
		char const * content;
	} message;

	/* MODEM_EVENT_TYPE_MESSAGE_DELETED, MODEM_EVENT_TYPE_MESSAGE_SENT */
	struct
	{
		ModemEventType type;
		unsigned int id;
	} message_deleted, message_sent;

	/* MODEM_EVENT_TYPE_MODEL */
	struct
	{
		ModemEventType type;
		char const * vendor;
		char const * name;
		char const * version;
	} model;

	/* MODEM_EVENT_TYPE_REGISTRATION */
	struct
	{
		ModemEventType type;
		ModemRegistrationMode mode;
		ModemRegistrationStatus status;
		char const * media;
		char const * _operator;
		double signal;
		int roaming;
	} registration;

	/* MODEM_EVENT_TYPE_STATUS */
	struct {
		ModemEventType type;
		int online;
	} status;
} ModemEvent;

/* ModemRequest */
typedef enum _ModemRequestType
{
	MODEM_REQUEST_AUTHENTICATE = 0,
	MODEM_REQUEST_CALL,
	MODEM_REQUEST_CALL_ANSWER,
	MODEM_REQUEST_CALL_CONTACT,
	MODEM_REQUEST_CALL_HANGUP,
	MODEM_REQUEST_CALL_LAST,
	MODEM_REQUEST_CALL_MUTE_DISABLE,
	MODEM_REQUEST_CALL_MUTE_ENABLE,
	MODEM_REQUEST_CALL_PRESENTATION,
	MODEM_REQUEST_CALL_WAITING_CONTROL,
	MODEM_REQUEST_CONTACT,
	MODEM_REQUEST_CONTACT_DELETE,
	MODEM_REQUEST_CONTACT_EDIT,
	MODEM_REQUEST_CONTACT_LIST,
	MODEM_REQUEST_CONTACT_NEW,
	MODEM_REQUEST_ENTER_SIM_PIN,
	MODEM_REQUEST_LINE_PRESENTATION,
	MODEM_REQUEST_MESSAGE,
	MODEM_REQUEST_MESSAGE_DELETE,
	MODEM_REQUEST_MESSAGE_LIST,
	MODEM_REQUEST_MESSAGE_SEND,
	MODEM_REQUEST_MUTE,
	MODEM_REQUEST_OPERATOR,
	MODEM_REQUEST_REGISTRATION,
	MODEM_REQUEST_REGISTRATION_REPORT,
	MODEM_REQUEST_SIGNAL_LEVEL,
	MODEM_REQUEST_UNSUPPORTED
} ModemRequestType;
# define MODEM_REQUEST_LAST MODEM_REQUEST_UNSUPPORTED
# define MODEM_REQUEST_COUNT (MODEM_REQUEST_LAST + 1)

typedef union _ModemRequest
{
	ModemRequestType type;

	/* MODEM_REQUEST_AUTHENTICATE */
	struct
	{
		ModemRequestType type;
		char const * name;
		char const * username;
		char const * password;
	} authenticate;

	/* MODEM_REQUEST_CALL */
	struct
	{
		ModemRequestType type;
		ModemCallType call_type;
		char const * number;
		int anonymous;
	} call;

	/* MODEM_REQUEST_CALL_PRESENTATION */
	struct
	{
		ModemRequestType type;
		int enabled;
	} call_presentation;

	/* MODEM_REQUEST_CONTACT, MODEM_REQUEST_CONTACT_DELETE */
	struct
	{
		ModemRequestType type;
		unsigned int id;
	} contact, contact_delete;

	/* MODEM_REQUEST_CONTACT_NEW */
	struct
	{
		ModemRequestType type;
		char const * name;
		char const * number;
	} contact_new;

	/* MODEM_REQUEST_MESSAGE, MODEM_REQUEST_MESSAGE_DELETE */
	struct
	{
		ModemRequestType type;
		unsigned int id;
	} message, message_delete;

	/* MODEM_REQUEST_MESSAGE_SEND */
	struct
	{
		ModemRequestType type;
		char const * number;
		ModemMessageEncoding encoding;
		size_t length;
		char const * content;
	} message_send;

	/* MODEM_REQUEST_PLUGIN */
	struct
	{
		ModemRequestType type;
		void * data;
	} plugin;

	/* MODEM_REQUEST_REGISTRATION */
	struct
	{
		ModemRequestType type;
		ModemRegistrationMode mode;
		char const * _operator;
	} registration;

	/* MODEM_REQUEST_UNSUPPORTED */
	struct
	{
		ModemRequestType type;
		char const * modem;
		unsigned int request_type;
		void * request;
		size_t size;
	} unsupported;
} ModemRequest;

typedef struct _ModemPluginHelper
{
	Modem * modem;
	int (*error)(Modem * modem, char const * message, int ret);
	void (*event)(Modem * modem, ModemEvent * event);
} ModemPluginHelper;

typedef struct _ModemPlugin ModemPlugin;

struct _ModemPlugin
{
	ModemPluginHelper * helper;
	char const * name;
	char const * icon;
	ModemConfig * config;
	int (*init)(ModemPlugin * plugin);
	int (*destroy)(ModemPlugin * plugin);
	int (*start)(ModemPlugin * plugin, unsigned int retry);
	int (*stop)(ModemPlugin * plugin);
	int (*request)(ModemPlugin * plugin, ModemRequest * request);
	int (*trigger)(ModemPlugin * plugin, ModemEventType event);
	void * priv;
};

#endif /* !DESKTOP_PHONE_MODEM_H */
