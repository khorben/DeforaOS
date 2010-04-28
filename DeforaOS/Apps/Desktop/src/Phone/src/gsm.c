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



#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <glib.h>
#include "phone.h"
#include "gsm.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* GSM */
/* private */
/* types */
typedef enum _GSMPriority
{
	GSM_PRIORITY_LOW = 0, GSM_PRIORITY_NORMAL, GSM_PRIORITY_HIGH
} GSMPriority;

typedef struct _GSMCommand
{
	GSMPriority priority;
	char * command;
	/* XXX also track if was written? more robust when tracking answers */
} GSMCommand;

typedef enum _GSMMode
{
	GSM_MODE_INIT = 0, GSM_MODE_COMMAND
} GSMMode;

typedef struct _GSMTrigger
{
	char const * trigger;
	size_t trigger_cnt;
	int (*callback)(GSM * gsm, char const * result);
} GSMTrigger;

struct _GSM
{
	/* settings */
	char * device;
	unsigned int baudrate;
	unsigned int retry;

	/* callback */
	GSMCallback callback;
	gpointer callback_data;
	GSMEvent event;

	/* queue */
	GSList * queue;

	/* internal */
	GSMMode mode;
	guint source;
	GIOChannel * channel;
	char * rd_buf;
	size_t rd_buf_cnt;
	guint rd_source;
	char * wr_buf;
	size_t wr_buf_cnt;
	guint wr_source;
};


/* variables */
/* ANSWERS */
static char const * _gsm_errors[] = { "ERROR", "NO CARRIER", NULL };

/* CME ERROR */
static struct
{
	int code;
	char const * error;
} _gsm_cme_errors[] =
{
	{ 0,	N_("Phone failure")				},
	{ 1,	N_("No connection to phone")			},
	{ 3,	N_("Operation not allowed")			},
	{ 4,	N_("Operation not supported")			},
	{ 10,	N_("SIM not inserted")				},
	{ 11,	N_("SIM PIN required")				},
	{ 12,	N_("SIM PUK required")				},
	{ 13,	N_("SIM failure")				},
	{ 14,	N_("SIM busy")					},
	{ 15,	N_("SIM wrong")					},
	{ 20,	N_("Memory full")				},
	{ 21,	N_("Invalid index")				},
	{ 30,	N_("No network service")			},
	{ 31,	N_("Network timeout")				},
	{ 32,	N_("Network not allowed - emergency calls only")},
	{ 0,	NULL						}
};


/* prototypes */
static int _is_code(char const * code);
static int _is_figure(int c);
static int _is_number(char const * number);

/* commands */
static GSMCommand * _gsm_command_new(GSMPriority priority,
		char const * command);
static void _gsm_command_delete(GSMCommand * command);

/* events */
static int _gsm_event(GSM * gsm, GSMEventType type, ...);
static int _gsm_event_send(GSM * gsm, GSMEventType type);

/* modem commands */
static int _gsm_modem_call(GSM * gsm, GSMCallType calltype,
		char const * number);
static int _gsm_modem_call_contact(GSM * gsm, GSMCallType calltype,
		unsigned int index);
static int _gsm_modem_call_last(GSM * gsm, GSMCallType calltype);
static int _gsm_modem_enter_pin(GSM * gsm, char const * code);
static int _gsm_modem_get_contact_list(GSM * gsm);
static int _gsm_modem_get_contacts(GSM * gsm, unsigned int start,
		unsigned int end);
static int _gsm_modem_get_message_list(GSM * gsm);
static int _gsm_modem_get_messages(GSM * gsm, unsigned int start,
		unsigned int end);
static int _gsm_modem_get_operator(GSM * gsm);
static int _gsm_modem_get_registration(GSM * gsm);
static int _gsm_modem_get_signal_level(GSM * gsm);
static int _gsm_modem_is_pin_needed(GSM * gsm);
static int _gsm_modem_hangup(GSM * gsm);
static int _gsm_modem_report_registration(GSM * gsm, gboolean report);
static int _gsm_modem_set_echo(GSM * gsm, gboolean echo);
static int _gsm_modem_set_operator_format(GSM * gsm, GSMOperatorFormat format);

