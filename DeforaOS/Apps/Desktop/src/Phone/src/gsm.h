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

# include <glib.h>


/* GSM */
/* types */
typedef struct _GSM GSM;

typedef enum _GSMCallType
{
	GSM_CALL_TYPE_UNKNOWN = 0,
	GSM_CALL_TYPE_DATA,
	GSM_CALL_TYPE_VOICE
} GSMCallType;

typedef struct _GSMCommand GSMCommand;
typedef void (*GSMCommandCallback)(GSM * gsm);

typedef enum _GSMEventType
{
	GSM_EVENT_TYPE_ERROR = 0,
	GSM_EVENT_TYPE_CONTACT,
	GSM_EVENT_TYPE_CONTACT_LIST,
	GSM_EVENT_TYPE_FUNCTIONAL,
	GSM_EVENT_TYPE_INCOMING_CALL,
	GSM_EVENT_TYPE_MESSAGE_LIST,
	GSM_EVENT_TYPE_MESSAGE_SENT,
	GSM_EVENT_TYPE_OPERATOR,
	GSM_EVENT_TYPE_REGISTRATION,
	GSM_EVENT_TYPE_SIGNAL_LEVEL,
	GSM_EVENT_TYPE_SIM_PIN_VALID,
	GSM_EVENT_TYPE_STATUS
} GSMEventType;

typedef enum _GSMError
{
	GSM_ERROR_UNKNOWN = 0,
	GSM_ERROR_ANSWER_FAILED,
	GSM_ERROR_CALL_FAILED,
	GSM_ERROR_CONTACT_FETCH_FAILED,
	GSM_ERROR_CONTACT_LIST_FAILED,
	GSM_ERROR_FUNCTIONAL_FAILED,
	GSM_ERROR_HANGUP_FAILED,
	GSM_ERROR_MESSAGE_FETCH_FAILED,
	GSM_ERROR_MESSAGE_LIST_FAILED,
	GSM_ERROR_MESSAGE_SEND_FAILED,
	GSM_ERROR_SIGNAL_LEVEL_FAILED,
	GSM_ERROR_RESET_FAILED,
	GSM_ERROR_SIM_PIN_REQUIRED,
	GSM_ERROR_SIM_PIN_WRONG
} GSMError;

typedef enum _GSMMessageFormat
{
	GSM_MESSAGE_FORMAT_PDU = 0,
	GSM_MESSAGE_FORMAT_TEXT = 1
} GSMMessageFormat;

typedef enum _GSMMode
{
	GSM_MODE_INIT = 0, GSM_MODE_COMMAND, GSM_MODE_PDU
} GSMMode;

typedef enum _GSMOperatorFormat
{
	GSM_OPERATOR_FORMAT_LONG = 0,
	GSM_OPERATOR_FORMAT_SHORT = 1,
	GSM_OPERATOR_FORMAT_LAI = 2
} GSMOperatorFormat;

typedef enum _GSMOperatorMode
{
	GSM_OPERATOR_MODE_AUTOMATIC = 0,
	GSM_OPERATOR_MODE_MANUAL = 1,
	GSM_OPERATOR_MODE_DEREGISTER = 2,
	GSM_OPERATOR_MODE_SET_FORMAT = 3,
	GSM_OPERATOR_MODE_MANUAL_WITH_FALLBACK = 4
} GSMOperatorMode;

typedef enum _GSMPriority
{
	GSM_PRIORITY_LOW = 0, GSM_PRIORITY_NORMAL, GSM_PRIORITY_HIGH,
	GSM_PRIORITY_HIGHEST
} GSMPriority;

typedef enum _GSMRegistrationReport
{
	GSM_REGISTRATION_REPORT_DISABLE_UNSOLLICITED = 0,
	GSM_REGISTRATION_REPORT_ENABLE_UNSOLLICITED = 1,
	GSM_REGISTRATION_REPORT_ENABLE_UNSOLLICITED_WITH_LOCATION = 2
} GSMRegistrationReport;

