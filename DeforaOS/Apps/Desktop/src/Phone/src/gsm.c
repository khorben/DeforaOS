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
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <System.h>
#include "command.h"
#include "modem.h"
#include "gsm.h"
#define _(string) gettext(string)
#define N_(string) (string)

#define min(a, b) ((a) < (b) ? (a) : (b))


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

	/* temporary buffers */
	char number[32];
};


/* variables */
/* conversions */
static struct
{
	unsigned char gsm;
	unsigned char iso;
} _gsm_conv[] =
{
	{ '\0',	'@'	},
	{ 0x01,	163	}, /* £ */
	{ 0x02,	'$'	},
	{ 0x03,	165	}, /* ¥ */
	{ 0x04,	232	}, /* è */
	{ 0x05,	233	}, /* é */
	{ 0x06,	249	}, /* ù */
	{ 0x07,	236	}, /* ì */
	{ 0x08,	242	}, /* ò */
	{ 0x09,	199	}, /* Ç */
	{ 0x0b,	216	}, /* Ø */
	{ 0x0c,	248	}, /* ø */
	{ 0x0e,	197	}, /* Å */
	{ 0x0f,	229	}, /* å */
	{ 0x10,	' '	}, /* XXX delta */
	{ 0x11,	'_'	},
	{ 0x12,	' '	}, /* XXX phi */
	{ 0x13,	' '	}, /* XXX gamma */
	{ 0x14,	' '	}, /* XXX lambda */
	{ 0x15,	' '	}, /* XXX omega */
	{ 0x16,	' '	}, /* XXX pi */
	{ 0x17,	' '	}, /* XXX psi */
	{ 0x18,	' '	}, /* XXX sigma */
	{ 0x19,	' '	}, /* XXX theta */
	{ 0x1a,	' '	}, /* XXX xi */
	{ 0x1b,	' '	}, /* FIXME escape */
	{ 0x1c,	198	},
	{ 0x1d,	230	},
	{ 0x1e,	223	},
	{ 0x1f,	201	},
	{ 0x24,	164	},
	{ 0x40,	161	},
	{ 0x5b,	196	},
	{ 0x5c,	214	},
	{ 0x5d,	209	},
	{ 0x5e,	220	},
	{ 0x5f,	167	},
	{ 0x60,	191	},
	{ 0x7b,	228	},
	{ 0x7c,	246	},
	{ 0x7d,	241	},
	{ 0x7e,	252	},
	{ 0x7f,	224	}
};

/* errors */
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
	{ 320,	N_("Memory failure")				},
	{ 321,	N_("Invalid memory index")			},
	{ 322,	N_("Memory full")				},
	{ 0,	NULL						}
};

/* EXT ERROR */
static struct
{
	int code;
	char const * error;
} _gsm_ext_errors[] =
{
	{ 0,	N_("Invalid index"),				},
	{ 1,	N_("Parameter not allowed"),			},
	{ 2,	N_("Data corrupted"),				},
	{ 3,	N_("Internal error"),				},
	{ 4,	N_("Call table full"),				},
	{ 5,	N_("Service table full"),			},
	{ 6,	N_("Call not found"),				},
	{ 7,	N_("No data-call supported"),			},
	{ 8,	N_("One call on hold"),				},
	{ 9,	N_("Hold call not supported for this type"),	},
	{ 10,	N_("Number not allowed by FDN"),		},
	{ 11,	N_("Number not allowed by BDN"),		},
	{ 12,	N_("Parallel USSD not supported"),		},
	{ 13,	N_("Fax minimum speed condition"),		},
	{ 14,	N_("Conflict with command details"),		},
	{ 0,	NULL						}
};


/* models */
static struct
{
	char const * model;
	unsigned long quirks;
} _gsm_models[] =
{
	{ "\"Neo1973 Embedded GSM Modem\"",
		GSM_MODEM_QUIRK_CPIN_QUOTES
			| GSM_MODEM_QUIRK_WANT_SMSC_IN_PDU	},
	{ "\"Neo1973 GTA01/GTA02 Embedded GSM Modem\"",
		GSM_MODEM_QUIRK_CPIN_QUOTES
			| GSM_MODEM_QUIRK_WANT_SMSC_IN_PDU	},
	{ "\"Neo1973 GTA02 Embedded GSM Modem\"",
		GSM_MODEM_QUIRK_CPIN_QUOTES
			| GSM_MODEM_QUIRK_WANT_SMSC_IN_PDU	},
	{ NULL,	0						}
};


