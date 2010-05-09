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
#include "command.h"
#include "modem.h"
#include "phone.h"
#include "gsm.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* GSM */
/* private */
/* types */
typedef int (*GSMTriggerCallback)(GSM * gsm, char const * result,
		gboolean * answered);

typedef struct _GSMTrigger
{
	char const * trigger;
	size_t trigger_cnt;
	GSMTriggerCallback callback;
} GSMTrigger;

struct _GSM
{
	/* settings */
	char * device;
	unsigned int baudrate;
	unsigned int retry;
	unsigned int hwflow;

	/* callback */
	GSMCallback callback;
	gpointer callback_data;
	GSMEvent event;
	GSMStatus status;

	/* queue */
	GSList * queue;

	/* internal */
	GSMModem * modem;
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
	{ 16,	N_("Incorrect password")			},
	{ 17,	N_("SIM PIN2 required")				},
	{ 18,	N_("SIM PUK2 required")				},
	{ 20,	N_("Memory full")				},
	{ 21,	N_("Invalid index")				},
	{ 22,	N_("Not found")					},
	{ 23,	N_("Memory failure")				},
	{ 24,	N_("Text string too long")			},
	{ 25,	N_("Invalid characters in text string")		},
	{ 26,	N_("Dial string too long")			},
	{ 27,	N_("Invalid characters in dial string")		},
	{ 30,	N_("No network service")			},
	{ 31,	N_("Network timeout")				},
	{ 32,	N_("Network not allowed - emergency calls only")},
	{ 100,	N_("Unknown error")				},
	{ 0,	NULL						}
};

/* CMS ERROR */
static struct
{
	int code;
	char const * error;
} _gsm_cms_errors[] =
{
	{ 300,	N_("ME failure")				},
	{ 301,	N_("SMS service of ME reserved")		},
	{ 302,	N_("Operation not allowed")			},
	{ 303,	N_("Operation not supported")			},
	{ 304,	N_("Invalid PDU mode parameter")		},
	{ 310,	N_("SIM not inserted")				},
	{ 311,	N_("SIM PIN required")				},
	{ 0,	NULL						}
};

/* models */
static struct
{
	char const * model;
	unsigned long quirks;
} _gsm_models[] =
{
	{ "\"Neo1973 GTA02 Embedded GSM Modem\"",
		GSM_MODEM_QUIRK_CPIN_QUOTES			},
	{ NULL,	0						}
};


/* prototypes */
/* events */
static int _gsm_event_send(GSM * gsm, GSMEventType type);
static int _gsm_event_set_status(GSM * gsm, GSMStatus status);

/* parsing */
static int _gsm_parse(GSM * gsm);
static int _gsm_parse_line(GSM * gsm, char const * line, gboolean * answered);

/* queue management */
static void _gsm_queue_flush(GSM * gsm);
static void _gsm_queue_pop(GSM * gsm);
static int _gsm_queue_push(GSM * gsm);

