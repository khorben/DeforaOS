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



#ifndef PHONE_MODEM_H
# define PHONE_MODEM_H

# include "gsm.h"


/* GSMModem */
/* public */
/* types */
typedef struct _GSMModem GSMModem;

typedef enum _GSMModemAlphabet
{
	GSM_MODEM_ALPHABET_DEFAULT = 0,
	GSM_MODEM_ALPHABET_DATA
} GSMModemAlphabet;

typedef enum _GSMModemQuirk
{
	GSM_MODEM_QUIRK_NONE = 0,
	GSM_MODEM_QUIRK_CPIN_QUOTES = 1,
	GSM_MODEM_QUIRK_WANT_SMSC_IN_PDU = 2
} GSMModemQuirk;


/* prototypes */
GSMModem * gsm_modem_new(GSM * gsm);
void gsm_modem_delete(GSMModem * gsmm);

/* accessors */
void gsm_modem_set_quirks(GSMModem * gsmm, unsigned long quirks);

/* useful */
int gsm_modem_call(GSMModem * gsmm, GSMCallType calltype, char const * number);
int gsm_modem_call_answer(GSMModem * gsmm);
int gsm_modem_call_contact(GSMModem * gsmm, GSMCallType calltype,
		unsigned int index);
int gsm_modem_call_hangup(GSMModem * gsmm);
int gsm_modem_call_last(GSMModem * gsmm, GSMCallType calltype);
int gsm_modem_call_reject(GSMModem * gsmm);

int gsm_modem_contact_delete(GSMModem * gsmm, unsigned int index);
int gsm_modem_contact_edit(GSMModem * gsmm, unsigned int index,
		char const * name, char const * number);
int gsm_modem_contact_new(GSMModem * gsmm, char const * name,
		char const * number);

int gsm_modem_enter_sim_pin(GSMModem * gsmm, char const * code);

int gsm_modem_get_battery_charge(GSMModem * gsmm);
int gsm_modem_get_contact_list(GSMModem * gsmm);
int gsm_modem_get_contacts(GSMModem * gsmm, unsigned int start,
		unsigned int end);
int gsm_modem_get_message_list(GSMModem * gsmm, GSMMessageList list);
int gsm_modem_get_message(GSMModem * gsmm, unsigned int index);
int gsm_modem_get_model(GSMModem * gsmm);
int gsm_modem_get_operator(GSMModem * gsmm);
int gsm_modem_get_registration(GSMModem * gsmm);
int gsm_modem_get_signal_level(GSMModem * gsmm);

int gsm_modem_is_alive(GSMModem * gsmm);
int gsm_modem_is_call_waiting_control(GSMModem * gsmm);
int gsm_modem_is_functional(GSMModem * gsmm);
int gsm_modem_is_gprs_attached(GSMModem * gsmm);
int gsm_modem_is_mute(GSMModem * gsmm);
int gsm_modem_is_phone_active(GSMModem * gsmm);
int gsm_modem_is_pin_needed(GSMModem * gsmm);
int gsm_modem_is_pin_valid(GSMModem * gsmm);
int gsm_modem_is_registered(GSMModem * gsmm);

int gsm_modem_message_delete(GSMModem * gsmm, unsigned int index);
int gsm_modem_message_send(GSMModem * gsmm, char const * number,
		GSMModemAlphabet alphabet, char const * text, size_t length);

int gsm_modem_reset(GSMModem * gsmm);

int gsm_modem_set_call_presentation(GSMModem * gsmm, gboolean set);
int gsm_modem_set_call_waiting_control(GSMModem * gsmm, gboolean unsollicited);
int gsm_modem_set_echo(GSMModem * gsmm, gboolean echo);
int gsm_modem_set_extended_errors(GSMModem * gsmm, gboolean extended);
int gsm_modem_set_extended_ring_reports(GSMModem * gsmm, gboolean extended);
int gsm_modem_set_line_presentation(GSMModem * gsmm, gboolean set);
int gsm_modem_set_functional(GSMModem * gsmm, gboolean functional);
int gsm_modem_set_gprs_attachment(GSMModem * gsmm, gboolean set);
int gsm_modem_set_gprs_registration_report(GSMModem * gsmm,
		GSMRegistrationReport report);
int gsm_modem_set_message_format(GSMModem * gsmm, GSMMessageFormat format);
int gsm_modem_set_message_indications(GSMModem * gsmm, GSMMessageMode mode,
		gboolean unsollicited);
int gsm_modem_set_mute(GSMModem * gsmm, gboolean mute);
int gsm_modem_set_operator_format(GSMModem * gsmm, GSMOperatorFormat format);
int gsm_modem_set_operator_mode(GSMModem * gsmm, GSMOperatorMode mode);
int gsm_modem_set_registration_report(GSMModem * gsmm,
		GSMRegistrationReport report);
int gsm_modem_set_supplementary_service_notifications(GSMModem * gsmm,
		gboolean intermediate, gboolean unsollicited);
int gsm_modem_set_verbose(GSMModem * gsmm, gboolean verbose);

#endif /* !PHONE_MODEM_H */