/* prototypes */
/* conversions */
static unsigned char _gsm_convert_from_iso(unsigned char c);
static unsigned char _gsm_convert_to_iso(unsigned char c);

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
static int _gsm_trigger_cbc(GSM * gsm, char const * result);
static int _gsm_trigger_ccwa(GSM * gsm, char const * result);
static int _gsm_trigger_cfun(GSM * gsm, char const * result);
static int _gsm_trigger_cgmm(GSM * gsm, char const * result);
static int _gsm_trigger_clip(GSM * gsm, char const * result);
static int _gsm_trigger_cme_error(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_cms_error(GSM * gsm, char const * result);
static int _gsm_trigger_cmgl(GSM * gsm, char const * result);
static int _gsm_trigger_cmgr(GSM * gsm, char const * result);
static int _gsm_trigger_cmgs(GSM * gsm, char const * result);
static int _gsm_trigger_cmti(GSM * gsm, char const * result);
static int _gsm_trigger_cmut(GSM * gsm, char const * result);
static int _gsm_trigger_connect(GSM * gsm, char const * result,
		gboolean * answered);
static int _gsm_trigger_cops(GSM * gsm, char const * result);
static int _gsm_trigger_cpas(GSM * gsm, char const * result);
static int _gsm_trigger_cpbr(GSM * gsm, char const * result);
static int _gsm_trigger_cpin(GSM * gsm, char const * result);
static int _gsm_trigger_creg(GSM * gsm, char const * result);
static int _gsm_trigger_cring(GSM * gsm, char const * result);
static int _gsm_trigger_cssi(GSM * gsm, char const * result);
static int _gsm_trigger_cssu(GSM * gsm, char const * result);
static int _gsm_trigger_csq(GSM * gsm, char const * result);
static int _gsm_trigger_ext_error(GSM * gsm, char const * result,
		gboolean * answered);
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
	GSM_TRIGGER("+CBC: ",		cbc),
	GSM_TRIGGER("+CCWA: ",		ccwa),
	GSM_TRIGGER("+CFUN: ",		cfun),
	GSM_TRIGGER("+CGMM: ",		cgmm),
	GSM_TRIGGER("+CLIP: ",		clip),
	GSM_TRIGGER("+CME ERROR: ",	cme_error),
	GSM_TRIGGER("+CMS ERROR: ",	cms_error),
	GSM_TRIGGER("+CMGL: ",		cmgl),
	GSM_TRIGGER("+CMGR: ",		cmgr),
	GSM_TRIGGER("+CMGS: ",		cmgs),
	GSM_TRIGGER("+CMTI: ",		cmti),
	GSM_TRIGGER("+CMUT: ",		cmut),
	GSM_TRIGGER("CONNECT",		connect),
	GSM_TRIGGER("+COPS: ",		cops),
	GSM_TRIGGER("+CPAS: ",		cpas),
	GSM_TRIGGER("+CPBR: ",		cpbr),
	GSM_TRIGGER("+CPIN: ",		cpin),
	GSM_TRIGGER("+CREG: ",		creg),
	GSM_TRIGGER("+CRING: ",		cring),
	GSM_TRIGGER("+CSQ: ",		csq),
	GSM_TRIGGER("+CSSI: ",		cssi),
	GSM_TRIGGER("+CSSU: ",		cssu),
	GSM_TRIGGER("+EXT ERROR: ",	ext_error),
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

	if((gsm = malloc(sizeof(*gsm))) == NULL)
		return NULL;
	/* settings */
	if(device == NULL)
		device = "/dev/modem";
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
	return gsm;
}