/* triggers */
static int _gsm_trigger_busy(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_cfun(GSM * gsm, char const * result);
static int _gsm_trigger_cgmm(GSM * gsm, char const * result);
static int _gsm_trigger_clip(GSM * gsm, char const * result);
static int _gsm_trigger_cme_error(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_cms_error(GSM * gsm, char const * result);
static int _gsm_trigger_cmgl(GSM * gsm, char const * result);
static int _gsm_trigger_cmgs(GSM * gsm, char const * result);
static int _gsm_trigger_cmti(GSM * gsm, char const * result);
static int _gsm_trigger_connect(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_cops(GSM * gsm, char const * result);
static int _gsm_trigger_cpas(GSM * gsm, char const * result);
static int _gsm_trigger_cpbr(GSM * gsm, char const * result);
static int _gsm_trigger_cpin(GSM * gsm, char const * result);
static int _gsm_trigger_creg(GSM * gsm, char const * result);
static int _gsm_trigger_cring(GSM * gsm, char const * result);
static int _gsm_trigger_csq(GSM * gsm, char const * result);
static int _gsm_trigger_no_answer(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_no_carrier(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_no_dialtone(GSM * gsm, char const * result,
		gboolean * answered);

/* triggers */
static GSMTrigger _gsm_triggers[] =
{
#define GSM_TRIGGER(trigger, callback) \
	{ trigger, sizeof(trigger) - 1, \
		(GSMTriggerCallback)_gsm_trigger_ ## callback }
	GSM_TRIGGER("BUSY",		busy),
	GSM_TRIGGER("+CFUN: ",		cfun),
	GSM_TRIGGER("+CGMM: ",		cgmm),
	GSM_TRIGGER("+CLIP: ",		clip),
	GSM_TRIGGER("+CME ERROR: ",	cme_error),
	GSM_TRIGGER("+CMS ERROR: ",	cms_error),
	GSM_TRIGGER("+CMGL: ",		cmgl),
	GSM_TRIGGER("+CMGS: ",		cmgs),
	GSM_TRIGGER("+CMTI: ",		cmti),
	GSM_TRIGGER("CONNECT",		connect),
	GSM_TRIGGER("+COPS: ",		cops),
	GSM_TRIGGER("+CPAS: ",		cpas),
	GSM_TRIGGER("+CPBR: ",		cpbr),
	GSM_TRIGGER("+CPIN: ",		cpin),
	GSM_TRIGGER("+CREG: ",		creg),
	GSM_TRIGGER("+CRING: ",		cring),
	GSM_TRIGGER("+CSQ: ",		csq),
	GSM_TRIGGER("NO ANSWER",	no_answer),
	GSM_TRIGGER("NO CARRIER",	no_carrier),
	GSM_TRIGGER("NO DIALTONE",	no_dialtone),
	{ NULL, 0, NULL }
};

/* callbacks */
static gboolean _on_reset(gpointer data);
static gboolean _on_timeout(gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* functions */
/* gsm_new */
static unsigned int _new_baudrate(unsigned int baudrate);

GSM * gsm_new(char const * device, unsigned int baudrate, unsigned int hwflow)
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
	gsm->hwflow = hwflow;
	/* callback */
	gsm->callback = NULL;
	gsm->callback_data = NULL;
	memset(&gsm->event, 0, sizeof(gsm->event));
	gsm->status = GSM_STATUS_UNKNOWN;
	/* queue */
	gsm->queue = NULL;
	/* internal */
	gsm->modem = gsm_modem_new(gsm);
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
	if(gsm->device == NULL || gsm->baudrate == 0 || gsm->modem == NULL)
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
		case 1200:
			return B1200;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
#ifdef B76800
		case 76800:
			return B76800;
#endif
#ifdef B14400
		case 14400:
			return B14400;
#endif
#ifdef B28800
		case 28800:
			return B28800;
#endif
		case 57600:
			return B57600;
		case 115200:
			return B115200;
		case 460800:
			return B460800;
		case 921600:
			return B921600;
		default:
			snprintf(buf, sizeof(buf), "%u%s", baudrate,
					_(": Unknown baudrate"));
			return phone_error(NULL, buf, baudrate);
	}
}


/* gsm_delete */
void gsm_delete(GSM * gsm)
{
	gsm_modem_delete(gsm->modem);
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


/* gsm_set_extended_errors */
int gsm_set_extended_errors(GSM * gsm, int extended)
{
	return gsm_modem_set_extended_errors(gsm->modem, (extended != 0) ? TRUE
			: FALSE);
}


/* gsm_set_extended_ring_reports */
int gsm_set_extended_ring_reports(GSM * gsm, gboolean extended)
{
	return gsm_modem_set_extended_ring_reports(gsm->modem, extended);
}


/* gsm_set_functional */
int gsm_set_functional(GSM * gsm, int functional)
{
	return gsm_modem_set_functional(gsm->modem, (functional != 0) ? TRUE
			: FALSE);
}


/* gsm_set_operator_format */
int gsm_set_operator_format(GSM * gsm, GSMOperatorFormat format)
{
	return gsm_modem_set_operator_format(gsm->modem, format);
}


/* gsm_set_operator_mode */
int gsm_set_operator_mode(GSM * gsm, GSMOperatorMode mode)
{
	return gsm_modem_set_operator_mode(gsm->modem, mode);
}


/* gsm_set_registration_report */
int gsm_set_registration_report(GSM * gsm, GSMRegistrationReport report)
{
	int ret;

	ret = gsm_modem_set_registration_report(gsm->modem, report);
	ret |= gsm_fetch_registration(gsm);
	return ret;
}


/* gsm_set_retry */
int gsm_set_retry(GSM * gsm, unsigned int retry)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, retry);
#endif
	gsm->retry = retry;
	return 0;
}


/* useful */
/* gsm_call */
int gsm_call(GSM * gsm, GSMCallType calltype, char const * number)
{
	if(number == NULL)
		return gsm_modem_call_last(gsm->modem, calltype);
	/* FIXME if the number is not valid try from the address book */
	return gsm_modem_call(gsm->modem, calltype, number);
}


/* gsm_call_answer */
int gsm_call_answer(GSM * gsm)
{
	return gsm_modem_call_answer(gsm->modem);
}


/* gsm_call_contact */
int gsm_call_contact(GSM * gsm, GSMCallType calltype, unsigned int index)
{
	return gsm_modem_call_contact(gsm->modem, calltype, index);
}


/* gsm_call_hangup */
int gsm_call_hangup(GSM * gsm)
{
	return gsm_modem_call_hangup(gsm->modem);
}


/* gsm_enter_sim_pin */
int gsm_enter_sim_pin(GSM * gsm, char const * code)
{
	if(code == NULL)
		return gsm_modem_is_pin_valid(gsm->modem);
	return gsm_modem_enter_sim_pin(gsm->modem, code);
}


/* gsm_event */
int gsm_event(GSM * gsm, GSMEventType type, ...)
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
			event->error.message = va_arg(ap, char const *);
			break;
		case GSM_EVENT_TYPE_CALL_PRESENTATION:
			event->call_presentation.number = va_arg(ap,
					char const *);
			event->call_presentation.format = va_arg(ap,
					unsigned int);
			break;
		case GSM_EVENT_TYPE_CONTACT:
			event->contact.index = va_arg(ap, unsigned int);
			event->contact.name = va_arg(ap, char const *);
			event->contact.number = va_arg(ap, char const *);
			break;
		case GSM_EVENT_TYPE_CONTACT_LIST:
			event->contact_list.start = va_arg(ap, unsigned int);
			event->contact_list.end = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_FUNCTIONAL:
			event->functional.functional = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_INCOMING_CALL:
			event->incoming_call.calltype = va_arg(ap,
					unsigned int);
			break;
		case GSM_EVENT_TYPE_INCOMING_MESSAGE:
			event->incoming_message.memory = va_arg(ap,
					char const *);
			event->incoming_message.index = va_arg(ap,
					unsigned int);
			break;
		case GSM_EVENT_TYPE_MESSAGE_LIST:
			event->message_list.start = va_arg(ap, unsigned int);
			event->message_list.end = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_MESSAGE_SENT:
			event->message_sent.mr = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_OPERATOR:
			event->operator.mode = va_arg(ap, GSMOperatorMode);
			event->operator.format = va_arg(ap, GSMOperatorFormat);
			event->operator.operator = va_arg(ap, char const *);
			event->operator.lai = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_PHONE_ACTIVITY:
			event->phone_activity.activity = va_arg(ap,
					GSMPhoneActivity);
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
		case GSM_EVENT_TYPE_SIM_PIN_VALID:
			break;
		case GSM_EVENT_TYPE_STATUS:
			event->status.status = va_arg(ap, GSMStatus);
			break;
	}
	va_end(ap);
	return _gsm_event_send(gsm, type);
}


/* gsm_fetch_contact_list */
int gsm_fetch_contact_list(GSM * gsm)
{
	return gsm_modem_get_contact_list(gsm->modem);
}


/* gsm_fetch_contacts */
int gsm_fetch_contacts(GSM * gsm, unsigned int start, unsigned int end)
{
	return gsm_modem_get_contacts(gsm->modem, start, end);
}


/* gsm_fetch_message_list */
int gsm_fetch_message_list(GSM * gsm)
{
	return gsm_modem_get_message_list(gsm->modem);
}


/* gsm_fetch_messages */
int gsm_fetch_messages(GSM * gsm, unsigned int start, unsigned int end)
{
	return gsm_modem_get_messages(gsm->modem, start, end);
}


/* gsm_fetch_operator */
int gsm_fetch_operator(GSM * gsm)
{
	return gsm_modem_get_operator(gsm->modem);
}


/* gsm_fetch_registration */
int gsm_fetch_registration(GSM * gsm)
{
	return gsm_modem_get_registration(gsm->modem);
}


/* gsm_fetch_signal_level */
int gsm_fetch_signal_level(GSM * gsm)
{
	return gsm_modem_get_signal_level(gsm->modem);
}


/* queries */
/* gsm_is_functional */
int gsm_is_functional(GSM * gsm)
{
	return gsm_modem_is_functional(gsm->modem);
}


/* gsm_is_phone_active */
int gsm_is_phone_active(GSM * gsm)
{
	return gsm_modem_is_phone_active(gsm->modem);
}


/* gsm_is_pin_needed */
int gsm_is_pin_needed(GSM * gsm)
{
	return gsm_modem_is_pin_needed(gsm->modem);
}


/* gsm_is_pin_valid */
int gsm_is_pin_valid(GSM * gsm)
{
	return gsm_modem_is_pin_valid(gsm->modem);
}


/* gsm_is_registered */
int gsm_is_registered(GSM * gsm)
{
	return gsm_modem_is_registered(gsm->modem);
}


/* queue management */
/* gsm_queue */
GSMCommand * gsm_queue(GSM * gsm, char const * command)
{
	GSMCommand * gsmc;

	if(command == NULL || command[0] == '\0')
		return NULL;
	if((gsmc = gsm_command_new(command)) == NULL)
		return NULL;
	if(gsm_queue_command(gsm, gsmc) == 0)
		return gsmc;
	gsm_command_delete(gsmc);
	return NULL;
}


/* gsm_queue_command */
int gsm_queue_command(GSM * gsm, GSMCommand * gsmc)
{
	GSMPriority priority;
	GSList * l;
	GSMCommand * p;

	if(gsmc == NULL)
		return 1;
	/* the GSM_PRIORITY_HIGHEST priority is meant to avoid races */
	if((priority = gsm_command_get_priority(gsmc)) > GSM_PRIORITY_HIGH)
		priority = GSM_PRIORITY_HIGH;
	for(l = gsm->queue; l != NULL; l = l->next)
	{
		p = l->data;
		if(gsm_command_get_priority(p) < priority)
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


/* gsm_queue_full */
int gsm_queue_full(GSM * gsm, GSMPriority priority, char const * command,
		GSMError error, GSMCommandCallback callback)
{
	return gsm_queue_full_mode(gsm, priority, command, error, callback,
			GSM_MODE_COMMAND);
}


/* gsm_queue_full_mode */
int gsm_queue_full_mode(GSM * gsm, GSMPriority priority, char const * command,
		GSMError error, GSMCommandCallback callback, GSMMode mode)
{
	GSMCommand * gsmc;

	if(command == NULL || command[0] == '\0')
		return 1;
	if((gsmc = gsm_command_new(command)) == NULL)
		return 1;
	gsm_command_set_priority(gsmc, priority);
	gsm_command_set_error(gsmc, error);
	gsm_command_set_callback(gsmc, callback);
	gsm_command_set_mode(gsmc, mode);
	if(gsm_queue_command(gsm, gsmc) == 0)
		return 0;
	gsm_command_delete(gsmc);
	return 1;
}


/* gsm_queue_with_error */
int gsm_queue_with_error(GSM * gsm, char const * command, GSMError error)
{
	GSMCommand * gsmc;

	if((gsmc = gsm_queue(gsm, command)) == NULL)
		return 1;
	gsm_command_set_error(gsmc, error);
	return 0;
}


/* gsm_reset */
int gsm_reset(GSM * gsm, unsigned int delay)
{
	_gsm_queue_flush(gsm);
	if(gsm->source != 0)
		g_source_remove(gsm->source);
	if(delay > 0)
		gsm->source = g_timeout_add(delay, _on_reset, gsm);
	else
		gsm->source = g_idle_add(_on_reset, gsm);
	return 0;
}


/* gsm_send_message */
int gsm_send_message(GSM * gsm, char const * number, char const * text)
{
	return gsm_modem_send_message(gsm->modem, number, text);
}


/* private */
/* functions */
/* events */
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


/* gsm_event_set_status */
static int _gsm_event_set_status(GSM * gsm, GSMStatus status)
{
	if(gsm->status == status)
		return 0;
	gsm->status = status;
	return gsm_event(gsm, GSM_EVENT_TYPE_STATUS, status);
}


/* gsm_parse */
static int _parse_pdu(GSM * gsm);
static int _parse_do(GSM * gsm, size_t * i);

static int _gsm_parse(GSM * gsm)
{
	int ret = 0;
	size_t i = 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__, gsm->rd_buf_cnt);
#endif
	while(i < gsm->rd_buf_cnt)
	{
		if(gsm->rd_buf[i++] != '\r' && gsm->rd_buf[i - 1] != '\n')
			continue;
		gsm->rd_buf[i - 1] = '\0';
		if(i < gsm->rd_buf_cnt && gsm->rd_buf[i] == '\n')
			i++;
		if(gsm->rd_buf[0] != '\0')
			ret |= _parse_do(gsm, &i);
		gsm->rd_buf_cnt -= i;
		memmove(gsm->rd_buf, &gsm->rd_buf[i], gsm->rd_buf_cnt);
		if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt)) != NULL)
			gsm->rd_buf = p; /* we can ignore errors... */
		else if(gsm->rd_buf_cnt == 0)
			gsm->rd_buf = NULL; /* ...except when it's not one */
		i = 0;
	}
	if(gsm->mode == GSM_MODE_PDU)
		return _parse_pdu(gsm);
	return ret;
}

