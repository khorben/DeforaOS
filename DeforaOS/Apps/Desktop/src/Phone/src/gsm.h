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



#ifndef PHONE_GSM_H
# define PHONE_GSM_H


/* GSM */
/* types */
typedef enum _GSMCallType
{
	GSM_CALL_TYPE_DATA, GSM_CALL_TYPE_VOICE
} GSMCallType;

typedef enum _GSMEventType
{
	GSM_EVENT_TYPE_ERROR = 0,
	GSM_EVENT_TYPE_CONTACT,
	GSM_EVENT_TYPE_CONTACT_LIST,
	GSM_EVENT_TYPE_MESSAGE_LIST,
	GSM_EVENT_TYPE_OPERATOR,
	GSM_EVENT_TYPE_REGISTRATION,
	GSM_EVENT_TYPE_SIGNAL_LEVEL,
	GSM_EVENT_TYPE_STATUS
} GSMEventType;

typedef enum _GSMError
{
	GSM_ERROR_UNKNOWN = 0,
	GSM_ERROR_SIM_PIN_REQUIRED
} GSMError;

typedef enum _GSMOperatorFormat
{
	GSM_OPERATOR_FORMAT_LONG = 0,
	GSM_OPERATOR_FORMAT_SHORT = 1,
	GSM_OPERATOR_FORMAT_LAI = 2
} GSMOperatorFormat;

typedef enum _GSMStatus
{
	GSM_STATUS_INITIALIZED = 0, GSM_STATUS_REGISTERED
} GSMStatus;

typedef union _GSMEvent
{
	GSMEventType type;

	/* GSM_EVENT_TYPE_ERROR */
	struct
	{
		GSMEventType type;
		GSMError error;
		char const * message;
	} error;

	/* GSM_EVENT_TYPE_CONTACT */
	struct
	{
		GSMEventType type;
		unsigned int index;
		char const * name;
		char const * number;
	} contact;

	/* GSM_EVENT_TYPE_CONTACT_LIST */
	/* GSM_EVENT_TYPE_MESSAGE_LIST */
	struct
	{
		GSMEventType type;
		unsigned int start;
		unsigned int end;
	} contact_list, message_list;

	/* GSM_EVENT_TYPE_OPERATOR */
	struct
	{
		GSMEventType type;
		unsigned int mode;
		unsigned int format;
		char const * operator;
		unsigned int lai;
	} operator;

	/* GSM_EVENT_TYPE_REGISTRATION */
	struct
	{
		GSMEventType type;
		unsigned int n;
		unsigned int stat;
		unsigned int cell;
		unsigned int area;
	} registration;

	/* GSM_EVENT_TYPE_SIGNAL_LEVEL */
	struct
	{
		GSMEventType type;
		gdouble level;
	} signal_level;

	/* GSM_EVENT_TYPE_STATUS */
	struct
	{
		GSMEventType type;
		GSMStatus status;
	} status;
} GSMEvent;

typedef int (*GSMCallback)(GSMEvent * event, gpointer data);

typedef struct _GSM GSM;


/* functions */
GSM * gsm_new(char const * device, unsigned int baudrate);
void gsm_delete(GSM * gsm);

/* accessors */
void gsm_set_callback(GSM * gsm, GSMCallback callback, gpointer data);

unsigned int gsm_get_retry(GSM * gsm);
void gsm_set_operator_format(GSM * gsm, GSMOperatorFormat format);
void gsm_set_retry(GSM * gsm, unsigned int retry);

/* useful */
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number);
int gsm_enter_pin(GSM * gsm, char const * code);
int gsm_fetch_contact_list(GSM * gsm);
int gsm_fetch_contacts(GSM * gsm, unsigned int start, unsigned int end);
int gsm_fetch_message_list(GSM * gsm);
int gsm_fetch_messages(GSM * gsm, unsigned int start, unsigned int end);
int gsm_fetch_operator(GSM * gsm);
int gsm_fetch_signal_level(GSM * gsm);
int gsm_hangup(GSM * gsm);
int gsm_is_pin_needed(GSM * gsm);
int gsm_report_registration(GSM * gsm, int report);
void gsm_reset(GSM * gsm, unsigned int delay);

#endif /* !PHONE_GSM_H */
