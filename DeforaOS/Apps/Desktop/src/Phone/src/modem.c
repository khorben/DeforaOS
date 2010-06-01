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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "command.h"
#include "modem.h"


/* GSMModem */
/* private */
/* types */
struct _GSMModem
{
	GSM * gsm;
	unsigned long quirks;
};


/* prototypes */
static int _is_code(char const * code);
static int _is_figure(int c);
static int _is_number(char const * number);

static int _modem_call_do(GSM * gsm, char const * command);


/* public */
/* functions */
/* gsm_modem_new */
GSMModem * gsm_modem_new(GSM * gsm)
{
	GSMModem * gsmm;

	if((gsmm = malloc(sizeof(*gsmm))) == NULL)
		return NULL;
	gsmm->gsm = gsm;
	gsmm->quirks = 0;
	return gsmm;
}


/* gsm_modem_delete */
void gsm_modem_delete(GSMModem * gsmm)
{
	free(gsmm);
}


/* accessors */
void gsm_modem_set_quirks(GSMModem * gsmm, unsigned long quirks)
{
	gsmm->quirks = quirks;
}


/* useful */
/* gsm_modem_call */
int gsm_modem_call(GSMModem * gsmm, GSMCallType calltype, char const * number)
{
	int ret;
	char const cmd[] = "ATD";
	char const * suffix = "";
	size_t len;
	char * buf;

	switch(calltype)
	{
		case GSM_CALL_TYPE_DATA:
			break;
		case GSM_CALL_TYPE_VOICE:
			suffix = ";";
			break;
		default:
			return 1;
	}
	if(!_is_number(number))
		return 1;
	len = sizeof(cmd) + strlen(number) + strlen(suffix);
	if((buf = malloc(len)) == NULL)
		return 1;
	snprintf(buf, len, "%s%s%s", cmd, number, suffix);
	ret = _modem_call_do(gsmm->gsm, buf);
	free(buf);
	return ret;
}


/* gsm_modem_call_answer */
static void _modem_call_answer_callback(GSM * gsm);

int gsm_modem_call_answer(GSMModem * gsmm)
{
	char const cmd[] = "ATA";

	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_HIGH, cmd,
			GSM_ERROR_ANSWER_FAILED, _modem_call_answer_callback);
}

static void _modem_call_answer_callback(GSM * gsm)
{
	/* FIXME trigger a "established" event before closing if failed? */
	gsm_is_phone_active(gsm);
}


/* gsm_modem_call_contact */
int gsm_modem_call_contact(GSMModem * gsmm, GSMCallType calltype,
		unsigned int index)
{
	char const cmd[] = "ATD>";
	char const * suffix = "";
	char buf[32];

	switch(calltype)
	{
		case GSM_CALL_TYPE_DATA:
			break;
		case GSM_CALL_TYPE_VOICE:
			suffix = ";";
			break;
		default:
			return 1;
	}
	snprintf(buf, sizeof(buf), "%s%u%s", cmd, index, suffix);
	return _modem_call_do(gsmm->gsm, buf);
}


/* gsm_modem_call_hangup */
static void _modem_call_hangup_callback(GSM * gsm);

int gsm_modem_call_hangup(GSMModem * gsmm)
{
	char const cmd[] = "ATH";

	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_HIGH, cmd,
			GSM_ERROR_HANGUP_FAILED, _modem_call_hangup_callback);
}

static void _modem_call_hangup_callback(GSM * gsm)
{
	gsm_is_phone_active(gsm);
}


/* gsm_modem_call_last */
int gsm_modem_call_last(GSMModem * gsmm, GSMCallType calltype)
{
	char const cmddata[] = "ATDL";
	char const cmdvoice[] = "ATDL;";
	char const * cmd;

	switch(calltype)
	{
		case GSM_CALL_TYPE_DATA:
			cmd = cmddata;
			break;
		case GSM_CALL_TYPE_VOICE:
			cmd = cmdvoice;
			break;
		default:
			return 1;
	}
	return _modem_call_do(gsmm->gsm, cmd);
}