static int _parse_pdu(GSM * gsm)
{
	size_t i = 0;
	char * p;

	if(gsm->rd_buf_cnt < 2)
		return 0;
	if(gsm->rd_buf[i++] == '>' && gsm->rd_buf[i++] == ' ')
	{
		_gsm_queue_pop(gsm);
		_gsm_queue_push(gsm);
	}
	gsm->rd_buf_cnt -= i;
	memmove(gsm->rd_buf, &gsm->rd_buf[i], gsm->rd_buf_cnt);
	if((p = realloc(gsm->rd_buf, gsm->rd_buf_cnt)) != NULL)
		gsm->rd_buf = p; /* we can ignore errors... */
	else if(gsm->rd_buf_cnt == 0)
		gsm->rd_buf = NULL; /* ...except when it's not one */
	return 0;
}

static int _parse_do(GSM * gsm, size_t * i)
{
	gboolean answered = FALSE;

	if(gsm->mode == GSM_MODE_INIT)
	{
		if(strcmp(gsm->rd_buf, "OK") != 0)
			return 0;
		if(gsm->source != 0)
			g_source_remove(gsm->source);
		gsm->source = 0;
		*i = gsm->rd_buf_cnt; /* XXX ugly: flush read buffer */
		g_io_channel_flush(gsm->channel, NULL); /* XXX check errors? */
		gsm->mode = GSM_MODE_COMMAND;
		gsm_modem_set_echo(gsm->modem, FALSE);
		gsm_modem_set_verbose(gsm->modem, TRUE);
		/* XXX should probably not be set by us */
		gsm_modem_set_extended_errors(gsm->modem, TRUE);
		gsm_modem_set_extended_ring_reports(gsm->modem, TRUE);
		gsm_modem_set_call_presentation(gsm->modem, TRUE);
		gsm_modem_get_model(gsm->modem);
		_gsm_event_set_status(gsm, GSM_STATUS_INITIALIZED);
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
	GSMCommand * gsmc;
	GSMCommandCallback callback;
	GSMError error = GSM_ERROR_UNKNOWN;
	char const * cmd;
	size_t j;
	int c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(answered != NULL)
		*answered = FALSE;
	if(strncmp(line, "AT", 2) == 0) /* ignore echo (tighter check?) */
		return 0;
	if(strcmp(line, "RING") == 0)
	{
		gsm_event(gsm, GSM_EVENT_TYPE_INCOMING_CALL,
				GSM_CALL_TYPE_UNKNOWN);
		return 0;
	}
	gsmc = g_slist_nth_data(gsm->queue, 0);
	if(strcmp(line, "OK") == 0)
	{
		/* XXX the trigger may not have been called (if any) */
		if(answered != NULL)
			*answered = TRUE;
		/* XXX call it only if we were really answered? */
		if(gsmc != NULL && (callback = gsm_command_get_callback(gsmc))
				!= NULL)
			callback(gsm);
		return 0;
	}
	for(i = 0; _gsm_triggers[i].trigger != NULL; i++)
		if(strncmp(line, _gsm_triggers[i].trigger,
					_gsm_triggers[i].trigger_cnt) == 0)
			return _gsm_triggers[i].callback(gsm,
					&line[_gsm_triggers[i].trigger_cnt],
					answered);
	if(strcmp(line, "ERROR") == 0)
	{
		if(answered != NULL)
			*answered = TRUE;
		if(gsmc != NULL)
			error = gsm_command_get_error(gsmc);
		gsm_event(gsm, GSM_EVENT_TYPE_ERROR, error, _("Unknown error"));
		return 0;
	}
	/* XXX look for a potential trigger */
	if(gsmc != NULL && (cmd = gsm_command_get_command(gsmc)) != NULL
			&& strncmp(cmd, "AT+", 3) == 0 && isupper((c = cmd[3])))
	{
		for(cmd += 2, j = 2; cmd[j] != '\0' && isupper((c = cmd[j]));
				j++);
		for(i = 0; _gsm_triggers[i].trigger != NULL; i++)
			if(strncmp(cmd, _gsm_triggers[i].trigger, j) == 0)
				return _gsm_triggers[i].callback(gsm, line,
						answered);
	}
	return 1;
}


/* queue management */
/* _gsm_queue_flush */
static void _gsm_queue_flush(GSM * gsm)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	g_slist_foreach(gsm->queue, (GFunc)gsm_command_delete, NULL);
	g_slist_free(gsm->queue);
	gsm->queue = NULL;
	free(gsm->rd_buf);
	gsm->rd_buf = NULL;
	gsm->rd_buf_cnt = 0;
	if(gsm->rd_source != 0)
	{
		g_source_remove(gsm->rd_source);
		gsm->rd_source = 0;
	}
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
	gsm_command_delete(gsmc);
	gsm->queue = g_slist_remove(gsm->queue, gsmc);
#if 0 /* FIXME this is useless */
	if(gsm->mode != GSM_MODE_COMMAND)
		return;
#endif
}