static unsigned int _new_baudrate(unsigned int baudrate)
{
	char const * error;

	error = _(": Unknown baudrate, assuming 115200");
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
			error_set("%u%s", baudrate, error, 115200);
			return 115200;
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


/* gsm_set_call_presentation */
int gsm_set_call_presentation(GSM * gsm, int set)
{
	return gsm_modem_set_call_presentation(gsm->modem, (set != 0) ? TRUE
			: FALSE);
}


/* gsm_set_call_waiting_control */
int gsm_set_call_waiting_control(GSM * gsm, int unsollicited)
{
	return gsm_modem_set_call_waiting_control(gsm->modem,
			(unsollicited != 0) ? TRUE : FALSE);
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


/* gsm_set_line_presentation */
int gsm_set_line_presentation(GSM * gsm, int set)
{
	return gsm_modem_set_line_presentation(gsm->modem, (set != 0) ? TRUE
			: FALSE);
}


/* gsm_set_message_indications */
int gsm_set_message_indications(GSM * gsm, GSMMessageMode mode,
		int unsollicited)
{
	return gsm_modem_set_message_indications(gsm->modem, mode,
			(unsollicited != 0) ? TRUE : FALSE);
}


/* gsm_set_mute */
int gsm_set_mute(GSM * gsm, int mute)
{
	return gsm_modem_set_mute(gsm->modem, (mute != 0) ? TRUE : FALSE);
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


/* gsm_set_supplementary_service_notifications */
int gsm_set_supplementary_service_notifications(GSM * gsm, int intermediate,
		int unsollicited)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s, %s)\n", __func__, (intermediate != 0)
			? "TRUE" : "FALSE", (unsollicited != 0) ? "TRUE"
			: "FALSE");
#endif
	return gsm_modem_set_supplementary_service_notifications(gsm->modem,
			(intermediate != 0) ? TRUE : FALSE,
			(unsollicited != 0) ? TRUE : FALSE);
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


/* gsm_call_reject */
int gsm_call_reject(GSM * gsm)
{
	return gsm_modem_call_reject(gsm->modem);
}


/* callbacks */
/* gsm_callback_on_message_deleted */
void gsm_callback_on_message_deleted(GSM * gsm)
{
	GSMCommand * gsmc;
	unsigned long index;

	if((gsmc = g_slist_nth_data(gsm->queue, 0)) == NULL)
		return;
	index = (unsigned long)gsm_command_get_data(gsmc);
	gsm_event(gsm, GSM_EVENT_TYPE_MESSAGE_DELETED, index);
}


/* gsm_contact_delete */
int gsm_contact_delete(GSM * gsm, unsigned int index)
{
	return gsm_modem_contact_delete(gsm->modem, index);
}


/* gsm_contact_edit */
int gsm_contact_edit(GSM * gsm, unsigned int index, char const * name,
		char const * number)
{
	return gsm_modem_contact_edit(gsm->modem, index, name, number);
}


/* gsm_contact_new */
int gsm_contact_new(GSM * gsm, char const * name, char const * number)
{
	return gsm_modem_contact_new(gsm->modem, name, number);
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
		case GSM_EVENT_TYPE_BATTERY_CHARGE:
			event->battery_charge.status = va_arg(ap,
					GSMBatteryStatus);
			event->battery_charge.level = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_CALL_PRESENTATION:
			event->call_presentation.number = va_arg(ap,
					char const *);
			event->call_presentation.format = va_arg(ap,
					unsigned int);
			break;
		case GSM_EVENT_TYPE_CALL_WAITING:
			event->call_waiting_control.unsollicited = va_arg(ap,
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
		case GSM_EVENT_TYPE_MESSAGE:
			event->message.index = va_arg(ap, unsigned int);
			event->message.number = va_arg(ap, char const *);
			event->message.date = va_arg(ap, time_t);
			event->message.encoding = va_arg(ap, GSMEncoding);
			event->message.length = va_arg(ap, unsigned int);
			event->message.content = va_arg(ap, char const *);
			break;
		case GSM_EVENT_TYPE_MESSAGE_DELETED:
			event->message.index = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_MESSAGE_LIST:
			event->message_list.start = va_arg(ap, unsigned int);
			event->message_list.end = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_MESSAGE_SENT:
			event->message_sent.mr = va_arg(ap, unsigned int);
			break;
		case GSM_EVENT_TYPE_MUTE:
			event->mute.mute = va_arg(ap, unsigned int);
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
		case GSM_EVENT_TYPE_UNKNOWN:
			event->unknown.command = va_arg(ap, char const *);
			event->unknown.result = va_arg(ap, char const *);
			break;
	}
	va_end(ap);
	return _gsm_event_send(gsm, type);
}


/* gsm_fetch_battery_charge */
int gsm_fetch_battery_charge(GSM * gsm)
{
	return gsm_modem_get_battery_charge(gsm->modem);
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
int gsm_fetch_message_list(GSM * gsm, GSMMessageList list)
{
	return gsm_modem_get_message_list(gsm->modem, list);
}


/* gsm_fetch_message */
int gsm_fetch_message(GSM * gsm, unsigned int index)
{
	return gsm_modem_get_message(gsm->modem, index);
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
/* gsm_is_alive */
int gsm_is_alive(GSM * gsm)
{
	return gsm_modem_is_alive(gsm->modem);
}


/* gsm_is_call_waiting_control */
int gsm_is_call_waiting_control(GSM * gsm)
{
	return gsm_modem_is_call_waiting_control(gsm->modem);
}


/* gsm_is_functional */
int gsm_is_functional(GSM * gsm)
{
	return gsm_modem_is_functional(gsm->modem);
}


/* gsm_is_mute */
int gsm_is_mute(GSM * gsm)
{
	return gsm_modem_is_mute(gsm->modem);
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


/* messaging */
/* gsm_message_delete */
int gsm_message_delete(GSM * gsm, unsigned int index)
{
	return gsm_modem_message_delete(gsm->modem, index);
}


/* gsm_message_send */
static int _message_send_utf8(GSM * gsm, char const * number, char const * text,
		size_t length);

int gsm_message_send(GSM * gsm, char const * number, GSMEncoding encoding,
		char const * text, size_t length)
{
	switch(encoding)
	{
		case GSM_ENCODING_UTF8:
			return _message_send_utf8(gsm, number, text, length);
		case GSM_ENCODING_RAW_DATA:
			return gsm_modem_message_send(gsm->modem, number,
					GSM_MODEM_ALPHABET_DATA, text, length);
	}
	return 1; /* should not be reached */
}

static int _message_send_utf8(GSM * gsm, char const * number, char const * text,
		size_t length)
{
	int ret;
	gchar * p;
	size_t i;

	if((p = g_convert(text, length, "ISO-8859-1", "UTF-8", NULL, &length,
					NULL)) == NULL)
		return 1; /* XXX report error */
	for(i = 0; i < length; i++)
		p[i] = _gsm_convert_from_iso(text[i]);
	ret = gsm_modem_message_send(gsm->modem, number,
			GSM_MODEM_ALPHABET_DEFAULT, text, length);
	g_free(p);
	return ret;
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


/* private */
/* functions */
/* conversions */
/* gsm_convert_from_iso */
static unsigned char _gsm_convert_from_iso(unsigned char c)
{
	size_t i;

	for(i = 0; i < sizeof(_gsm_conv) / sizeof(*_gsm_conv); i++)
		if(_gsm_conv[i].iso == c)
			return _gsm_conv[i].gsm;
	return c & 0x7f;
}


/* gsm_convert_to_iso */
static unsigned char _gsm_convert_to_iso(unsigned char c)
{
	size_t i;

	for(i = 0; i < sizeof(_gsm_conv) / sizeof(*_gsm_conv); i++)
		if(_gsm_conv[i].gsm == c)
			return _gsm_conv[i].iso;
	return c;
}


/* events */
/* gsm_event_send */
static int _gsm_event_send(GSM * gsm, GSMEventType type)
{
	int ret;

	gsm->event.type = type;
	ret = gsm->callback(&gsm->event, gsm->callback_data);
	if(ret == 0)
		return 0;
	return error_set_code(ret, "%u: %s", type, _("Event not handled"));
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
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__,
			(unsigned long)gsm->rd_buf_cnt);
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
		gsm_modem_set_extended_errors(gsm->modem, TRUE);
		gsm_modem_get_model(gsm->modem);
		_gsm_event_set_status(gsm, GSM_STATUS_INITIALIZED);
		_gsm_queue_push(gsm);
	}
	else if(gsm->mode == GSM_MODE_COMMAND
			/* XXX not sure about PDU mode here */
			|| gsm->mode == GSM_MODE_PDU)
	{
		_gsm_parse_line(gsm, gsm->rd_buf, &answered);
		if(answered)
		{
			if(gsm->source != 0)
			{
				g_source_remove(gsm->source);
				gsm->source = 0;
			}
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
	char const * cmd = NULL;
	size_t j;
	int c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, line);
#endif
	if(answered != NULL)
		*answered = FALSE;
	if((gsmc = g_slist_nth_data(gsm->queue, 0)) != NULL
			&& (cmd = gsm_command_get_command(gsmc)) != NULL
			&& strcmp(line, cmd) == 0)
		return 0; /* ignore echo */
	if(strcmp(line, "RING") == 0)
	{
		gsm_event(gsm, GSM_EVENT_TYPE_INCOMING_CALL,
				GSM_CALL_TYPE_UNKNOWN);
		return 0;
	}
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
	if(cmd != NULL && strncmp(cmd, "AT+", 3) == 0 && isupper((c = cmd[3])))
	{
		for(j = 2; cmd[j + 2] != '\0' && isupper((c = cmd[j + 2]));
				j++);
		for(i = 0; _gsm_triggers[i].trigger != NULL; i++)
			if(strncmp(cmd + 2, _gsm_triggers[i].trigger, j) == 0)
				return _gsm_triggers[i].callback(gsm, line,
						answered);
	}
	return gsm_event(gsm, GSM_EVENT_TYPE_UNKNOWN, cmd, line);
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


/* gsm_trigger_cbc */
static int _gsm_trigger_cbc(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u,%u", &gsm->event.battery_charge.status,
				&gsm->event.battery_charge.level) != 2)
		return gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_BATTERY_CHARGE_FAILED, result);
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_BATTERY_CHARGE);
}


/* gsm_trigger_ccwa */
static int _gsm_trigger_ccwa(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u", &gsm->event.call_waiting_control.unsollicited)
			== 1)
		return _gsm_event_send(gsm, GSM_EVENT_TYPE_CALL_WAITING);
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
			GSM_ERROR_CALL_WAITING_FAILED, result);
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
	if(sscanf(result, "\"%31[^\"]\",%u", number,
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
	unsigned int * start = &gsm->event.message_list.start;
	unsigned int u;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	/* FIXME this command may be long and the timeout triggered too soon */
	/* XXX we could already be reading the message at this point */
	if(sscanf(result, "%u,%u,%u,%u", start, &u, &u, &u) != 4
			&& sscanf(result, "%u,%u,,%u", start, &u, &u) != 3)
		/* XXX we may be stuck in PDU mode at this point */
		return gsm_event(gsm, GSM_EVENT_TYPE_ERROR,
				GSM_ERROR_MESSAGE_FETCH_FAILED,
				_("Unknown error"));
	gsm->event.message_list.end = *start;
	gsm->mode = GSM_MODE_PDU;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_MESSAGE_LIST);
}