typedef enum _GSMRegistrationStatus
{
	GSM_REGISTRATION_STATUS_NOT_SEARCHING = 0,
	GSM_REGISTRATION_STATUS_REGISTERED_HOME = 1,
	GSM_REGISTRATION_STATUS_NOT_REGISTERED = 2,
	GSM_REGISTRATION_STATUS_DENIED = 3,
	GSM_REGISTRATION_STATUS_UNKNOWN = 4,
	GSM_REGISTRATION_STATUS_REGISTERED_ROAMING = 5
} GSMRegistrationStatus;

typedef enum _GSMStatus
{
	GSM_STATUS_UNKNOWN = 0,
	GSM_STATUS_INITIALIZED,
	GSM_STATUS_READY,
	GSM_STATUS_REGISTERING,
	GSM_STATUS_REGISTERING_DENIED,
	GSM_STATUS_REGISTERED_HOME,
	GSM_STATUS_REGISTERED_ROAMING
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

	/* GSM_EVENT_TYPE_FUNCTIONAL */
	struct
	{
		GSMEventType type;
		unsigned int functional;
	} functional;

	/* GSM_EVENT_TYPE_INCOMING_CALL */
	struct
	{
		GSMEventType type;
		GSMCallType calltype;
	} incoming_call;

	/* GSM_EVENT_TYPE_MESSAGE_SENT */
	struct
	{
		GSMEventType type;
		unsigned int mr;
	} message_sent;

	/* GSM_EVENT_TYPE_OPERATOR */
	struct
	{
		GSMEventType type;
		GSMOperatorMode mode;
		GSMOperatorFormat format;
		char const * operator;
		unsigned int lai;
	} operator;

	/* GSM_EVENT_TYPE_REGISTRATION */
	struct
	{
		GSMEventType type;
		GSMRegistrationReport n;
		GSMRegistrationStatus stat;
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


/* functions */
GSM * gsm_new(char const * device, unsigned int baudrate, unsigned int hwflow);
void gsm_delete(GSM * gsm);

/* accessors */
void gsm_set_callback(GSM * gsm, GSMCallback callback, gpointer data);

unsigned int gsm_get_retry(GSM * gsm);
int gsm_set_extended_ring_reports(GSM * gsm, int extended);
int gsm_set_functional(GSM * gsm, int functional);
int gsm_set_operator_format(GSM * gsm, GSMOperatorFormat format);
int gsm_set_operator_mode(GSM * gsm, GSMOperatorMode mode);
int gsm_set_registration_report(GSM * gsm, GSMRegistrationReport report);
int gsm_set_retry(GSM * gsm, unsigned int retry);

/* useful */
/* call management */
int gsm_call_answer(GSM * gsm);
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number);
int gsm_call_contact(GSM * gsm, GSMCallType calltype, unsigned int index);
int gsm_call_hangup(GSM * gsm);

int gsm_enter_sim_pin(GSM * gsm, char const * code);

/* event */
int gsm_event(GSM * gsm, GSMEventType type, ...);

/* fetching data */
int gsm_fetch_contact_list(GSM * gsm);
int gsm_fetch_contacts(GSM * gsm, unsigned int start, unsigned int end);
int gsm_fetch_message_list(GSM * gsm);
int gsm_fetch_messages(GSM * gsm, unsigned int start, unsigned int end);
int gsm_fetch_operator(GSM * gsm);
int gsm_fetch_registration(GSM * gsm);
int gsm_fetch_signal_level(GSM * gsm);

/* queries */
int gsm_is_functional(GSM * gsm);
int gsm_is_pin_needed(GSM * gsm);
int gsm_is_registered(GSM * gsm);

/* queue management */
GSMCommand * gsm_queue(GSM * gsm, char const * command);
int gsm_queue_command(GSM * gsm, GSMCommand * command);
int gsm_queue_full(GSM * gsm, GSMPriority priority, char const * command,
		GSMError error, GSMCommandCallback callback);
int gsm_queue_full_mode(GSM * gsm, GSMPriority priority, char const * command,
		GSMError error, GSMCommandCallback callback, GSMMode mode);
int gsm_queue_with_error(GSM * gsm, char const * command, GSMError error);

int gsm_reset(GSM * gsm, unsigned int delay);

int gsm_send_message(GSM * gsm, char const * number, char const * text);

#endif /* !PHONE_GSM_H */