/* gsm_queue_push */
static int _gsm_queue_push(GSM * gsm)
{
	GSMCommand * gsmc;
	char const * command;
	char const suffix[] = "\r\n";

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(gsm->queue == NULL)
		return 0;
	gsmc = gsm->queue->data;
	command = gsm_command_get_command(gsmc);
	gsm->wr_buf_cnt = strlen(command) + sizeof(suffix);
	if((gsm->wr_buf = malloc(gsm->wr_buf_cnt)) == NULL)
		return 1;
	snprintf(gsm->wr_buf, gsm->wr_buf_cnt--, "%s%s", command, suffix);
	/* FIXME shouldn't it always be the case? flush input queue first? */
	if(gsm->channel != NULL && gsm->wr_source == 0)
	{
		gsm->wr_source = g_io_add_watch(gsm->channel, G_IO_OUT,
				_on_watch_can_write, gsm);
		if(gsm->source != 0 && gsm->mode != GSM_MODE_INIT)
		{
			/* XXX still not sure it is always _on_timeout */
			g_source_remove(gsm->source);
			gsm->source = 0;
		}
	}
	return 0;
}


/* triggers */
/* gsm_trigger_busy */
static int _gsm_trigger_busy(GSM * gsm, char const * result,
		gboolean * answered)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(answered != NULL)
		*answered = TRUE;
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_BUSY, "BUSY");
}