/* parsing */
static int _gsm_parse(GSM * gsm);
static int _gsm_parse_line(GSM * gsm, char const * line, gboolean * answered);

/* queue management */
static int _gsm_queue_command(GSM * gsm, GSMPriority priority,
		char const * command);
static void _gsm_queue_flush(GSM * gsm);
static void _gsm_queue_pop(GSM * gsm);
static int _gsm_queue_push(GSM * gsm);

/* triggers */
static int _gsm_trigger_cme_error(GSM * gsm, char const * result);
static int _gsm_trigger_cmgl(GSM * gsm, char const * result);
static int _gsm_trigger_cops(GSM * gsm, char const * result);
static int _gsm_trigger_cpbr(GSM * gsm, char const * result);
static int _gsm_trigger_cpin(GSM * gsm, char const * result);
static int _gsm_trigger_creg(GSM * gsm, char const * result);
static int _gsm_trigger_csq(GSM * gsm, char const * result);

/* triggers */
static GSMTrigger _gsm_triggers[] =
{
#define GSM_TRIGGER(trigger, callback) \
	{ trigger, sizeof(trigger) - 1, _gsm_trigger_ ## callback }
	GSM_TRIGGER("+CME ERROR: ",	cme_error),
	GSM_TRIGGER("+CMGL: ",		cmgl),
	GSM_TRIGGER("+COPS: ",		cops),
	GSM_TRIGGER("+CPBR: ",		cpbr),
	GSM_TRIGGER("+CPIN: ",		cpin),
	GSM_TRIGGER("+CREG: ",		creg),
	GSM_TRIGGER("+CSQ: ",		csq),
	{ NULL, 0, NULL }
};

/* callbacks */
static gboolean _on_reset(gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* functions */
/* gsm_new */
static unsigned int _new_baudrate(unsigned int baudrate);

GSM * gsm_new(char const * device, unsigned int baudrate)
{
	GSM * gsm;

	if(device == NULL)
		return NULL;
	if((gsm = malloc(sizeof(*gsm))) == NULL)
		return NULL;
	/* settings */
	gsm->device = strdup(device);
	gsm->baudrate = _new_baudrate(baudrate);
	gsm->retry = 1000;
	/* callback */
	gsm->callback = NULL;
	gsm->callback_data = NULL;
	memset(&gsm->event, 0, sizeof(gsm->event));
	/* queue */
	gsm->queue = NULL;
	/* internal */
	gsm->mode = GSM_MODE_INIT;
	gsm->source = 0;
	gsm->channel = NULL;
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	gsm->rd_source = 0;
	gsm->wr_buf = NULL;
	gsm->wr_buf_cnt = 0;
	gsm->wr_source = 0;
	/* error checking */
	if(gsm->device == NULL || gsm->baudrate == 0)
	{
		gsm_delete(gsm);
		return NULL;
	}
	gsm_reset(gsm, 0);
	return gsm;
}

static unsigned int _new_baudrate(unsigned int baudrate)
{
	char buf[256];

	switch(baudrate)
	{
		case B1200:	case B2400:	case B4800:	case B9600:
		case B19200:	case B38400:
#ifdef B76800
		case B76800:
#endif
#ifdef B14400
		case B14400:
#endif
#ifdef B28800
		case B28800:
#endif
		case B57600:	case B115200:
		case B460800:	case B921600:
			break;
		default:
			snprintf(buf, sizeof(buf), "%u%s", baudrate,
					_(": Unknown baudrate"));
			return phone_error(NULL, buf, baudrate);
	}
	return baudrate;
}


/* gsm_delete */
void gsm_delete(GSM * gsm)
{
	if(gsm->rd_source != 0)
		g_source_remove(gsm->rd_source);
	gsm->rd_source = 0;
	_gsm_queue_flush(gsm);
	free(gsm->device);
	free(gsm);
}


/* accessors */
/* gsm_get_retry */
unsigned int gsm_get_retry(GSM * gsm)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %u\n", __func__, gsm->retry);
#endif
	return gsm->retry;
}


/* gsm_set_callback */
void gsm_set_callback(GSM * gsm, GSMCallback callback, gpointer data)
{
	gsm->callback = callback;
	gsm->callback_data = data;
}


/* gsm_set_operator_format */
void gsm_set_operator_format(GSM * gsm, GSMOperatorFormat format)
{
	_gsm_modem_set_operator_format(gsm, format);
}


/* gsm_set_retry */
void gsm_set_retry(GSM * gsm, unsigned int retry)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, retry);
#endif
	gsm->retry = retry;
}