/* gsm_trigger_cmgr */
static char * _cmgr_pdu_parse(char const * pdu, time_t * timestamp,
		char number[32], GSMEncoding * encoding, size_t * length);
static void _cmgr_pdu_parse_number(unsigned int type, char const * number,
		size_t length, char buf[32]);
static time_t _cmgr_pdu_parse_timestamp(char const * timestamp);
static char * _cmgr_pdu_parse_encoding_default(char const * pdu, size_t len,
		size_t i, size_t hdr, GSMEncoding * encoding, size_t * length);
static char * _cmgr_pdu_parse_encoding_data(char const * pdu, size_t len,
		size_t i, size_t hdr, GSMEncoding * encoding, size_t * length);

static int _gsm_trigger_cmgr(GSM * gsm, char const * result)
{
	char buf[32];
	char date[32];
	unsigned int mbox;
	unsigned int alpha = 0;
	unsigned int * length = &gsm->event.message.length;
	struct tm t;
	char * p;
	GSMCommand * gsmc;
	size_t l = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	/* FIXME report which mailbox contains the message? */
	/* text mode support */
	if(sscanf(result, "\"%31[^\"]\",\"%31[^\"]\",,\"%31[^\"]\"", buf,
				gsm->number, date) == 3) /* XXX improve */
	{
		gsm->number[sizeof(gsm->number) - 1] = '\0';
		gsm->event.message.number = gsm->number;
		date[sizeof(date) - 1] = '\0';
		if(strptime(date, "%y/%m/%d,%H:%M:%S", &t) == NULL)
			/* XXX also parse the timezone? */
			localtime_r(NULL, &t);
		gsm->event.message.date = mktime(&t);
		*length = 0;
		return 0; /* we need to wait for the next line */
	}
	/* PDU mode support */
	if(sscanf(result, "%u,%u,%u", &mbox, &alpha, length) == 3
			|| sscanf(result, "%u,,%u", &mbox, length) == 2)
		return 0; /* we need to wait for the next line */
	/* message content */
	if(*length == 0) /* XXX assumes this is text mode */
	{
		gsm->event.message.encoding = GSM_ENCODING_UTF8;
		gsm->event.message.content = result;
		*length = strlen(result) + 1;
		_gsm_event_send(gsm, GSM_EVENT_TYPE_MESSAGE);
	}
	else if((p = _cmgr_pdu_parse(result, &gsm->event.message.date,
					gsm->number,
					&gsm->event.message.encoding, &l))
			!= NULL)
	{
		gsm->event.message.index = 0;
		if((gsmc = g_slist_nth_data(gsm->queue, 0)) != NULL)
			gsm->event.message.index /* XXX ugly */
				= (unsigned long)gsm_command_get_data(gsmc);
		gsm->event.message.number = gsm->number; /* XXX ugly */
		gsm->event.message.length = l;
		gsm->event.message.content = p;
		_gsm_event_send(gsm, GSM_EVENT_TYPE_MESSAGE);
		free(p);
	}
	return 0;
}