/* gsm_trigger_cfun */
static int _gsm_trigger_cfun(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u", &gsm->event.functional.functional) != 1)
		/* XXX nicer message */
		return gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_FUNCTIONAL_FAILED, result);
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_FUNCTIONAL);
}


/* gsm_trigger_cgmm */
static int _gsm_trigger_cgmm(GSM * gsm, char const * result)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	for(i = 0; _gsm_models[i].model != NULL; i++)
		if(strcmp(_gsm_models[i].model, result) == 0)
			break;
	if(_gsm_models[i].model == NULL)
		return 1; /* we do not know this model */
	gsm_modem_set_quirks(gsm->modem, _gsm_models[i].quirks);
	return 0;
}


/* gsm_trigger_clip */
static int _gsm_trigger_clip(GSM * gsm, char const * result)
{
	char number[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "\"%31[^\"]\", %u", number,
				&gsm->event.call_presentation.format) != 2)
		return 1; /* XXX report error? */
	number[sizeof(number) - 1] = '\0';
	gsm->event.call_presentation.number = number;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_CALL_PRESENTATION);
}


/* gsm_trigger_cme_error */
static int _gsm_trigger_cme_error(GSM * gsm, char const * result,
		gboolean * answered)
{
	int code;
	char * p;
	size_t i;
	GSMError type = GSM_ERROR_UNKNOWN;
	char const * error;
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	error = _("Unknown error");
	if(answered != NULL)
		*answered = TRUE;
	code = strtol(result, &p, 10);
	if(result[0] == '\0' || *p != '\0')
		return 1;
	for(i = 0; _gsm_cme_errors[i].error != NULL; i++)
		if(_gsm_cme_errors[i].code == code)
			break;
	if(_gsm_cme_errors[i].error != NULL)
		error = _(_gsm_cme_errors[i].error);
	if(gsm->queue != NULL && (gsmc = gsm->queue->data) != NULL)
		type = gsm_command_get_error(gsmc);
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, type, error);
}