/* useful */
/* gsm_call */
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number)
{
	if(number == NULL)
		return _gsm_modem_call_last(gsm, calltype);
	return _gsm_modem_call(gsm, calltype, number);
}


/* gsm_call_contact */
int gsm_call_contact(GSM * gsm, GSMCallType calltype, unsigned int index)
{
	return _gsm_modem_call_contact(gsm, calltype, index);
}


/* gsm_enter_pin */
int gsm_enter_pin(GSM * gsm, char const * code)
{
	if(code == NULL)
		return _gsm_modem_is_pin_needed(gsm);
	return _gsm_modem_enter_pin(gsm, code);
}


/* gsm_fetch_contact_list */
int gsm_fetch_contact_list(GSM * gsm)
{
	return _gsm_modem_get_contact_list(gsm);
}


/* gsm_fetch_contacts */
int gsm_fetch_contacts(GSM * gsm, unsigned int start, unsigned int end)
{
	return _gsm_modem_get_contacts(gsm, start, end);
}


/* gsm_fetch_message_list */
int gsm_fetch_message_list(GSM * gsm)
{
	return _gsm_modem_get_message_list(gsm);
}


/* gsm_fetch_messages */
int gsm_fetch_messages(GSM * gsm, unsigned int start, unsigned int end)
{
	return _gsm_modem_get_messages(gsm, start, end);
}


/* gsm_fetch_operator */
int gsm_fetch_operator(GSM * gsm)
{
	return _gsm_modem_get_operator(gsm);
}


/* gsm_fetch_registration */
int gsm_fetch_registration(GSM * gsm)
{
	return _gsm_modem_get_registration(gsm);
}


/* gsm_fetch_signal_level */
int gsm_fetch_signal_level(GSM * gsm)
{
	return _gsm_modem_get_signal_level(gsm);
}


/* gsm_hangup */
int gsm_hangup(GSM * gsm)
{
	return _gsm_modem_hangup(gsm);
}


/* gsm_is_pin_needed */
int gsm_is_pin_needed(GSM * gsm)
{
	return _gsm_modem_is_pin_needed(gsm);
}


/* gsm_report_registration */
int gsm_report_registration(GSM * gsm, int report)
{
	int ret;

	ret = _gsm_modem_report_registration(gsm, (report != 0) ? TRUE
			: FALSE);
	ret |= gsm_fetch_registration(gsm);
	return ret;
}


/* gsm_reset */
void gsm_reset(GSM * gsm, unsigned int delay)
{
	_gsm_queue_flush(gsm);
	if(delay > 0)
		gsm->source = g_timeout_add(delay, _on_reset, gsm);
	else
		gsm->source = g_idle_add(_on_reset, gsm);
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


/* commands */
/* gsm_command_new */
static GSMCommand * _gsm_command_new(GSMPriority priority,
		char const * command)
{
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, \"%s\")\n", __func__, priority, command);
#endif
	if((gsmc = malloc(sizeof(*gsmc))) == NULL)
		return NULL; /* XXX report error */
	gsmc->priority = priority;
	gsmc->command = strdup(command);
	/* check errors */
	if(gsmc->command == NULL)
	{
		_gsm_command_delete(gsmc);
		return NULL;
	}
	return gsmc;
}


/* gsm_command_delete */
static void _gsm_command_delete(GSMCommand * gsmc)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	free(gsmc->command);
	free(gsmc);
}