/* XXX this function is fat and ugly */
static char * _cmgr_pdu_parse(char const * pdu, time_t * timestamp,
		char number[32], GSMEncoding * encoding, size_t * length)
{
	size_t len;
	unsigned int smscl;
	unsigned int tp;
	unsigned int hdr;
	unsigned int addrl;
	unsigned int pid;
	unsigned int dcs;
	unsigned int datal;
	unsigned int u;
	char const * q;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, pdu);
#endif
	len = strlen(pdu);
	if(sscanf(pdu, "%02X", &smscl) != 1) /* SMSC length */
		return NULL;
	if((smscl * 2) + 2 > len)
		return NULL;
	q = pdu + (smscl * 2) + 2;
	if(sscanf(q, "%02X", &tp) != 1)
		return NULL;
	if((tp & 0x03) != 0x00) /* TP-MTI not SMS-DELIVER */
		return NULL;
	hdr = ((tp & 0x40) == 0x40) ? 1 : 0; /* TP-UDHI header present */
	if((smscl * 2) + 4 > len)
		return NULL;
	q = pdu + (smscl * 2) + 4;
	if(sscanf(q, "%02X", &addrl) != 1) /* address length */
		return NULL;
	if((smscl * 2) + 6 > len)
		return NULL;
	q = pdu + (smscl * 2) + 6;
	if(sscanf(q, "%02X", &u) != 1) /* type of address */
		return NULL;
	/* FIXME this probably depends on the type of address */
	if(addrl % 2 == 1)
		addrl++;
	if((smscl * 2) + 2 + 4 + addrl + 2 > len)
		return NULL;
	_cmgr_pdu_parse_number(u, q + 2, addrl, number);
	q = pdu + (smscl * 2) + 2 + 4 + addrl + 2;
	if(sscanf(q, "%02X", &pid) != 1) /* PID */
		return NULL;
	if((smscl * 2) + 2 + 4 + addrl + 4 > len)
		return NULL;
	q = pdu + (smscl * 2) + 2 + 4 + addrl + 4;
	if(sscanf(q, "%02X", &dcs) != 1) /* DCS */
		return NULL;
	if((smscl * 2) + 2 + 4 + addrl + 6 > len)
		return NULL;
	q = pdu + (smscl * 2) + 2 + 4 + addrl + 6;
	if(timestamp != NULL)
		*timestamp = _cmgr_pdu_parse_timestamp(q);
	if((smscl * 2) + 2 + 4 + addrl + 6 + 14 > len)
		return NULL;
	q = pdu + (smscl * 2) + 2 + 4 + addrl + 6 + 14;
	if(sscanf(q, "%02X", &datal) != 1) /* data length */
		return NULL;
	/* XXX check the data length */
	if((i = (smscl * 2) + 2 + 4 + addrl + 6 + 16) > len)
		return NULL;
	if(hdr != 0 && sscanf(&pdu[i], "%02X", &hdr) != 1)
		return NULL;
	if(dcs == 0x00)
		return _cmgr_pdu_parse_encoding_default(pdu, len, i, hdr,
				encoding, length);
	if(dcs == 0x04)
		return _cmgr_pdu_parse_encoding_data(pdu, len, i, hdr,
				encoding, length);
	return NULL;
}