/* gsm_trigger_cms_error */
static int _gsm_trigger_cms_error(GSM * gsm, char const * result)
{
	int code;
	char * p;
	size_t i;
	GSMError error = GSM_ERROR_UNKNOWN;
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	code = strtol(result, &p, 10);
	if(result[0] == '\0' || *p != '\0')
		return 1;
	for(i = 0; _gsm_cms_errors[i].error != NULL; i++)
		if(_gsm_cms_errors[i].code == code)
			break;
	if(_gsm_cms_errors[i].error == NULL)
		return 1; /* XXX report an error anyway? */
	if(gsm->queue != NULL && (gsmc = gsm->queue->data) != NULL)
		error = gsm_command_get_error(gsmc);
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, error,
			_(_gsm_cms_errors[i].error));
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


/* gsm_trigger_cmgs */
static int _gsm_trigger_cmgs(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u", &gsm->event.message_sent.mr) != 1)
		return 1;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_MESSAGE_SENT);
}


/* gsm_trigger_cmti */
static int _gsm_trigger_cmti(GSM * gsm, char const * result)
{
	char memory[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "\"%31[^\"]\",%u", memory,
				&gsm->event.incoming_message.index) != 2)
		return 1;
	memory[sizeof(memory) - 1] = '\0';
	gsm->event.incoming_message.memory = memory;
	return 0;
}


/* gsm_trigger_connect */
static int _gsm_trigger_connect(GSM * gsm, char const * result,
		gboolean * answered)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(answered != NULL)
		*answered = TRUE;
	/* FIXME implement pass-through */
	/* FIXME reset is probably not enough (send "+++"?) */
	return gsm_reset(gsm, gsm->retry);
}