/* events */
/* gsm_event */
static int _gsm_event(GSM * gsm, GSMEventType type, ...)
{
	va_list ap;
	GSMEvent * event = &gsm->event;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, type);
#endif
	va_start(ap, type);
	switch(type)
	{
		case GSM_EVENT_TYPE_ERROR:
			event->error.error = va_arg(ap, GSMError);
			event->error.message = va_arg(ap, char *);
			break;
		case GSM_EVENT_TYPE_CONTACT:
			event->contact.index = va_arg(ap, unsigned int);
			event->contact.name = va_arg(ap, char *);
			event->contact.number = va_arg(ap, char *);
			break;
		case GSM_EVENT_TYPE_CONTACT_LIST:
			event->contact_list.start = va_arg(ap, unsigned int);
			event->contact_list.end = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_MESSAGE_LIST:
			event->message_list.start = va_arg(ap, unsigned int);
			event->message_list.end = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_OPERATOR:
			event->operator.mode = va_arg(ap, GSMOperatorMode);
			event->operator.format = va_arg(ap, GSMOperatorFormat);
			event->operator.operator = va_arg(ap, char *);
			event->operator.lai = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_REGISTRATION:
			event->registration.n = va_arg(ap, unsigned int);
			event->registration.stat = va_arg(ap, unsigned int);
			event->registration.area = va_arg(ap, unsigned int);
			event->registration.cell = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_SIGNAL_LEVEL:
			event->signal_level.level = va_arg(ap, gdouble);
			break;
		case GSM_EVENT_TYPE_STATUS:
			event->status.status = va_arg(ap, GSMStatus);
			break;
		default:
			va_end(ap);
			return 1;
	}
	va_end(ap);
	return _gsm_event_send(gsm, type);
}


/* gsm_event_send */
static int _gsm_event_send(GSM * gsm, GSMEventType type)
{
	int ret;
	char buf[80];

	gsm->event.type = type;
	ret = gsm->callback(&gsm->event, gsm->callback_data);
	if(ret == 0)
		return 0;
	snprintf(buf, sizeof(buf), "%u: %s", type, _("Event not handled"));
	return phone_error(NULL, buf, ret);
}


/* modem commands */
/* gsm_modem_call */
static int _gsm_modem_call(GSM * gsm, GSMCallType calltype, char const * number)
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
	ret = _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, buf);
	free(buf);
	return ret;
}


/* gsm_modem_call_contact */
static int _gsm_modem_call_contact(GSM * gsm, GSMCallType calltype,
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
	return _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, buf);
}


/* gsm_modem_call_last */
static int _gsm_modem_call_last(GSM * gsm, GSMCallType calltype)
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
	return _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, cmd);
}


/* gsm_modem_enter_pin */
static int _gsm_modem_enter_pin(GSM * gsm, char const * code)
{
	int ret;
	char const cmd[] = "AT+CPIN=";
	size_t len;
	char * buf;

	if(!_is_code(code))
		return 1;
	len = sizeof(cmd) + strlen(code);
	if((buf = malloc(len)) == NULL)
		return 1;
	snprintf(buf, len, "%s%s", cmd, code);
	ret = _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, buf);
	free(buf);
	return ret;
}