static char * _cmgr_pdu_parse_encoding_default(char const * pdu, size_t len,
		size_t i, size_t hdr, GSMEncoding * encoding, size_t * length)
{
	unsigned char * p;
	size_t j;
	unsigned char rest;
	int shift = 0;
	char const * q;
	unsigned int u;
	unsigned char byte;
	char * r;

	if((p = malloc(len - i + 1)) == NULL)
		return NULL;
	if(hdr != 0)
	{
		/* FIXME actually parse the header */
		u = 2 + (hdr * 2);
		if(u % 7 != 0) /* fill bits */
			u += 7 - (u % 7);
		i += u;
	}
	p[0] = '\0';
	for(j = 0, rest = 0; i + 1 < len; i+=2)
	{
		q = &pdu[i];
		if(sscanf(q, "%02X", &u) != 1)
			break; /* FIXME report an error instead? */
		byte = u;
		p[j] = (byte << (shift + 1) >> (shift + 1) << shift) & 0x7f;
		p[j] |= rest;
		p[j] = _gsm_convert_to_iso(p[j]);
		j++;
		rest = (byte >> (7 - shift)) & 0x7f;
		if(++shift == 7)
		{
			shift = 0;
			p[j++] = rest;
			rest = 0;
		}
	}
	*encoding = GSM_ENCODING_UTF8;
	if((r = g_convert((char *)p, j, "UTF-8", "ISO-8859-1", NULL, NULL,
					NULL)) != NULL)
	{
		free(p);
		p = (unsigned char *)r;
		j = strlen(r);
	}
	*length = j;
	return (char *)p;
}

