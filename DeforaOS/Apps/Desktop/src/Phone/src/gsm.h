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

typedef enum _GSMBatteryStatus
{
	GSM_BATTERY_STATUS_POWERED = 0,
	GSM_BATTERY_STATUS_NOT_POWERED = 1,
	GSM_BATTERY_STATUS_NO_BATTERY = 2,
	GSM_BATTERY_STATUS_POWER_FAULT = 3
} GSMBatteryStatus;

typedef enum _GSMCallType
{
	GSM_CALL_TYPE_UNKNOWN = 0,
	GSM_CALL_TYPE_DATA,
	GSM_CALL_TYPE_VOICE
} GSMCallType;

typedef struct _GSMCommand GSMCommand;
typedef void (*GSMCommandCallback)(GSM * gsm);

typedef enum _GSMEncoding
{
	GSM_ENCODING_UTF8 = 0,
	GSM_ENCODING_RAW_DATA
} GSMEncoding;

typedef enum _GSMError
{
	GSM_ERROR_UNKNOWN = 0,
	GSM_ERROR_ANSWER_FAILED,
	GSM_ERROR_BATTERY_CHARGE_FAILED,
	GSM_ERROR_BUSY,
	GSM_ERROR_CALL_FAILED,
	GSM_ERROR_CALL_WAITING_FAILED,
	GSM_ERROR_CONTACT_FETCH_FAILED,
	GSM_ERROR_CONTACT_LIST_FAILED,
	GSM_ERROR_FUNCTIONAL_FAILED,
	GSM_ERROR_HANGUP_FAILED,
	GSM_ERROR_MESSAGE_DELETE_FAILED,
	GSM_ERROR_MESSAGE_FETCH_FAILED,
	GSM_ERROR_MESSAGE_LIST_FAILED,
	GSM_ERROR_MESSAGE_SEND_FAILED,
	GSM_ERROR_MUTE_FAILED,
	GSM_ERROR_NO_ANSWER,
	GSM_ERROR_NO_CARRIER,
	GSM_ERROR_NO_DIALTONE,
	GSM_ERROR_OPERATOR_MODE_FAILED,
	GSM_ERROR_REJECT_FAILED,
	GSM_ERROR_RESET_FAILED,
	GSM_ERROR_SIGNAL_LEVEL_FAILED,
	GSM_ERROR_SIM_PIN_REQUIRED,
	GSM_ERROR_SIM_PIN_WRONG
} GSMError;

typedef enum _GSMEventType
{
	GSM_EVENT_TYPE_ERROR = 0,
	GSM_EVENT_TYPE_BATTERY_CHARGE,
	GSM_EVENT_TYPE_CALL_PRESENTATION,
	GSM_EVENT_TYPE_CALL_WAITING,
	GSM_EVENT_TYPE_CONTACT,
	GSM_EVENT_TYPE_CONTACT_LIST,
	GSM_EVENT_TYPE_FUNCTIONAL,
	GSM_EVENT_TYPE_INCOMING_CALL,
	GSM_EVENT_TYPE_INCOMING_MESSAGE,
	GSM_EVENT_TYPE_MESSAGE,
	GSM_EVENT_TYPE_MESSAGE_DELETED,
	GSM_EVENT_TYPE_MESSAGE_LIST,
	GSM_EVENT_TYPE_MESSAGE_SENT,
	GSM_EVENT_TYPE_MUTE,
	GSM_EVENT_TYPE_OPERATOR,
	GSM_EVENT_TYPE_PHONE_ACTIVITY,
	GSM_EVENT_TYPE_REGISTRATION,
	GSM_EVENT_TYPE_SIGNAL_LEVEL,
	GSM_EVENT_TYPE_SIM_PIN_VALID,
	GSM_EVENT_TYPE_STATUS,
	GSM_EVENT_TYPE_UNKNOWN
} GSMEventType;

typedef enum _GSMMessageFormat
{
	GSM_MESSAGE_FORMAT_PDU = 0,
	GSM_MESSAGE_FORMAT_TEXT = 1
} GSMMessageFormat;

typedef enum _GSMMessageList
{
	GSM_MESSAGE_LIST_UNREAD = 0,
	GSM_MESSAGE_LIST_READ = 1,
	GSM_MESSAGE_LIST_UNSENT = 2,
	GSM_MESSAGE_LIST_SENT = 3,
	GSM_MESSAGE_LIST_ALL = 4
} GSMMessageList;

typedef enum _GSMMessageMode
{
	GSM_MESSAGE_MODE_BUFFER_REPLACE = 0,
	GSM_MESSAGE_MODE_DISCARD_REJECT = 1,
	GSM_MESSAGE_MODE_BUFFER_FLUSH = 2,
	GSM_MESSAGE_MODE_FORWARD = 3
} GSMMessageMode;

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