/* gsm_modem_get_contact_list */
static int _gsm_modem_get_contact_list(GSM * gsm)
{
	char const cmd[] = "AT+CPBR=?";

	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_get_contacts */
static int _gsm_modem_get_contacts(GSM * gsm, unsigned int start,
		unsigned int end)
{
	char cmd[32];
	
	snprintf(cmd, sizeof(cmd), "%s%u,%u", "AT+CPBR=", start, end);

	return _gsm_queue_command(gsm, GSM_PRIORITY_LOW, cmd);
}


/* gsm_modem_get_message_list */
static int _gsm_modem_get_message_list(GSM * gsm)
{
	char const cmd[] = "AT+CMGL=?";

	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_get_messages */
static int _gsm_modem_get_messages(GSM * gsm, unsigned int start,
		unsigned int end)
{
	char cmd[32];
	
	snprintf(cmd, sizeof(cmd), "%s%u,%u", "AT+CMGR=", start, end);

	return _gsm_queue_command(gsm, GSM_PRIORITY_LOW, cmd);
}


/* gsm_modem_get_operator */
static int _gsm_modem_get_operator(GSM * gsm)
{
	char const cmd[] = "AT+COPS?";

	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_get_registration */
static int _gsm_modem_get_registration(GSM * gsm)
{
	char const cmd[] = "AT+CREG?";

	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_get_signal_level */
static int _gsm_modem_get_signal_level(GSM * gsm)
{
	char const cmd[] = "AT+CSQ";

	return _gsm_queue_command(gsm, GSM_PRIORITY_LOW, cmd);
}


/* gsm_modem_hangup */
static int _gsm_modem_hangup(GSM * gsm)
{
	char const cmd[] = "ATH";

	return _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, cmd);
}


/* gsm_modem_is_pin_needed */
static int _gsm_modem_is_pin_needed(GSM * gsm)
{
	char const cmd[] = "AT+CPIN?";

	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_report_registration */
static int _gsm_modem_report_registration(GSM * gsm, gboolean report)
{
	char cmd[] = "AT+CREG=X";

	cmd[8] = report ? '1' : '0';
	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_modem_reset */
static int _gsm_modem_reset(GSM * gsm)
{
	char const cmd[] = "ATZ";

	return _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, cmd);
}


/* gsm_modem_set_echo */
static int _gsm_modem_set_echo(GSM * gsm, gboolean echo)
{
	char cmd[] = "ATE?";

	cmd[3] = echo ? '1' : '0';
	return _gsm_queue_command(gsm, GSM_PRIORITY_HIGH, cmd);
}


/* gsm_modem_set_operator_format */
static int _gsm_modem_set_operator_format(GSM * gsm, GSMOperatorFormat format)
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
	return _gsm_queue_command(gsm, GSM_PRIORITY_NORMAL, cmd);
}


/* gsm_parse */
static int _parse_do(GSM * gsm);

static int _gsm_parse(GSM * gsm)
{
	int ret = 0;
	size_t i = 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%zu\n", __func__, gsm->rd_buf_cnt);
#endif
	while(i < gsm->rd_buf_cnt)
	{
		if(gsm->rd_buf[i++] != '\r')
			continue;
		if(i == gsm->rd_buf_cnt)
			break;
		if(gsm->rd_buf[i] != '\n')
			continue;
		gsm->rd_buf[i++ - 1] = '\0';
		if(gsm->rd_buf[0] != '\0')
			ret |= _parse_do(gsm);
		gsm->rd_buf_cnt -= i;
		memmove(gsm->rd_buf, &gsm->rd_buf[i], gsm->rd_buf_cnt);
		if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt)) != NULL)
			gsm->rd_buf = p; /* we can ignore errors */
		i = 0;
	}
	return ret;
}

static int _parse_do(GSM * gsm)
{
	gboolean answered = FALSE;

	if(gsm->mode == GSM_MODE_INIT)
	{
		if(strcmp(gsm->rd_buf, "OK") != 0)
			return 0;
		g_source_remove(gsm->source);
		gsm->source = 0;
		gsm->mode = GSM_MODE_COMMAND;
		_gsm_modem_set_echo(gsm, FALSE);
		_gsm_modem_set_operator_format(gsm, GSM_OPERATOR_FORMAT_LONG);
		_gsm_event(gsm, GSM_EVENT_TYPE_STATUS, GSM_STATUS_INITIALIZED);
		_gsm_queue_push(gsm);
	}
	else if(gsm->mode == GSM_MODE_COMMAND)
	{
		_gsm_parse_line(gsm, gsm->rd_buf, &answered);
		if(answered)
		{
			_gsm_queue_pop(gsm);
			_gsm_queue_push(gsm);
		}
	}
	return 0;
}