static char * _cmgr_pdu_parse_encoding_data(char const * pdu, size_t len,
		size_t i, size_t hdr, GSMEncoding * encoding, size_t * length)
{
	unsigned char * p;
	size_t j;
	unsigned int u;

	if((p = malloc(len - i + 1)) == NULL) /* XXX 2 times big enough? */
		return NULL;
	/* FIXME actually parse the header */
	if(hdr != 0)
		i += 2 + (hdr * 2);
	for(j = 0; i + 1 < len; i+=2)
	{
		if(sscanf(&pdu[i], "%02X", &u) != 1)
		{
			free(p);
			return NULL;
		}
		p[j++] = u;
	}
	*encoding = GSM_ENCODING_RAW_DATA;
	*length = j;
	p[j] = '\0';
	return (char *)p;
}

static void _cmgr_pdu_parse_number(unsigned int type, char const * number,
		size_t length, char buf[32])
{
	char * b = buf;
	size_t i;

	if(type == 0x91)
		*(b++) = '+';
	for(i = 0; i < length - 1 && i < 32 - 1; i+=2)
	{
		if((number[i] != 'F' && (number[i] < '0' || number[i] > '9'))
				|| number[i + 1] < '0' || number[i + 1] > '9')
			break;
		b[i] = number[i + 1];
		if((b[i + 1] = number[i]) == 'F')
			b[i + 1] = '\0';
	}
	b[i] = '\0';
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %lu) => \"%s\"\n", __func__, number,
			(unsigned long)length, b);
#endif
}

static time_t _cmgr_pdu_parse_timestamp(char const * timestamp)
{
	char const * p = timestamp;
	size_t i;
	struct tm t;
#ifdef DEBUG
	char buf[32];
#endif

	if(strlen(p) < 14)
		return 0;
	for(i = 0; i < 14; i++)
		if(p[i] < '0' || p[i] > '9')
			return 0;
	memset(&t, 0, sizeof(t));
	t.tm_year = (p[0] - '0') + ((p[1] - '0') * 10);
	t.tm_year = (t.tm_year > 70) ? t.tm_year : (100 + t.tm_year);
	t.tm_mon = (p[2] - '0') + ((p[3] - '0') * 10);
	if(t.tm_mon > 0)
		t.tm_mon--;
	t.tm_mday = (p[4] - '0') + ((p[5] - '0') * 10);
	t.tm_hour = (p[6] - '0') + ((p[7] - '0') * 10);
	t.tm_min = (p[8] - '0') + ((p[9] - '0') * 10);
	t.tm_sec = (p[10] - '0') + ((p[11] - '0') * 10);
#ifdef DEBUG
	strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", &t);
	fprintf(stderr, "DEBUG: %s() => \"%s\"\n", __func__, buf);
#endif
	return mktime(&t);
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
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_INCOMING_MESSAGE);
}