/* gsm_modem_call_reject */
static void _modem_call_reject_callback(GSM * gsm);

int gsm_modem_call_reject(GSMModem * gsmm)
{
	char const cmd[] = "AT+CHUP";

	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_HIGH, cmd,
			GSM_ERROR_REJECT_FAILED, _modem_call_reject_callback);
}

static void _modem_call_reject_callback(GSM * gsm)
{
	gsm_is_phone_active(gsm);
}


/* gsm_modem_enter_sim_pin */
static void _modem_enter_sim_pin_callback(GSM * gsm);

int gsm_modem_enter_sim_pin(GSMModem * gsmm, char const * code)
{
	int ret;
	char const cmd[] = "AT+CPIN=";
	size_t len;
	char * buf;

	if(!_is_code(code))
	{
		gsm_event(gsmm->gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_SIM_PIN_WRONG);
		return 1;
	}
	len = sizeof(cmd) + 1 + strlen(code) + 1;
	if((buf = malloc(len)) == NULL)
		return 1;
	if(gsmm->quirks & GSM_MODEM_QUIRK_CPIN_QUOTES)
		snprintf(buf, len, "%s\"%s\"", cmd, code);
	else
		snprintf(buf, len, "%s%s", cmd, code);
	ret = gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, buf,
			GSM_ERROR_SIM_PIN_WRONG, _modem_enter_sim_pin_callback);
	free(buf);
	return ret;
}

static void _modem_enter_sim_pin_callback(GSM * gsm)
{
	/* did it really work? */
	gsm_is_pin_valid(gsm);
}


/* gsm_modem_get_battery_charge */
int gsm_modem_get_battery_charge(GSMModem * gsmm)
{
	char const cmd[] = "AT+CBC";

	return gsm_queue_with_error(gsmm->gsm, cmd,
			GSM_ERROR_BATTERY_CHARGE_FAILED);
}


/* gsm_modem_get_contact_list */
int gsm_modem_get_contact_list(GSMModem * gsmm)
{
	char const cmd[] = "AT+CPBR=?";

	return gsm_queue_with_error(gsmm->gsm, cmd,
			GSM_ERROR_CONTACT_LIST_FAILED);
}


/* gsm_modem_get_contacts */
int gsm_modem_get_contacts(GSMModem * gsmm, unsigned int start,
		unsigned int end)
{
	char cmd[32];
	
	snprintf(cmd, sizeof(cmd), "%s%u,%u", "AT+CPBR=", start, end);
	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_LOW, cmd,
			GSM_ERROR_CONTACT_FETCH_FAILED, NULL);
}


/* gsm_modem_get_message_list */
int gsm_modem_get_message_list(GSMModem * gsmm, GSMMessageList list)
{
	char cmd[] = "AT+CMGL=X";

	switch(list)
	{
		case GSM_MESSAGE_LIST_UNREAD:
		case GSM_MESSAGE_LIST_READ:
		case GSM_MESSAGE_LIST_UNSENT:
		case GSM_MESSAGE_LIST_SENT:
		case GSM_MESSAGE_LIST_ALL:
			break;
		default:
			return 1;
	}
	if(gsm_modem_set_message_format(gsmm, GSM_MESSAGE_FORMAT_PDU) != 0)
		return 1;
	cmd[8] = list + '0';
	return gsm_queue_with_error(gsmm->gsm, cmd,
			GSM_ERROR_MESSAGE_LIST_FAILED);
}