/* gsm_parse_line */
static int _gsm_parse_line(GSM * gsm, char const * line, gboolean * answered)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(answered != NULL)
		*answered = FALSE;
	if(strncmp(line, "AT", 2) == 0) /* ignore echo (tighter check?) */
		return 0;
	if(strcmp(line, "OK") == 0)
	{
		if(answered != NULL)
			*answered = TRUE;
		return 0;
	}
	for(i = 0; _gsm_errors[i] != NULL; i++)
	{
		if(strcmp(_gsm_errors[i], line) != 0)
			continue;
		if(answered != NULL)
			*answered = TRUE;
		_gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_UNKNOWN, line);
		return 0;
	}
	for(i = 0; _gsm_triggers[i].trigger != NULL; i++)
		if(strncmp(line, _gsm_triggers[i].trigger,
					_gsm_triggers[i].trigger_cnt) == 0)
			return _gsm_triggers[i].callback(gsm,
					&line[_gsm_triggers[i].trigger_cnt]);
	/* XXX not handled... */
	return 1;
}


/* queue management */
/* gsm_queue_command */
static int _gsm_queue_command(GSM * gsm, GSMPriority priority,
		char const * command)
{
	GSMCommand * gsmc;
	GSList * l;
	GSMCommand * p;

	if(command == NULL || command[0] == '\0')
		return 1;
	if((gsmc = _gsm_command_new(priority, command)) == NULL)
		return 1;
	for(l = gsm->queue; l != NULL; l = l->next)
	{
		p = l->data;
		if(p->priority < priority)
			break;
	}
	if(l != NULL)
		gsm->queue = g_slist_insert_before(gsm->queue, l, gsmc);
	else if(gsm->queue == NULL && gsm->mode == GSM_MODE_COMMAND)
	{
		gsm->queue = g_slist_append(gsm->queue, gsmc);
		_gsm_queue_push(gsm);
	}
	else if(gsm->mode == GSM_MODE_INIT && gsm->wr_source == 0)
	{
		gsm->queue = g_slist_append(gsm->queue, gsmc);
		_gsm_queue_push(gsm);
	}
	else
		gsm->queue = g_slist_append(gsm->queue, gsmc);
	return 0;
}


/* _gsm_queue_flush */
static void _gsm_queue_flush(GSM * gsm)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(; gsm->queue != NULL; gsm->queue = g_slist_delete_link(gsm->queue,
				gsm->queue))
		_gsm_command_delete(gsm->queue->data);
	free(gsm->rd_buf);
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	free(gsm->wr_buf);
	gsm->wr_buf = NULL;
	gsm->wr_buf_cnt = 0;
	if(gsm->wr_source != 0)
	{
		g_source_remove(gsm->wr_source);
		gsm->wr_source = 0;
	}
	if(gsm->source != 0)
	{
		g_source_remove(gsm->source);
		gsm->source = 0;
	}
}


/* gsm_queue_pop */
static void _gsm_queue_pop(GSM * gsm)
{
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(gsm->queue == NULL)
		return;
	gsmc = gsm->queue->data;
	_gsm_command_delete(gsmc);
	gsm->queue = g_slist_remove(gsm->queue, gsmc);
	if(gsm->mode != GSM_MODE_COMMAND)
		return;
}


/* gsm_queue_push */
static int _gsm_queue_push(GSM * gsm)
{
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(gsm->queue == NULL)
		return 0;
	gsmc = gsm->queue->data;
	gsm->wr_buf_cnt = strlen(gsmc->command) + 2;
	if((gsm->wr_buf = malloc(gsm->wr_buf_cnt + 1)) == NULL)
		return 1;
	snprintf(gsm->wr_buf, gsm->wr_buf_cnt + 1, "%s%s", gsmc->command,
			"\r\n");
	if(gsm->channel != NULL && gsm->wr_source == 0)
		gsm->wr_source = g_io_add_watch(gsm->channel, G_IO_OUT,
				_on_watch_can_write, gsm);
	return 0;
}