/* gsm_trigger_cmut */
static int _gsm_trigger_cmut(GSM * gsm, char const * result)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u", &gsm->event.mute.mute) != 1)
		return 1;
	return _gsm_event_send(gsm, GSM_EVENT_TYPE_MUTE);
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
	else if(res == 3)
	{
		gsm->event.registration.stat = gsm->event.registration.n;
		gsm->event.registration.n = 2;
		ret = _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	}
	else if(res == 2)
	{
		gsm->event.registration.area = 0;
		gsm->event.registration.cell = 0;
		ret = _gsm_event_send(gsm, GSM_EVENT_TYPE_REGISTRATION);
	}
	else if(res == 1)
	{
		gsm->event.registration.stat = gsm->event.registration.n;
		gsm->event.registration.n = 2;
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


/* gsm_trigger_ext */
static int _gsm_trigger_ext_error(GSM * gsm, char const * result,
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
	for(i = 0; _gsm_ext_errors[i].error != NULL; i++)
		if(_gsm_ext_errors[i].code == code)
			break;
	if(_gsm_ext_errors[i].error != NULL)
		error = _(_gsm_ext_errors[i].error);
	if(gsm->queue != NULL && (gsmc = gsm->queue->data) != NULL)
		type = gsm_command_get_error(gsmc);
	return gsm_event(gsm, GSM_EVENT_TYPE_ERROR, type, error);
}


/* gsm_trigger_cssi */
static int _gsm_trigger_cssi(GSM * gsm, char const * result)
{
	unsigned int i;
	unsigned int index = 10; /* XXX not used */

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u,%u", &i, &index) < 1)
		return 1;
	if(i == 1) /* FIXME implement the rest */
		i = GSM_PHONE_ACTIVITY_CALL;
	else
		i = GSM_PHONE_ACTIVITY_UNKNOWN;
	return gsm_event(gsm, GSM_EVENT_TYPE_PHONE_ACTIVITY, i);
}


/* gsm_trigger_cssu */
static int _gsm_trigger_cssu(GSM * gsm, char const * result)
{
	unsigned int code;
	unsigned int index = 10;
	char number[32];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, result);
#endif
	if(sscanf(result, "%u,%u,\"%31[^\"]\"", &code, &index, number) < 1)
		return 1;
	number[sizeof(number) - 1] = '\0';
	/* FIXME implement */
	return 1;
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
			"No answer");
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
			"No carrier");
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
			"No dialtone");
}


/* callbacks */
/* on_reset */
static int _reset_do(GSM * gsm, int fd);
static gboolean _reset_settle(gpointer data);

static gboolean _on_reset(gpointer data)
{
	GSM * gsm = data;
	int fd;
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
		if(fd >= 0)
			close(fd);
		if(gsm->retry > 0)
			gsm->source = g_timeout_add(gsm->retry, _on_reset, gsm);
		gsm->source = 0;
		return error_set_code(FALSE, "%s%s%s", gsm->device, ": ",
				strerror(errno));
	}
	gsm->channel = g_io_channel_unix_new(fd);
	if((g_io_channel_set_encoding(gsm->channel, NULL, &error))
			!= G_IO_STATUS_NORMAL)
		error_set(0, "%s", error->message);
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
			error_set("%s", gsm->device); /* go on anyway */
		if(cfsetospeed(&term, gsm->baudrate) != 0)
			error_set("%s", gsm->device); /* go on anyway */
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
	char const * cmd = "AT";
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
	if(gsm->mode == GSM_MODE_PDU) /* let's get out of here */
		gsm->mode = GSM_MODE_COMMAND;
	else if(gsm->queue == NULL || (gsmc = gsm->queue->data) == NULL
			/* re-inject the command */
			|| (cmd = gsm_command_get_command(gsmc)) == NULL)
		return FALSE;
	len = strlen(cmd) + 2;
	if((p = realloc(gsm->wr_buf, len + 1)) == NULL)
		return FALSE;
	gsm->wr_buf = p;
	snprintf(gsm->wr_buf, len + 1, "%s%s", cmd, "\r\n");
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
			error_set("%s", error->message); /* XXX really print */
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
	unsigned int timeout = 2000;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__,
			(unsigned long)gsm->wr_buf_cnt);
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
			error_set("%s", error->message); /* XXX really print */
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
		{
			gsm->mode = gsm_command_get_mode(gsmc);
			timeout = gsm_command_get_timeout(gsmc);
		}
		if(gsm->source != 0)
			g_source_remove(gsm->source);
		if(timeout != 0)
			gsm->source = g_timeout_add(timeout, _on_timeout, gsm);
	}
	return FALSE;
}