typedef enum _GSMPhoneActivity
{
	GSM_PHONE_ACTIVITY_READY = 0,
	GSM_PHONE_ACTIVITY_UNKNOWN = 2,
	GSM_PHONE_ACTIVITY_RINGING = 3,
	GSM_PHONE_ACTIVITY_CALL = 4
} GSMPhoneActivity;

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

	/* GSM_EVENT_TYPE_BATTERY_CHARGE */
	struct
	{
		GSMEventType type;
		GSMBatteryStatus status;
		unsigned int level;
	} battery_charge;

	/* GSM_EVENT_TYPE_CALL_PRESENTATION */
	struct
	{
		GSMEventType type;
		char const * number;
		unsigned int format;
	} call_presentation;

	/* GSM_EVENT_TYPE_CALL_WAITING_CONTROL */
	struct
	{
		GSMEventType type;
		unsigned int unsollicited;
	} call_waiting_control;

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

	/* GSM_EVENT_TYPE_INCOMING_MESSAGE */
	struct
	{
		GSMEventType type;
		char const * memory;
		unsigned int index;
	} incoming_message;

	/* GSM_EVENT_TYPE_MESSAGE */
	struct
	{
		GSMEventType type;
		unsigned int index;
		char const * number;
		time_t date;
		GSMEncoding encoding;
		unsigned int length;
		char const * content;
	} message;

	/* GSM_EVENT_TYPE_MESSAGE_DELETED */
	struct
	{
		GSMEventType type;
		unsigned int index;
	} message_deleted;

	/* GSM_EVENT_TYPE_MESSAGE_SENT */
	struct
	{
		GSMEventType type;
		unsigned int mr;
	} message_sent;

	/* GSM_EVENT_TYPE_MUTE */
	struct
	{
		GSMEventType type;
		unsigned int mute;
	} mute;

	/* GSM_EVENT_TYPE_OPERATOR */
	struct
	{
		GSMEventType type;
		GSMOperatorMode mode;
		GSMOperatorFormat format;
		char const * operator;
		unsigned int lai;
	} operator;

	/* GSM_EVENT_TYPE_PHONE_ACTIVITY */
	struct
	{
		GSMEventType type;
		GSMPhoneActivity activity;
	} phone_activity;

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

	/* GSM_EVENT_TYPE_UNKNOWN */
	struct
	{
		GSMEventType type;
		char const * result;
	} unknown;
} GSMEvent;

typedef int (*GSMCallback)(GSMEvent * event, gpointer data);


/* functions */
GSM * gsm_new(char const * device, unsigned int baudrate, unsigned int hwflow);
void gsm_delete(GSM * gsm);

/* accessors */
void gsm_set_callback(GSM * gsm, GSMCallback callback, gpointer data);

unsigned int gsm_get_retry(GSM * gsm);
int gsm_set_call_presentation(GSM * gsm, int set);
int gsm_set_call_waiting_control(GSM * gsm, int unsollicited);
int gsm_set_extended_ring_reports(GSM * gsm, int extended);
int gsm_set_functional(GSM * gsm, int functional);
int gsm_set_line_presentation(GSM * gsm, int set);
int gsm_set_message_indications(GSM * gsm, GSMMessageMode mode,
		int unsollicited);
int gsm_set_mute(GSM * gsm, int mute);
int gsm_set_operator_format(GSM * gsm, GSMOperatorFormat format);
int gsm_set_operator_mode(GSM * gsm, GSMOperatorMode mode);
int gsm_set_registration_report(GSM * gsm, GSMRegistrationReport report);
int gsm_set_retry(GSM * gsm, unsigned int retry);
int gsm_set_supplementary_service_notifications(GSM * gsm, int intermediate,
		int unsollicited);

/* useful */
/* call management */
int gsm_call_answer(GSM * gsm);
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number);
int gsm_call_contact(GSM * gsm, GSMCallType calltype, unsigned int index);
int gsm_call_hangup(GSM * gsm);
int gsm_call_reject(GSM * gsm);

/* callbacks */
void gsm_callback_on_message_deleted(GSM * gsm);

int gsm_enter_sim_pin(GSM * gsm, char const * code);

/* event */
int gsm_event(GSM * gsm, GSMEventType type, ...);

/* fetching data */
int gsm_fetch_battery_charge(GSM * gsm);
int gsm_fetch_contact_list(GSM * gsm);
int gsm_fetch_contacts(GSM * gsm, unsigned int start, unsigned int end);
int gsm_fetch_message_list(GSM * gsm, GSMMessageList list);
int gsm_fetch_message(GSM * gsm, unsigned int index);
int gsm_fetch_mute(GSM * gsm);
int gsm_fetch_operator(GSM * gsm);
int gsm_fetch_registration(GSM * gsm);
int gsm_fetch_signal_level(GSM * gsm);

/* messaging */
int gsm_message_delete(GSM * gsm, unsigned int index);
int gsm_message_send(GSM * gsm, char const * number, GSMEncoding encoding,
		char const * text, size_t length);

/* queries */
int gsm_is_alive(GSM * gsm);
int gsm_is_call_waiting_control(GSM * gsm);
int gsm_is_functional(GSM * gsm);
int gsm_is_mute(GSM * gsm);
int gsm_is_phone_active(GSM * gsm);
int gsm_is_pin_needed(GSM * gsm);
int gsm_is_pin_valid(GSM * gsm);
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

#endif /* !PHONE_GSM_H */