/* triggers */
/* gsm_trigger_cme_error */
static int _gsm_trigger_cme_error(GSM * gsm, char const * result)
{
	int code;
	char * p;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	code = strtol(result, &p, 10);
	if(result[0] == '\0' || *p != '\0')
		return 1;
	for(i = 0; _gsm_cme_errors[i].error != NULL; i++)
		if(_gsm_cme_errors[i].code == code)
			break;
	if(_gsm_cme_errors[i].error == NULL)
		return 1;
	/* FIXME implement errors */
	return _gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_UNKNOWN,
			_(_gsm_cme_errors[i].error));
}


/* gsm_trigger_cmgl */
static int _gsm_trigger_cmgl(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "(%u-%u)", &gsm->event.message_list.start,
				&gsm->event.message_list.end) != 2)
		return 1;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_MESSAGE_LIST);
}


/* gsm_trigger_cops */
static int _gsm_trigger_cops(GSM * gsm, char const * result)
{
	char operator[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	gsm->event.operator.operator = operator;
	if(sscanf(result, "%u,%u,\"%31[^\"]\",%u", &gsm->event.operator.mode,
				&gsm->event.operator.format, operator,
				&gsm->event.operator.lai) == 4)
		return _gsm_event_send(gsm, GSM_EVENT_TYPE_OPERATOR);
	return 1;
}


/* _gsm_trigger_cpbr */
static int _gsm_trigger_cpbr(GSM * gsm, char const * result)
{
	unsigned int start;
	unsigned int end;
	unsigned int index;
	char number[32];
	char name[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "(%u-%u)", &start, &end) == 2)
		return _gsm_event(gsm, GSM_EVENT_TYPE_CONTACT_LIST, start, end);
	if(sscanf(result, "%u,\"%31[^\"]\",145,\"%31[^\"]\"", &index, number,
				name) == 3
			|| sscanf(result, "%u,\"%31[^\"]\",129,\"%31[^\"]\"",
				&index, number, name) == 3)
	{
		number[sizeof(number) - 1] = '\0';
		name[sizeof(name) - 1] = '\0';
		return _gsm_event(gsm, GSM_EVENT_TYPE_CONTACT, index, name,
				number);
	}
	return 1;
}


/* _gsm_trigger_cpin */
static int _gsm_trigger_cpin(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(strcmp(result, "READY") == 0)
		return _gsm_event(gsm, GSM_EVENT_TYPE_STATUS, GSM_STATUS_READY);
	if(strcmp(result, "SIM PIN") == 0)
		return _gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_SIM_PIN_REQUIRED, NULL);
	else
		/* XXX nicer message */
		return _gsm_event(gsm, GSM_EVENT_TYPE_ERROR, result);
}


/* gsm_trigger_creg */
static int _gsm_trigger_creg(GSM * gsm, char const * result)
{
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if((res = sscanf(result, "%u,%u,%X,%X", &gsm->event.registration.n,
					&gsm->event.registration.stat,
					&gsm->event.registration.area,
					&gsm->event.registration.cell)) == 4)
		return _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	else if(res == 2)
	{
		gsm->event.registration.area = 0;
		gsm->event.registration.cell = 0;
		return _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	}
	return 1;
}


/* _gsm_trigger_csq */
static int _gsm_trigger_csq(GSM * gsm, char const * result)
{
	unsigned int rssi;
	unsigned int ber;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u,%u", &rssi, &ber) != 2)
		return 1;
	gsm->event.signal_level.level = rssi;
	if(rssi > 31)
		gsm->event.signal_level.level /= 0.0;
	else
		gsm->event.signal_level.level /= 32;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_SIGNAL_LEVEL);
}


/* callbacks */
/* on_reset */
static int _reset_do(int fd);
static gboolean _reset_settle(gpointer data);

