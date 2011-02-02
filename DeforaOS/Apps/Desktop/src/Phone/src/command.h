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



#ifndef PHONE_COMMAND_H
# define PHONE_COMMAND_H


/* GSMCommand */
/* private */
/* types */
typedef struct _GSMCommand GSMCommand;

typedef struct _GSM GSM;
typedef void (*GSMCommandCallback)(GSM * gsm);

typedef enum _GSMError
{
	GSM_ERROR_UNKNOWN = 0,
	GSM_ERROR_ANSWER_FAILED,
	GSM_ERROR_BATTERY_CHARGE_FAILED,
	GSM_ERROR_BUSY,
	GSM_ERROR_CALL_FAILED,
	GSM_ERROR_CALL_WAITING_FAILED,
	GSM_ERROR_CONTACT_DELETE_FAILED,
	GSM_ERROR_CONTACT_EDIT_FAILED,
	GSM_ERROR_CONTACT_FETCH_FAILED,
	GSM_ERROR_CONTACT_LIST_FAILED,
	GSM_ERROR_CONTACT_NEW_FAILED,
	GSM_ERROR_FUNCTIONAL_FAILED,
	GSM_ERROR_HANGUP_FAILED,
	GSM_ERROR_MESSAGE_DELETE_FAILED,
	GSM_ERROR_MESSAGE_FETCH_FAILED,
	GSM_ERROR_MESSAGE_INDICATIONS_FAILED,
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

typedef enum _GSMMode
{
	GSM_MODE_INIT = 0, GSM_MODE_COMMAND, GSM_MODE_DATA, GSM_MODE_PDU
} GSMMode;

typedef enum _GSMPriority
{
	GSM_PRIORITY_LOW = 0, GSM_PRIORITY_NORMAL, GSM_PRIORITY_HIGH,
	GSM_PRIORITY_HIGHEST
} GSMPriority;


/* public */
/* functions */
/* commands */
GSMCommand * gsm_command_new(char const * command);
void gsm_command_delete(GSMCommand * gsmc);

/* accessors */
GSMCommandCallback gsm_command_get_callback(GSMCommand * gsmc);
char const * gsm_command_get_command(GSMCommand * gsmc);
void * gsm_command_get_data(GSMCommand * gsmc);
GSMError gsm_command_get_error(GSMCommand * gsmc);
GSMMode gsm_command_get_mode(GSMCommand * gsmc);
GSMPriority gsm_command_get_priority(GSMCommand * gsmc);
unsigned int gsm_command_get_timeout(GSMCommand * gsmc);

void gsm_command_set_callback(GSMCommand * gsmc, GSMCommandCallback callback);
void gsm_command_set_data(GSMCommand * gsmc, void * data);
void gsm_command_set_error(GSMCommand * gsmc, GSMError error);
void gsm_command_set_mode(GSMCommand * gsmc, GSMMode mode);
void gsm_command_set_priority(GSMCommand * gsmc, GSMPriority priority);
void gsm_command_set_timeout(GSMCommand * gsmc, unsigned int timeout);

#endif /* !PHONE_COMMAND_H */