/* gsm_modem_get_message */
int gsm_modem_get_message(GSMModem * gsmm, unsigned int index)
{
	GSMCommand * gsmc;
	char cmd[32];
	unsigned long i = index;

	if(gsm_modem_set_message_format(gsmm, GSM_MESSAGE_FORMAT_PDU) != 0)
		return 1;
	snprintf(cmd, sizeof(cmd), "%s%u", "AT+CMGR=", index);
	if((gsmc = gsm_command_new(cmd)) == NULL)
		return 1;
	gsm_command_set_priority(gsmc, GSM_PRIORITY_NORMAL);
	gsm_command_set_error(gsmc, GSM_ERROR_MESSAGE_FETCH_FAILED);
	gsm_command_set_data(gsmc, (void *)i); /* XXX ugly */
	/* XXX race condition here if the user forces out of PDU mode */
	if(gsm_queue_command(gsmm->gsm, gsmc) == 0)
		return 0;
	gsm_command_delete(gsmc);
	return 1;
}


/* gsm_modem_get_model */
int gsm_modem_get_model(GSMModem * gsmm)
{
	char const cmd[] = "AT+CGMM";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_get_operator */
int gsm_modem_get_operator(GSMModem * gsmm)
{
	char const cmd[] = "AT+COPS?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_get_registration */
int gsm_modem_get_registration(GSMModem * gsmm)
{
	char const cmd[] = "AT+CREG?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_get_signal_level */
int gsm_modem_get_signal_level(GSMModem * gsmm)
{
	char const cmd[] = "AT+CSQ";

	return gsm_queue_with_error(gsmm->gsm, cmd,
			GSM_ERROR_SIGNAL_LEVEL_FAILED);
}


/* gsm_modem_is_alive */
int gsm_modem_is_alive(GSMModem * gsmm)
{
	char const cmd[] = "AT";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_call_waiting_control */
int gsm_modem_is_call_waiting_control(GSMModem * gsmm)
{
	char const cmd[] = "AT+CCWA?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_functional */
int gsm_modem_is_functional(GSMModem * gsmm)
{
	char const cmd[] = "AT+CFUN?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_mute */
int gsm_modem_is_mute(GSMModem * gsmm)
{
	char const cmd[] = "AT+CMUT?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_phone_active */
int gsm_modem_is_phone_active(GSMModem * gsmm)
{
	char const cmd[] = "AT+CPAS";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_pin_needed */
int gsm_modem_is_pin_needed(GSMModem * gsmm)
{
	char const cmd[] = "AT+CPIN?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_is_pin_valid */
static void _modem_is_pin_valid_callback(GSM * gsm);

int gsm_modem_is_pin_valid(GSMModem * gsmm)
{
	char const cmd[] = "AT+CPIN?";

	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, cmd,
			GSM_ERROR_SIM_PIN_WRONG, _modem_is_pin_valid_callback);
}

static void _modem_is_pin_valid_callback(GSM * gsm)
{
	gsm_event(gsm, GSM_EVENT_TYPE_SIM_PIN_VALID);
}


/* gsm_modem_is_registered */
int gsm_modem_is_registered(GSMModem * gsmm)
{
	char const cmd[] = "AT+CREG?";

	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_reset */
int gsm_modem_reset(GSMModem * gsmm)
{
	char const cmd[] = "ATZ";

	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_HIGH, cmd,
			GSM_ERROR_RESET_FAILED, NULL);
}


/* gsm_modem_send_message */
static char * _number_to_address(char const * number);
static char * _text_to_data(char const * text, size_t length);
static char * _text_to_sept(char const * text, size_t length);

int gsm_modem_send_message(GSMModem * gsmm, char const * number,
		GSMModemAlphabet alphabet, char const * text, size_t length)
{
	int ret = 1;
	char const cmd1[] = "AT+CMGS=";
	char * buf1;
	unsigned long len1;
	char const cmd2[] = "1100";
	char * buf2;
	unsigned long len2;
	char * addr;
	char * data = NULL;
	char const pid[] = "00";
	char dcs[] = "0X";
	char const vp[] = "AA";
	GSMCommand * gsmc;

	if(!_is_number(number) || text == NULL
			|| gsm_modem_set_message_format(gsmm,
				GSM_MESSAGE_FORMAT_PDU) != 0)
		return gsm_event(gsmm->gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_MESSAGE_SEND_FAILED, NULL);
	switch(alphabet)
	{
		case GSM_MODEM_ALPHABET_DEFAULT:
			dcs[1] = '0';
			data = _text_to_sept(text, length);
			break;
		case GSM_MODEM_ALPHABET_DATA:
			dcs[1] = '4';
			data = _text_to_data(text, length);
			break;
	}
	addr = _number_to_address(number);
	len2 = sizeof(cmd2) + 2 + strlen(addr ? addr : "") + sizeof(pid)
		+ sizeof(dcs) + sizeof(vp) + 2 + strlen(data ? data : "") + 1;
	buf2 = malloc(len2);
	len1 = sizeof(cmd1) + 3;
	buf1 = malloc(len1);
	if(addr == NULL || data == NULL || buf1 == NULL || buf2 == NULL)
	{
		free(addr);
		free(data);
		free(buf1);
		free(buf2);
		return gsm_event(gsmm->gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_MESSAGE_SEND_FAILED, NULL);
	}
	fprintf(stderr, "DEBUG: len2=%lu\n", len2);
	if(number[0] == '+')
		number++;
	snprintf(buf2, len2, "%s%02lX%s%s%s%s%02lX%s\x1a", cmd2, strlen(number),
			addr, pid, dcs, vp, length, data);
	snprintf(buf1, len1, "%s%lu", cmd1, (len2 - 1) / 2);
	free(addr);
	free(data);
	if((gsmc = gsm_command_new(buf1)) != NULL
			&& (ret = gsm_queue_command(gsmm->gsm, gsmc)) == 0)
	{
		gsm_command_set_error(gsmc, GSM_ERROR_MESSAGE_SEND_FAILED);
		gsm_command_set_mode(gsmc, GSM_MODE_PDU);
		gsm_command_set_priority(gsmc, GSM_PRIORITY_HIGHEST);
		if((gsmc = gsm_command_new(buf2)) != NULL
				/* XXX if this fails we're stuck in PDU mode */
				&& (ret = gsm_queue_command(gsmm->gsm, gsmc))
				== 0)
		{
			gsm_command_set_error(gsmc,
					GSM_ERROR_MESSAGE_SEND_FAILED);
			/* this ensures that no command gets in between */
			gsm_command_set_priority(gsmc, GSM_PRIORITY_HIGHEST);
			gsm_command_set_timeout(gsmc, 60000);
		}
	}
	if(ret != 0)
		gsm_command_delete(gsmc);
	free(buf1);
	free(buf2);
	return ret;
}

static char * _number_to_address(char const * number)
{
	char * buf;
	size_t len;
	size_t i;

	len = 2 + strlen(number) + 2;
	if((buf = malloc(len)) == NULL)
		return NULL;
	snprintf(buf, len, "%02X", (number[0] == '+') ? 145 : 129);
	if(number[0] == '+')
		number++;
	for(i = 2; i < len; i+=2)
	{
		if(number[i - 2] == '\0')
			break;
		buf[i] = number[i - 1];
		buf[i + 1] = number[i - 2];
		if(number[i - 1] == '\0')
		{
			buf[i] = 'F';
			i += 2;
			break;
		}
	}
	buf[i] = '\0';
	return buf;
}

static char * _text_to_data(char const * text, size_t length)
{
	char const tab[16] = "0123456789ABCDEF";
	char * buf;
	size_t i;

	if((buf = malloc((length * 2) + 1)) == NULL)
		return NULL;
	for(i = 0; i < length; i++)
	{
		buf[i * 2] = tab[text[i] & 0x0f];
		buf[(i * 2) + 1] = tab[((text[i] & 0xf0) >> 4) & 0x0f];
	}
	buf[i * 2] = '\0';
	return buf;
}

/* this function is heavily inspired from gsmd, (c) 2007 OpenMoko, Inc. */
static char * _text_to_sept(char const * text, size_t length)
{
	char const tab[16] = "0123456789ABCDEF";
	unsigned char const * t = (unsigned char const *)text;
	char * buf;
	char * p;
	size_t i;
	unsigned char ch1;
	unsigned char ch2;
	int shift = 0;

	if((buf = malloc((length * 2) + 1)) == NULL)
		return NULL;
	p = buf;
	for(i = 0; i < length; i++)
	{
		ch1 = t[i] & 0x7f;
		ch1 = (ch1 >> shift);
		ch2 = t[i + 1] & 0x7f;
		ch2 = ch2 << (7 - shift);
		ch1 = ch1 | ch2;
		*(p++) = tab[(ch1 & 0xf0) >> 4];
		*(p++) = tab[ch1 & 0x0f];
		if(++shift == 7)
		{
			shift = 0;
			i++;
		}
	}
	*p = '\0';
	return buf;
}


/* gsm_modem_set_call_presentation */
int gsm_modem_set_call_presentation(GSMModem * gsmm, gboolean set)
{
	char cmd[] = "AT+CLIP=X";

	cmd[8] = set ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_call_waiting_control */
static void _modem_set_call_waiting_control_callback(GSM * gsm);

int gsm_modem_set_call_waiting_control(GSMModem * gsmm, gboolean unsollicited)
{
	char cmd[] = "AT+CCWA=X";

	cmd[8] = unsollicited ? '1' : '0';
	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, cmd,
			GSM_ERROR_CALL_WAITING_FAILED,
			_modem_set_call_waiting_control_callback);
}

static void _modem_set_call_waiting_control_callback(GSM * gsm)
{
	/* did it really work? */
	gsm_is_call_waiting_control(gsm);
}


/* gsm_modem_set_echo */
int gsm_modem_set_echo(GSMModem * gsmm, gboolean echo)
{
	char cmd[] = "ATEX";

	cmd[3] = echo ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_extended_errors */
int gsm_modem_set_extended_errors(GSMModem * gsmm, gboolean extended)
{
	char cmd[] = "AT+CMEE=X";

	cmd[8] = extended ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_extended_ring_reports */
int gsm_modem_set_extended_ring_reports(GSMModem * gsmm, gboolean extended)
{
	char cmd[] = "AT+CRC=X";

	cmd[7] = extended ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_line_presentation */
int gsm_modem_set_line_presentation(GSMModem * gsmm, gboolean set)
{
	char cmd[] = "AT+COLP=X";

	cmd[8] = set ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_functional */
static void _modem_set_functional_callback(GSM * gsm);

int gsm_modem_set_functional(GSMModem * gsmm, gboolean functional)
{
	char cmd[] = "AT+CFUN=X";

	cmd[8] = functional ? '1' : '0';
	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, cmd,
			GSM_ERROR_FUNCTIONAL_FAILED,
			_modem_set_functional_callback);
}

static void _modem_set_functional_callback(GSM * gsm)
{
	/* did it really work? */
	gsm_is_functional(gsm);
}


/* gsm_modem_set_message_format */
int gsm_modem_set_message_format(GSMModem * gsmm, GSMMessageFormat format)
{
	char cmd[] = "AT+CMGF=X";

	switch(format)
	{
		case GSM_MESSAGE_FORMAT_PDU:
		case GSM_MESSAGE_FORMAT_TEXT:
			break;
		default:
			return 1;
	}
	cmd[8] = format + '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_mute */
static void _modem_set_mute_callback(GSM * gsm);

int gsm_modem_set_mute(GSMModem * gsmm, gboolean mute)
{
	char cmd[] = "AT+CMUT=X";

	cmd[8] = mute ? '1' : '0';
	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, cmd,
			GSM_ERROR_MUTE_FAILED, _modem_set_mute_callback);
}

static void _modem_set_mute_callback(GSM * gsm)
{
	/* did it really work? */
	gsm_is_mute(gsm);
}


/* gsm_modem_set_operator_format */
int gsm_modem_set_operator_format(GSMModem * gsmm, GSMOperatorFormat format)
{
	char cmd[] = "AT+COPS=X,X";

	switch(format)
	{
		case GSM_OPERATOR_FORMAT_LONG:
		case GSM_OPERATOR_FORMAT_SHORT:
		case GSM_OPERATOR_FORMAT_LAI:
			break;
		default:
			return 1;
	}
	cmd[8] = GSM_OPERATOR_MODE_SET_FORMAT + '0';
	cmd[10] = format + '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_operator_mode */
static void _modem_set_operator_mode_callback(GSM * gsm);

int gsm_modem_set_operator_mode(GSMModem * gsmm, GSMOperatorMode mode)
{
	char cmd[] = "AT+COPS=X";

	switch(mode)
	{
		case GSM_OPERATOR_MODE_AUTOMATIC:
		case GSM_OPERATOR_MODE_MANUAL:
		case GSM_OPERATOR_MODE_DEREGISTER:
			break;
		case GSM_OPERATOR_MODE_MANUAL_WITH_FALLBACK:
			/* FIXME implement this one too */
		default:
			return 1;
	}
	cmd[8] = mode + '0';
	return gsm_queue_full(gsmm->gsm, GSM_PRIORITY_NORMAL, cmd,
			GSM_ERROR_OPERATOR_MODE_FAILED,
			_modem_set_operator_mode_callback);
}

static void _modem_set_operator_mode_callback(GSM * gsm)
{
	/* did it really work? */
	gsm_fetch_operator(gsm);
}


/* gsm_modem_set_registration_report */
int gsm_modem_set_registration_report(GSMModem * gsmm,
		GSMRegistrationReport report)
{
	char cmd[] = "AT+CREG=X";

	switch(report)
	{
		case GSM_REGISTRATION_REPORT_DISABLE_UNSOLLICITED:
		case GSM_REGISTRATION_REPORT_ENABLE_UNSOLLICITED:
		case GSM_REGISTRATION_REPORT_ENABLE_UNSOLLICITED_WITH_LOCATION:
			break;
		default:
			return 1;
	}
	cmd[8] = report + '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_supplementary_service_notifications */
int gsm_modem_set_supplementary_service_notifications(GSMModem * gsmm,
		gboolean intermediate, gboolean unsollicited)
{
	char cmd[] = "AT+CSSN=X,X";

	cmd[8] = intermediate ? '1' : '0';
	cmd[10] = unsollicited ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* gsm_modem_set_verbose */
int gsm_modem_set_verbose(GSMModem * gsmm, gboolean verbose)
{
	char cmd[] = "ATVX";

	cmd[3] = verbose ? '1' : '0';
	return (gsm_queue(gsmm->gsm, cmd) != NULL) ? 0 : 1;
}


/* private */
/* functions */
/* is_code */
static int _is_code(char const * code)
{
	int c;

	while((c = *(code++)) != '\0')
		if(!isdigit(c))
			return 0;
	return 1;
}


/* is_figure */
static int _is_figure(int c)
{
	if(c >= '0' && c <= '9')
		return 1;
	if(c == '*' || c == '+' || c == '#')
		return 1;
	return 0;
}


/* is_number */
static int _is_number(char const * number)
{
	if(number == NULL || number[0] == '\0')
		return 0;
	while(*number != '\0')
		if(!_is_figure(*(number++)))
			return 0;
	return 1;
}


/* modem_call_do */
static int _modem_call_do(GSM * gsm, char const * command)
{
	GSMCommand * gsmc;

	if((gsmc = gsm_command_new(command)) == NULL)
		return 1;
	gsm_command_set_priority(gsmc, GSM_PRIORITY_HIGH);
	gsm_command_set_error(gsmc, GSM_ERROR_CALL_FAILED);
	gsm_command_set_callback(gsmc, NULL); /* XXX check if active? */
	gsm_command_set_timeout(gsmc, 30000); /* XXX is it really necessary? */
	if(gsm_queue_command(gsm, gsmc) == 0)
		return 0;
	gsm_command_delete(gsmc);
	return 1;
}