static gboolean _on_reset(gpointer data)
{
	GSM * gsm = data;
	int fd;
	char buf[256];
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gsm->source = 0;
	if((fd = open(gsm->device, O_RDWR | O_NONBLOCK)) < 0
			|| _reset_do(fd) != 0)
	{
		snprintf(buf, sizeof(buf), "%s%s%s", gsm->device, ": ",
				strerror(errno));
		if(fd >= 0)
			close(fd);
		if(gsm->retry > 0)
			gsm->source = g_timeout_add(gsm->retry, _on_reset, gsm);
		return phone_error(NULL, buf, FALSE);
	}
	gsm->channel = g_io_channel_unix_new(fd);
	if((g_io_channel_set_encoding(gsm->channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
		fprintf(stderr, "%s%s\n", "phone: ", error->message);
	g_io_channel_set_buffered(gsm->channel, FALSE);
	gsm->rd_source = g_io_add_watch(gsm->channel, G_IO_IN,
			_on_watch_can_read, gsm);
	if(gsm->wr_buf_cnt > 0)
		gsm->wr_source = g_io_add_watch(gsm->channel, G_IO_OUT,
				_on_watch_can_write, gsm);
	gsm->source = g_timeout_add(500, _reset_settle, gsm);
	_reset_settle(gsm);
	return FALSE;
}

static int _reset_do(int fd)
{
	struct stat st;
	int fl;
	struct termios term;

	if(flock(fd, LOCK_EX | LOCK_NB) != 0)
		return 1;
	if(fstat(fd, &st) != 0)
		return 1;
	if(st.st_mode & S_IFCHR) /* character special */
	{
		if(tcgetattr(fd, &term) != 0)
			return 1;
		term.c_cflag |= CS8;
		term.c_cflag |= CREAD;
		term.c_cflag |= CLOCAL;
		term.c_iflag = (IGNPAR | IGNBRK);
		term.c_lflag = 0;
		term.c_oflag = 0;
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		if(tcsetattr(fd, TCSAFLUSH, &term) != 0)
			return 1;
	}
	fl = fcntl(fd, F_GETFL, 0);
	if(fcntl(fd, F_SETFL, fl & ~O_NONBLOCK) == -1)
		return 1;
	return 0;
}

static gboolean _reset_settle(gpointer data)
{
	GSM * gsm = data;

	_gsm_modem_reset(gsm);
	return TRUE;
}


/* on_watch_can_read */
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	GSM * gsm = data;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(condition != G_IO_IN || source != gsm->channel)
		return FALSE; /* should not happen */
	if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	gsm->rd_buf = p;
	status = g_io_channel_read_chars(source, &gsm->rd_buf[gsm->rd_buf_cnt],
			256, &cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: MODEM: ");
	fwrite(&gsm->rd_buf[gsm->rd_buf_cnt], sizeof(*p), cnt, stderr);
#endif
	gsm->rd_buf_cnt += cnt;
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			fprintf(stderr, "%s%s\n", "phone: ", error->message);
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			if(gsm->retry > 0)
				gsm_reset(gsm, gsm->retry);
			gsm->rd_source = 0;
			return FALSE;
	}
	_gsm_parse(gsm);
	return TRUE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	GSM * gsm = data;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%zu\n", __func__, gsm->wr_buf_cnt);
#endif
	if(condition != G_IO_OUT || source != gsm->channel)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, gsm->wr_buf, gsm->wr_buf_cnt,
			&cnt, &error);
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: PHONE: ");
	fwrite(gsm->wr_buf, sizeof(*p), cnt, stderr);
#endif
	if(cnt != 0) /* some data may have been written anyway */
	{
		gsm->wr_buf_cnt -= cnt;
		memmove(gsm->wr_buf, &gsm->wr_buf[cnt], gsm->wr_buf_cnt);
		if((p = realloc(gsm->wr_buf, gsm->wr_buf_cnt)) != NULL)
			gsm->wr_buf = p; /* we can ignore errors */
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			fprintf(stderr, "phone: %s\n", error->message);
		case G_IO_STATUS_EOF:
		default: /* should not happen */
			if(gsm->retry > 0)
				gsm_reset(gsm, gsm->retry);
			gsm->wr_source = 0;
			return FALSE;
	}
	if(gsm->wr_buf_cnt > 0) /* there is more data to write */
		return TRUE;
	gsm->wr_source = 0;
	if(gsm->mode == GSM_MODE_INIT)
		_gsm_queue_pop(gsm);
	return FALSE;
}