/* gsm_trigger_cops */
static int _gsm_trigger_cops(GSM * gsm, char const * result)
{
	char operator[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	gsm->event.operator.operator = operator;
	gsm->event.operator.lai = 0;
	if(sscanf(result, "%u,%u,\"%31[^\"]\",%u", &gsm->event.operator.mode,
				&gsm->event.operator.format, operator,
				&gsm->event.operator.lai) >= 3)
		return _gsm_event_send(gsm, GSM_EVENT_TYPE_OPERATOR);
	return 1;
}


/* gsm_trigger_cpas */
static int _gsm_trigger_cpas(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u", &gsm->event.phone_activity.activity) != 1)
		return 1;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_PHONE_ACTIVITY);
}


/* _gsm_trigger_cpbr */
static int _gsm_trigger_cpbr(GSM * gsm, char const * result)
{
	unsigned int start;
	unsigned int end;
	char number[32];
	char name[32];
	gchar * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "(%u-%u)", &start, &end) == 2)
		return gsm_event(gsm, GSM_EVENT_TYPE_CONTACT_LIST, start, end);
	if(sscanf(result, "%u,\"%31[^\"]\",145,\"%31[^\"]\"",
				&gsm->event.contact.index, number, name) != 3
			&& sscanf(result, "%u,\"%31[^\"]\",129,\"%31[^\"]\"",
				&gsm->event.contact.index, number, name) != 3)
		return 1;
	number[sizeof(number) - 1] = '\0';
	gsm->event.contact.number = number;
	name[sizeof(name) - 1] = '\0';
	gsm->event.contact.name = name;
	if((p = g_convert(name, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL))
			!= NULL)
	{
		snprintf(name, sizeof(name), "%s", p);
		g_free(p);
	}
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_CONTACT);
}


/* _gsm_trigger_cpin */
static int _gsm_trigger_cpin(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(strcmp(result, "READY") == 0)
		return _gsm_event_set_status(gsm, GSM_STATUS_READY);
	if(strcmp(result, "SIM PIN") == 0)
		return gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_SIM_PIN_REQUIRED, NULL);
	/* XXX nicer message */
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_UNKNOWN, result);
}


/* gsm_trigger_creg */
static int _gsm_trigger_creg(GSM * gsm, char const * result)
{
	int ret;
	int res;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if((res = sscanf(result, "%u,%u,%X,%X", &gsm->event.registration.n,
					&gsm->event.registration.stat,
					&gsm->event.registration.area,
					&gsm->event.registration.cell)) == 4)
		ret = _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	else if(res == 2)
	{
		gsm->event.registration.area = 0;
		gsm->event.registration.cell = 0;
		ret = _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	}
	else
		return 1;
	switch(gsm->event.registration.stat)
	{
		case GSM_REGISTRATION_STATUS_NOT_SEARCHING:
			ret |= _gsm_event_set_status(gsm, GSM_STATUS_READY);
			break;
		case GSM_REGISTRATION_STATUS_NOT_REGISTERED:
			ret |= _gsm_event_set_status(gsm,
					GSM_STATUS_REGISTERING);
			break;
		case GSM_REGISTRATION_STATUS_DENIED:
			ret |= _gsm_event_set_status(gsm,
					GSM_STATUS_REGISTERING_DENIED);
			break;
		case GSM_REGISTRATION_STATUS_REGISTERED_HOME:
			ret |= _gsm_event_set_status(gsm,
					GSM_STATUS_REGISTERED_HOME);
			break;
		case GSM_REGISTRATION_STATUS_REGISTERED_ROAMING:
			ret |= _gsm_event_set_status(gsm,
					GSM_STATUS_REGISTERED_ROAMING);
			break;
		case GSM_REGISTRATION_STATUS_UNKNOWN:
		default:
			ret |= _gsm_event_set_status(gsm, GSM_STATUS_UNKNOWN);
			break;
	}
	return ret;
}


/* gsm_trigger_cring */
static int _gsm_trigger_cring(GSM * gsm, char const * result)
{
	GSMCallType calltype = GSM_CALL_TYPE_UNKNOWN;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	/* XXX implement the other call types */
	if(strcmp(result, "VOICE") == 0)
		calltype = GSM_CALL_TYPE_VOICE;
	return gsm_event(gsm, GSM_EVENT_TYPE_INCOMING_CALL, calltype);
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


/* gsm_trigger_no_answer */
static int _gsm_trigger_no_answer(GSM * gsm, char const * result,
		gboolean * answered)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(answered != NULL)
		*answered = TRUE;
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_NO_ANSWER,
			"NO ANSWER");
}


/* gsm_trigger_no_carrier */
static int _gsm_trigger_no_carrier(GSM * gsm, char const * result,
		gboolean * answered)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(answered != NULL)
		*answered = TRUE;
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_NO_CARRIER,
			"NO CARRIER");
}


/* gsm_trigger_no_dialtone */
static int _gsm_trigger_no_dialtone(GSM * gsm, char const * result,
		gboolean * answered)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(answered != NULL)
		*answered = TRUE;
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, GSM_ERROR_NO_DIALTONE,
			"NO DIALTONE");
}


/* callbacks */
/* on_reset */
static int _reset_do(GSM * gsm, int fd);
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
	if(gsm->source != 0)
		g_source_remove(gsm->source);
	if(gsm->channel != NULL)
	{
		/* XXX should the file descriptor also be freed? */
		g_io_channel_shutdown(gsm->channel, TRUE, &error);
		g_io_channel_unref(gsm->channel);
		gsm->channel = NULL;
	}
	if((fd = open(gsm->device, O_RDWR | O_NONBLOCK)) < 0
			|| _reset_do(gsm, fd) != 0)
	{
		snprintf(buf, sizeof(buf), "%s%s%s", gsm->device, ": ",
				strerror(errno));
		if(fd >= 0)
			close(fd);
		if(gsm->retry > 0)
			gsm->source = g_timeout_add(gsm->retry, _on_reset, gsm);
		gsm->source = 0;
		return phone_error(NULL, buf, FALSE);
	}
	gsm->channel = g_io_channel_unix_new(fd);
	if((g_io_channel_set_encoding(gsm->channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
		phone_error(NULL, error->message, 0);
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

static int _reset_do(GSM * gsm, int fd)
{
	struct stat st;
	int fl;
	struct termios term;

	if(flock(fd, LOCK_EX | LOCK_NB) != 0)
		return 1;
	fl = fcntl(fd, F_GETFL, 0);
	if(fcntl(fd, F_SETFL, fl & ~O_NONBLOCK) == -1)
		return 1;
	if(fstat(fd, &st) != 0)
		return 1;
	if(st.st_mode & S_IFCHR) /* character special */
	{
		if(tcgetattr(fd, &term) != 0)
			return 1;
		term.c_cflag &= ~(CSIZE | PARENB);
		term.c_cflag |= CS8;
		term.c_cflag |= CREAD;
		term.c_cflag |= CLOCAL;
		if(gsm->hwflow)
			term.c_cflag |= CRTSCTS;
		else
			term.c_cflag &= ~CRTSCTS;
		term.c_iflag = (IGNPAR | IGNBRK);
		term.c_lflag = 0;
		term.c_oflag = 0;
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		if(cfsetispeed(&term, 0) != 0) /* same speed as output speed */
			phone_error(NULL, gsm->device, 0); /* go on anyway */
		if(cfsetospeed(&term, gsm->baudrate) != 0)
			phone_error(NULL, gsm->device, 0); /* go on anyway */
		if(tcsetattr(fd, TCSAFLUSH, &term) != 0)
			return 1;
	}
	return 0;
}

static gboolean _reset_settle(gpointer data)
{
	GSM * gsm = data;

	gsm_modem_reset(gsm->modem);
	return TRUE;
}


/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	GSM * gsm = data;
	GSMCommand * gsmc;
	char const * cmd;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gsm->source = 0;
	/* FIXME this code partly duplicates _gsm_queue_push() */
	/* check if the write handler is still running */
	if(gsm->channel == NULL || gsm->wr_source != 0)
		return FALSE;
	if(gsm->queue == NULL || (gsmc = gsm->queue->data) == NULL)
		return FALSE;
	if((cmd = gsm_command_get_command(gsmc)) == NULL)
		return FALSE;
	len = strlen(cmd) + 2;
	/* re-inject the command */
	if((p = realloc(gsm->wr_buf, len + 1)) == NULL)
		return FALSE;
	gsm->wr_buf = p;
	snprintf(p, len + 1, "%s%s", cmd, "\r\n");
	gsm->wr_buf_cnt = len;
	gsm->wr_source = g_io_add_watch(gsm->channel, G_IO_OUT,
			_on_watch_can_write, gsm);
	return FALSE;
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
			phone_error(NULL, error->message, 0);
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
	GSMCommand * gsmc;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__, gsm->wr_buf_cnt);
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
			gsm->wr_buf = p; /* we can ignore errors... */
		else if(gsm->wr_buf_cnt == 0)
			gsm->wr_buf = NULL; /* ...except when it's not one */
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			phone_error(NULL, error->message, 0);
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
	else
	{
		if(gsm->queue != NULL && (gsmc = gsm->queue->data) != NULL)
			gsm->mode = gsm_command_get_mode(gsmc);
		if(gsm->source != 0)
			g_source_remove(gsm->source);
		gsm->source = g_timeout_add(2000, _on_timeout, gsm);
	}
	return FALSE;
}
