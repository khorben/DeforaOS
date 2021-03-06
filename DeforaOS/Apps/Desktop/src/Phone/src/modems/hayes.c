/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
/* FIXME:
 * - implement *#06# (obtain IMEI)
 * - implement priorities again
 * - parse messages from within +CMGL already
 * - test with a SIM card without a PIN code
 * - add MCT callbacks/buttons to change the SIM code (via a helper in phone.c)
 * - implement new contacts */



#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <errno.h>
#include <glib.h>
#include <System.h>
#include <Phone/modem.h>
#include "hayes.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* Hayes */
/* private */
/* types */
typedef enum _HayesMode
{
	HAYES_MODE_INIT = 0,
	HAYES_MODE_COMMAND,
	HAYES_MODE_DATA
} HayesMode;

typedef struct _ModemPlugin
{
	ModemPluginHelper * helper;

	unsigned int retry;
	unsigned int quirks;

	/* modem */
	guint source;
	guint timeout;
	GIOChannel * channel;
	char * rd_buf;
	size_t rd_buf_cnt;
	guint rd_source;
	char * wr_buf;
	size_t wr_buf_cnt;
	guint wr_source;
	GIOChannel * rd_ppp_channel;
	guint rd_ppp_source;
	GIOChannel * wr_ppp_channel;
	guint wr_ppp_source;

	/* logging */
	FILE * fp;

	/* queue */
	HayesMode mode;
	GSList * queue;
	GSList * queue_timeout;

	/* internal */
	ModemEvent events[MODEM_EVENT_TYPE_COUNT];
	char * authentication_name;
	char * call_number;
	char * contact_name;
	char * contact_number;
	char * gprs_username;
	char * gprs_password;
	char * message_number;
	char * model_name;
	char * model_vendor;
	char * model_version;
	char * registration_media;
	char * registration_operator;
} Hayes;

typedef enum _HayesCommandPriority
{
	HCP_LOWER = 0,
	HCP_NORMAL,
	HCP_HIGHER,
	HCP_IMMEDIATE
} HayesCommandPriority;

typedef enum _HayesCommandStatus
{
	HCS_PENDING = 0,
	HCS_QUEUED,
	HCS_ACTIVE,
	HCS_TIMEOUT,
	HCS_ERROR,
	HCS_SUCCESS
} HayesCommandStatus;

typedef struct _HayesCommand HayesCommand;

typedef HayesCommandStatus (*HayesCommandCallback)(HayesCommand * command,
		HayesCommandStatus status, void * priv);

struct _HayesCommand
{
	HayesCommandPriority priority;
	HayesCommandStatus status;

	/* request */
	char * attention;
	unsigned int timeout;
	HayesCommandCallback callback;
	void * priv;

	/* answer */
	char * answer;

	/* XXX should be handled a better way */
	void * data;
};

typedef struct _HayesRequestContactList
{
	unsigned int from;
	unsigned int to;
} HayesRequestContactList;

typedef struct _HayesRequestMessageData
{
	unsigned int id;
	ModemMessageFolder folder;
	ModemMessageStatus status;
} HayesRequestMessageData;

typedef enum _HayesQuirk
{
	HAYES_QUIRK_BATTERY_70			= 0x1,
	HAYES_QUIRK_CPIN_QUOTES			= 0x2,
	HAYES_QUIRK_CONNECTED_LINE_DISABLED	= 0x4,
	HAYES_QUIRK_WANT_SMSC_IN_PDU		= 0x8,
	HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR	= 0x10
} HayesQuirk;

typedef struct _HayesRequestHandler
{
	unsigned int type;
	char const * attention;
	HayesCommandCallback callback;
} HayesRequestHandler;

typedef struct _HayesCodeHandler
{
	char const * code;
	void (*callback)(ModemPlugin * modem, char const * answer);
} HayesCodeHandler;


/* constants */
enum
{
	HAYES_REQUEST_ALIVE = MODEM_REQUEST_COUNT,
	HAYES_REQUEST_CALL_WAITING_UNSOLLICITED_DISABLE,
	HAYES_REQUEST_CALL_WAITING_UNSOLLICITED_ENABLE,
	HAYES_REQUEST_CONNECTED_LINE_DISABLE,
	HAYES_REQUEST_CONNECTED_LINE_ENABLE,
	HAYES_REQUEST_CONTACT_LIST,
	HAYES_REQUEST_EXTENDED_ERRORS,
	HAYES_REQUEST_EXTENDED_RING_REPORTS,
	HAYES_REQUEST_FUNCTIONAL,
	HAYES_REQUEST_FUNCTIONAL_DISABLE,
	HAYES_REQUEST_FUNCTIONAL_ENABLE,
	HAYES_REQUEST_FUNCTIONAL_ENABLE_RESET,
	HAYES_REQUEST_GPRS_ATTACHED,
	HAYES_REQUEST_LOCAL_ECHO_DISABLE,
	HAYES_REQUEST_LOCAL_ECHO_ENABLE,
	HAYES_REQUEST_MESSAGE_FORMAT_PDU,
	HAYES_REQUEST_MESSAGE_LIST_INBOX_READ,
	HAYES_REQUEST_MESSAGE_LIST_INBOX_UNREAD,
	HAYES_REQUEST_MESSAGE_LIST_SENT_READ,
	HAYES_REQUEST_MESSAGE_LIST_SENT_UNREAD,
	HAYES_REQUEST_MESSAGE_UNSOLLICITED_DISABLE,
	HAYES_REQUEST_MESSAGE_UNSOLLICITED_ENABLE,
	HAYES_REQUEST_MODEL,
	HAYES_REQUEST_OPERATOR,
	HAYES_REQUEST_OPERATOR_FORMAT_SHORT,
	HAYES_REQUEST_OPERATOR_FORMAT_LONG,
	HAYES_REQUEST_OPERATOR_FORMAT_NUMERIC,
	HAYES_REQUEST_PHONE_ACTIVE,
	HAYES_REQUEST_REGISTRATION,
	HAYES_REQUEST_REGISTRATION_AUTOMATIC,
	HAYES_REQUEST_REGISTRATION_DISABLED,
	HAYES_REQUEST_REGISTRATION_UNSOLLICITED_DISABLE,
	HAYES_REQUEST_REGISTRATION_UNSOLLICITED_ENABLE,
	HAYES_REQUEST_SIM_PIN_VALID,
	HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_CANCEL,
	HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_ENABLE,
	HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_DISABLE,
	HAYES_REQUEST_VENDOR,
	HAYES_REQUEST_VERBOSE_DISABLE,
	HAYES_REQUEST_VERBOSE_ENABLE,
	HAYES_REQUEST_VERSION
};


/* prototypes */
/* plug-in */
static ModemPlugin * _hayes_init(ModemPluginHelper * helper);
static void _hayes_destroy(ModemPlugin * modem);
static int _hayes_start(ModemPlugin * modem, unsigned int retry);
static int _hayes_stop(ModemPlugin * modem);
static int _hayes_request(ModemPlugin * modem, ModemRequest * request);
static int _hayes_trigger(ModemPlugin * modem, ModemEventType event);

/* accessors */
static void _hayes_set_mode(ModemPlugin * modem, HayesMode mode);

/* useful */
static int _hayes_request_type(ModemPlugin * modem, ModemRequestType type);

/* messages */
static char * _hayes_message_to_pdu(ModemPlugin * modem, char const * number,
		ModemMessageEncoding encoding, size_t length,
		char const * content);

/* conversions */
static unsigned char _hayes_convert_char_to_iso(unsigned char c);
static char * _hayes_convert_number_to_address(char const * number);

/* parser */
static int _hayes_parse(ModemPlugin * modem);
static int _hayes_parse_trigger(ModemPlugin * modem, char const * answer,
		HayesCommand * command);

/* queue */
static int _hayes_queue_command(ModemPlugin * modem, HayesCommand * command);
#if 0 /* XXX no longer used */
static int _hayes_queue_command_full(ModemPlugin * modem,
		char const * attention, HayesCommandCallback callback);
#endif
static void _hayes_queue_flush(ModemPlugin * modem);
static int _hayes_queue_pop(ModemPlugin * modem);
static int _hayes_queue_push(ModemPlugin * modem);

static void _hayes_reset(ModemPlugin * modem);
static void _hayes_reset_start(ModemPlugin * modem, unsigned int retry);
static void _hayes_reset_stop(ModemPlugin * modem);

/* commands */
static HayesCommand * _hayes_command_new(char const * attention);
static void _hayes_command_delete(HayesCommand * command);
static char const * _hayes_command_get_answer(HayesCommand * command);
static char const * _hayes_command_get_attention(HayesCommand * command);
static void * _hayes_command_get_data(HayesCommand * command);
#if 0 /* XXX no longer being used */
static char * _hayes_command_get_line(HayesCommand * command,
		char const * prefix);
#endif
static HayesCommandPriority _hayes_command_get_priority(HayesCommand * command);
static HayesCommandStatus _hayes_command_get_status(HayesCommand * command);
static unsigned int _hayes_command_get_timeout(HayesCommand * command);
static void _hayes_command_set_callback(HayesCommand * command,
		HayesCommandCallback callback, void * priv);
static void _hayes_command_set_data(HayesCommand * command, void * data);
static void _hayes_command_set_priority(HayesCommand * command,
		HayesCommandPriority priority);
static void _hayes_command_set_status(HayesCommand * command,
		HayesCommandStatus status);
static void _hayes_command_set_timeout(HayesCommand * command,
		unsigned int timeout);
static int _hayes_command_answer_append(HayesCommand * command,
		char const * answer);
static HayesCommandStatus _hayes_command_callback(HayesCommand * command);

/* callbacks */
static gboolean _on_queue_timeout(gpointer data);
static gboolean _on_reset(gpointer data);
static gboolean _on_timeout(gpointer data);
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_read_ppp(GIOChannel * source,
		GIOCondition condition, gpointer data);
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _on_watch_can_write_ppp(GIOChannel * source,
		GIOCondition condition, gpointer data);

static HayesCommandStatus _on_request_authenticate(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_battery_level(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_call(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_call_incoming(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_call_outgoing(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_call_status(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_contact_delete(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_functional(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_functional_enable(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_functional_enable_reset(
		HayesCommand * command, HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_generic(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_message(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_message_delete(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_message_list(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_model(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_registration(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_registration_automatic(
		HayesCommand * command, HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_registration_disabled(
		HayesCommand * command, HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_sim_pin_valid(HayesCommand * command,
		HayesCommandStatus status, void * priv);
static HayesCommandStatus _on_request_unsupported(HayesCommand * command,
		HayesCommandStatus status, void * priv);

static void _on_code_call_error(ModemPlugin * modem, char const * answer);
static void _on_code_cbc(ModemPlugin * modem, char const * answer);
static void _on_code_cfun(ModemPlugin * modem, char const * answer);
static void _on_code_cgatt(ModemPlugin * modem, char const * answer);
static void _on_code_cgmi(ModemPlugin * modem, char const * answer);
static void _on_code_cgmm(ModemPlugin * modem, char const * answer);
static void _on_code_cgmr(ModemPlugin * modem, char const * answer);
static void _on_code_clip(ModemPlugin * modem, char const * answer);
static void _on_code_cme_error(ModemPlugin * modem, char const * answer);
static void _on_code_cmgl(ModemPlugin * modem, char const * answer);
static void _on_code_cmgr(ModemPlugin * modem, char const * answer);
static void _on_code_cmgs(ModemPlugin * modem, char const * answer);
static void _on_code_cms_error(ModemPlugin * modem, char const * answer);
static void _on_code_cmti(ModemPlugin * modem, char const * answer);
static void _on_code_connect(ModemPlugin * modem, char const * answer);
static void _on_code_colp(ModemPlugin * modem, char const * answer);
static void _on_code_cops(ModemPlugin * modem, char const * answer);
static void _on_code_cpas(ModemPlugin * modem, char const * answer);
static void _on_code_cpbr(ModemPlugin * modem, char const * answer);
static void _on_code_cpin(ModemPlugin * modem, char const * answer);
static void _on_code_creg(ModemPlugin * modem, char const * answer);
static void _on_code_cring(ModemPlugin * modem, char const * answer);
static void _on_code_csq(ModemPlugin * modem, char const * answer);
static void _on_code_cusd(ModemPlugin * modem, char const * answer);
static void _on_code_ext_error(ModemPlugin * modem, char const * answer);

/* helpers */
static int _is_figure(int c);
static int _is_number(char const * number);


/* variables */
static ModemConfig _hayes_config[] =
{
	{ "device",	"Device",		MCT_FILENAME	},
	{ "baudrate",	"Baudrate",		MCT_UINT32	},
	{ "hwflow",	"Hardware flow control",MCT_BOOLEAN	},
	{ NULL,		"Advanced",		MCT_SUBSECTION	},
	{ "logfile",	"Log file",		MCT_FILENAME	},
	{ NULL,		NULL,			MCT_NONE	},
};

static struct
{
	unsigned char gsm;
	unsigned char iso;
} _hayes_gsm_iso[] =
{
	{ '\0', '@'	},
	{ 0x01, 163	}, /* £ */
	{ 0x02, '$'	},
	{ 0x03, 165	}, /* ¥ */
	{ 0x04, 232	}, /* è */
	{ 0x05, 233	}, /* é */
	{ 0x06, 249	}, /* ù */
	{ 0x07, 236	}, /* ì */
	{ 0x08, 242	}, /* ò */
	{ 0x09, 199	}, /* Ç */
	{ 0x0b, 216	}, /* Ø */
	{ 0x0c, 248	}, /* ø */
	{ 0x0e, 197	}, /* Å */
	{ 0x0f, 229	}, /* å */
	{ 0x10, ' '	}, /* XXX delta */
	{ 0x11, '_'	},
	{ 0x12, ' '	}, /* XXX phi */
	{ 0x13, ' '	}, /* XXX gamma */
	{ 0x14, ' '	}, /* XXX lambda */
	{ 0x15, ' '	}, /* XXX omega */
	{ 0x16, ' '	}, /* XXX pi */
	{ 0x17, ' '	}, /* XXX psi */
	{ 0x18, ' '	}, /* XXX sigma */
	{ 0x19, ' '	}, /* XXX theta */
	{ 0x1a, ' '	}, /* XXX xi */
	{ 0x1b, ' '	}, /* FIXME escape */
	{ 0x1c, 198	},
	{ 0x1d, 230	},
	{ 0x1e, 223	},
	{ 0x1f, 201	},
	{ 0x24, 164	}, /* $ */
	{ 0x40, 161	}, /* @ */
	{ 0x5b, 196	}, /* [ */
	{ 0x5c, 214	}, /* \ */
	{ 0x5d, 209	}, /* ] */
	{ 0x5e, 220	}, /* ^ */
	{ 0x5f, 167	}, /* _ */
	{ 0x60, 191	}, /* ` */
	{ 0x7b, 228	}, /* { */
	{ 0x7c, 246	}, /* | */
	{ 0x7d, 241	}, /* } */
	{ 0x7e, 252	}, /* ~ */
	{ 0x7f, 224	}
};

static const struct
{
	char const * model;
	unsigned int quirks;
} _hayes_quirks[] =
{
	{ "\"Neo1973 Embedded GSM Modem\"",
		HAYES_QUIRK_CPIN_QUOTES | HAYES_QUIRK_WANT_SMSC_IN_PDU
			| HAYES_QUIRK_CONNECTED_LINE_DISABLED
			| HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR		},
	{ "\"Neo1973 GTA01/GTA02 Embedded GSM Modem\"",
		HAYES_QUIRK_CPIN_QUOTES | HAYES_QUIRK_WANT_SMSC_IN_PDU
			| HAYES_QUIRK_CONNECTED_LINE_DISABLED
			| HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR		},
	{ "\"Neo1973 GTA02 Embedded GSM Modem\"",
		HAYES_QUIRK_CPIN_QUOTES | HAYES_QUIRK_WANT_SMSC_IN_PDU
			| HAYES_QUIRK_CONNECTED_LINE_DISABLED
			| HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR		},
	{ "Nokia N900",
		HAYES_QUIRK_CPIN_QUOTES | HAYES_QUIRK_BATTERY_70	},
	{ NULL,			0					}
};

static HayesRequestHandler _hayes_request_handlers[] =
{
	{ HAYES_REQUEST_ALIVE,				"AT",
		_on_request_generic },
	{ HAYES_REQUEST_CALL_WAITING_UNSOLLICITED_DISABLE,"AT+CCWA=1",
		_on_request_generic },
	{ HAYES_REQUEST_CALL_WAITING_UNSOLLICITED_ENABLE,"AT+CCWA=1",
		_on_request_generic },
	{ HAYES_REQUEST_CONNECTED_LINE_DISABLE,		"AT+COLP=0",
		_on_request_generic },
	{ HAYES_REQUEST_CONNECTED_LINE_ENABLE,		"AT+COLP=1",
		_on_request_generic },
	{ HAYES_REQUEST_CONTACT_LIST,			NULL,
		_on_request_generic },
	{ HAYES_REQUEST_EXTENDED_ERRORS,		"AT+CMEE=1",
		_on_request_generic },
	{ HAYES_REQUEST_EXTENDED_RING_REPORTS,		"AT+CRC=1",
		_on_request_generic },
	{ HAYES_REQUEST_FUNCTIONAL,			"AT+CFUN?",
		_on_request_functional },
	{ HAYES_REQUEST_FUNCTIONAL_DISABLE,		"AT+CFUN=0",
		_on_request_generic },
	{ HAYES_REQUEST_FUNCTIONAL_ENABLE,		"AT+CFUN=1",
		_on_request_functional_enable },
	{ HAYES_REQUEST_FUNCTIONAL_ENABLE_RESET,	"AT+CFUN=1,1",
		_on_request_functional_enable_reset },
	{ HAYES_REQUEST_GPRS_ATTACHED,			"AT+CGATT?",
		_on_request_generic },
	{ HAYES_REQUEST_LOCAL_ECHO_DISABLE,		"ATE0",
		_on_request_generic },
	{ HAYES_REQUEST_LOCAL_ECHO_ENABLE,		"ATE1",
		_on_request_generic },
	{ HAYES_REQUEST_MESSAGE_FORMAT_PDU,		"AT+CMGF=0",
		_on_request_generic },
	{ HAYES_REQUEST_MESSAGE_LIST_INBOX_UNREAD,	"AT+CMGL=0",
		_on_request_message_list },
	{ HAYES_REQUEST_MESSAGE_LIST_INBOX_READ,	"AT+CMGL=1",
		_on_request_message_list },
	{ HAYES_REQUEST_MESSAGE_LIST_SENT_UNREAD,	"AT+CMGL=2",
		_on_request_message_list },
	{ HAYES_REQUEST_MESSAGE_LIST_SENT_READ,		"AT+CMGL=3",
		_on_request_message_list },
	{ HAYES_REQUEST_MESSAGE_UNSOLLICITED_DISABLE,	"AT+CNMI=0",
		_on_request_generic },
	{ HAYES_REQUEST_MESSAGE_UNSOLLICITED_ENABLE,	"AT+CNMI=1",
		_on_request_generic }, /* XXX report error? */
	{ HAYES_REQUEST_MODEL,				"AT+CGMM",
		_on_request_model },
	{ HAYES_REQUEST_OPERATOR,			"AT+COPS?",
		_on_request_generic },
	{ HAYES_REQUEST_OPERATOR_FORMAT_LONG,		"AT+COPS=3,0",
		_on_request_registration },
	{ HAYES_REQUEST_OPERATOR_FORMAT_NUMERIC,	"AT+COPS=3,2",
		_on_request_registration },
	{ HAYES_REQUEST_OPERATOR_FORMAT_SHORT,		"AT+COPS=3,1",
		_on_request_registration },
	{ HAYES_REQUEST_PHONE_ACTIVE,			"AT+CPAS",
		_on_request_call },
	{ HAYES_REQUEST_REGISTRATION,			"AT+CREG?",
		_on_request_generic },
	{ HAYES_REQUEST_REGISTRATION_AUTOMATIC,		"AT+COPS=0",
		_on_request_registration_automatic },
	{ HAYES_REQUEST_REGISTRATION_DISABLED,		"AT+COPS=2",
		_on_request_registration_disabled },
	{ HAYES_REQUEST_REGISTRATION_UNSOLLICITED_DISABLE,"AT+CREG=0",
		_on_request_generic },
	{ HAYES_REQUEST_REGISTRATION_UNSOLLICITED_ENABLE,"AT+CREG=2",
		_on_request_registration },
	{ HAYES_REQUEST_SIM_PIN_VALID,			"AT+CPIN?",
		_on_request_sim_pin_valid },
	{ HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_CANCEL,"AT+CUSD=2",
		_on_request_generic },
	{ HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_DISABLE,"AT+CUSD=0",
		_on_request_generic },
	{ HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_ENABLE,"AT+CUSD=1",
		_on_request_generic },
	{ HAYES_REQUEST_VENDOR,				"AT+CGMI",
		_on_request_model },
	{ HAYES_REQUEST_VERBOSE_DISABLE,		"ATV0",
		_on_request_generic },
	{ HAYES_REQUEST_VERBOSE_ENABLE,			"ATV1",
		_on_request_generic },
	{ HAYES_REQUEST_VERSION,			"AT+CGMR",
		_on_request_model },
	{ MODEM_REQUEST_AUTHENTICATE,			NULL,
		_on_request_authenticate },
	{ MODEM_REQUEST_BATTERY_LEVEL,			"AT+CBC",
		_on_request_battery_level },
	{ MODEM_REQUEST_CALL,				NULL,
		_on_request_call_outgoing },
	{ MODEM_REQUEST_CALL_ANSWER,			"ATA",
		_on_request_call_incoming },
	{ MODEM_REQUEST_CALL_HANGUP,			NULL,
		_on_request_call_status },
	{ MODEM_REQUEST_CALL_PRESENTATION,		NULL,
		_on_request_generic },
	{ MODEM_REQUEST_CONNECTIVITY,			NULL,
		_on_request_generic },
	{ MODEM_REQUEST_CONTACT_DELETE,			NULL,
		_on_request_contact_delete },
	{ MODEM_REQUEST_CONTACT_LIST,			"AT+CPBR=?",
		_on_request_generic },
	{ MODEM_REQUEST_DTMF_SEND,			NULL,
		_on_request_generic },
	{ MODEM_REQUEST_MESSAGE,			NULL,
		_on_request_message },
	{ MODEM_REQUEST_MESSAGE_DELETE,			NULL,
		_on_request_message_delete },
	{ MODEM_REQUEST_MESSAGE_LIST,			NULL,
		_on_request_message_list },
	{ MODEM_REQUEST_MESSAGE_SEND,			NULL,
		_on_request_generic },
	{ MODEM_REQUEST_REGISTRATION,			NULL,
		_on_request_registration },
	{ MODEM_REQUEST_SIGNAL_LEVEL,			"AT+CSQ",
		_on_request_generic },
	{ MODEM_REQUEST_UNSUPPORTED,			NULL,
		_on_request_unsupported }
};

static HayesCodeHandler _hayes_code_handlers[] =
{
	{ "+CBC",	_on_code_cbc		},
	{ "+CFUN",	_on_code_cfun		},
	{ "+CGATT",	_on_code_cgatt		},
	{ "+CGMI",	_on_code_cgmi		},
	{ "+CGMM",	_on_code_cgmm		},
	{ "+CGMR",	_on_code_cgmr		},
	{ "+CLIP",	_on_code_clip		},
	{ "+CME ERROR",	_on_code_cme_error	},
	{ "+CMGL",	_on_code_cmgl		},
	{ "+CMGR",	_on_code_cmgr		},
	{ "+CMGS",	_on_code_cmgs		},
	{ "+CMS ERROR",	_on_code_cms_error	},
	{ "+CMTI",	_on_code_cmti		},
	{ "+COLP",	_on_code_colp		},
	{ "+COPS",	_on_code_cops		},
	{ "+CPAS",	_on_code_cpas		},
	{ "+CPBR",	_on_code_cpbr		},
	{ "+CPIN",	_on_code_cpin		},
	{ "+CREG",	_on_code_creg		},
	{ "+CRING",	_on_code_cring		},
	{ "+CSQ",	_on_code_csq		},
	{ "+CUSD",	_on_code_cusd		},
	{ "+EXT ERROR",	_on_code_ext_error	},
	{ "BUSY",	_on_code_call_error	},
	{ "CONNECT",	_on_code_connect	},
	{ "NO CARRIER",	_on_code_call_error	},
	{ "NO DIALTONE",_on_code_call_error	},
	{ "RING",	_on_code_cring		}
};


/* public */
/* variables */
ModemPluginDefinition plugin =
{
	"Hayes",
	NULL,
	_hayes_config,
	_hayes_init,
	_hayes_destroy,
	_hayes_start,
	_hayes_stop,
	_hayes_request,
	_hayes_trigger
};


/* private */
/* plug-in */
/* functions */
static ModemPlugin * _hayes_init(ModemPluginHelper * helper)
{
	Hayes * hayes;
	size_t i;

	if((hayes = object_new(sizeof(*hayes))) == NULL)
		return NULL;
	memset(hayes, 0, sizeof(*hayes));
	hayes->helper = helper;
	hayes->mode = HAYES_MODE_INIT;
	for(i = 0; i < sizeof(hayes->events) / sizeof(*hayes->events); i++)
		hayes->events[i].type = i;
	hayes->events[MODEM_EVENT_TYPE_REGISTRATION].registration.signal
		= 0.0 / 0.0;
	return hayes;
}


/* hayes_destroy */
static void _hayes_destroy(ModemPlugin * modem)
{
	Hayes * hayes = modem;

	_hayes_stop(modem);
	object_delete(hayes);
}


/* hayes_request */
static int _request_do(ModemPlugin * modem, ModemRequest * request,
		void * data);
static char * _request_attention(ModemPlugin * modem, ModemRequest * request);
static char * _request_attention_apn(char const * protocol, char const * apn);
static char * _request_attention_call(ModemPlugin * modem,
		ModemRequest * request);
static char * _request_attention_call_ussd(ModemPlugin * modem,
		ModemRequest * request);
static char * _request_attention_call_hangup(ModemPlugin * modem);
static char * _request_attention_connectivity(ModemPlugin * modem,
		unsigned int enabled);
static char * _request_attention_contact_delete(ModemPlugin * modem,
		unsigned int id);
static char * _request_attention_contact_list(ModemRequest * request);
static char * _request_attention_dtmf_send(ModemRequest * request);
static char * _request_attention_gprs(ModemPlugin * modem,
		char const * username, char const * password);
static char * _request_attention_message(ModemPlugin * modem, unsigned int id);
static char * _request_attention_message_delete(ModemPlugin * modem,
		unsigned int id);
static char * _request_attention_message_list(ModemPlugin * modem);
static char * _request_attention_message_send(ModemPlugin * modem,
		char const * number, ModemMessageEncoding encoding,
		size_t length, char const * content);
static char * _request_attention_registration(ModemPlugin * modem,
		ModemRegistrationMode mode, char const * _operator);
static char * _request_attention_sim_pin(ModemPlugin * modem,
		char const * password);
static char * _request_attention_sim_puk(ModemPlugin * modem,
		char const * password);
static char * _request_attention_unsupported(ModemPlugin * modem,
		ModemRequest * request);

static int _hayes_request(ModemPlugin * modem, ModemRequest * request)
{
	return _request_do(modem, request, NULL);
}

static int _request_do(ModemPlugin * modem, ModemRequest * request, void * data)
{
	Hayes * hayes = modem;
	HayesCommand * command;
	unsigned int type = request->type;
	size_t i;
	size_t count = sizeof(_hayes_request_handlers)
		/ sizeof(*_hayes_request_handlers);
	char const * attention;
	char * p = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, (request != NULL)
			? request->type : (unsigned)-1);
#endif
	if(request == NULL)
		return -1;
	if(hayes->quirks & HAYES_QUIRK_CONNECTED_LINE_DISABLED
			&& type == HAYES_REQUEST_CONNECTED_LINE_ENABLE)
		request->type = HAYES_REQUEST_CONNECTED_LINE_DISABLE;
	for(i = 0; i < count; i++)
		if(_hayes_request_handlers[i].type == request->type)
			break;
	if(i == count)
#ifdef DEBUG
		return -hayes->helper->error(hayes->helper->modem,
				"Unable to handle request", 1);
#else
		return -1;
#endif
	if((attention = _hayes_request_handlers[i].attention) == NULL)
	{
		if((p = _request_attention(modem, request)) == NULL)
			return 0; /* XXX errors should not be ignored */
		attention = p;
	}
	/* XXX using _hayes_queue_command_full() was more elegant */
	command = _hayes_command_new(attention);
	free(p);
	if(command == NULL)
		return -1;
	_hayes_command_set_callback(command,
			_hayes_request_handlers[i].callback, modem);
	if(_hayes_queue_command(modem, command) != 0)
	{
		_hayes_command_delete(command);
		return -1;
	}
	if(data != NULL)
		_hayes_command_set_data(command, data);
	return 0;
}

static char * _request_attention(ModemPlugin * modem, ModemRequest * request)
{
	unsigned int type = request->type;
	char buf[32];
	char const * p;
	size_t len;

	switch(type)
	{
		case HAYES_REQUEST_CONTACT_LIST:
			return _request_attention_contact_list(request);
		case MODEM_REQUEST_AUTHENTICATE:
			if(strcmp(request->authenticate.name, "APN") == 0)
				return _request_attention_apn(
						request->authenticate.username,
						request->authenticate.password);
			if(strcmp(request->authenticate.name, "GPRS") == 0)
				return _request_attention_gprs(modem,
						request->authenticate.username,
						request->authenticate.password);
			if(strcmp(request->authenticate.name, "SIM PIN") == 0)
				return _request_attention_sim_pin(modem,
						request->authenticate.password);
			if(strcmp(request->authenticate.name, "SIM PUK") == 0)
				return _request_attention_sim_puk(modem,
						request->authenticate.password);
			break;
		case MODEM_REQUEST_CALL:
			if(request->call.call_type == MODEM_CALL_TYPE_VOICE
					&& (p = request->call.number) != NULL
					&& (len = strlen(p)) > 2
					&& p[0] == '*' && p[len - 1] == '#')
				return _request_attention_call_ussd(modem,
						request);
			return _request_attention_call(modem, request);
		case MODEM_REQUEST_CALL_HANGUP:
			return _request_attention_call_hangup(modem);
		case MODEM_REQUEST_CALL_PRESENTATION:
			snprintf(buf, sizeof(buf), "%s%u", "AT+CLIP=",
					request->call_presentation.enabled
					? 1 : 0);
			return strdup(buf);
		case MODEM_REQUEST_CONNECTIVITY:
			return _request_attention_connectivity(modem,
					request->connectivity.enabled);
		case MODEM_REQUEST_CONTACT_DELETE:
			return _request_attention_contact_delete(modem,
					request->contact_delete.id);
		case MODEM_REQUEST_DTMF_SEND:
			return _request_attention_dtmf_send(request);
		case MODEM_REQUEST_MESSAGE:
			return _request_attention_message(modem,
					request->message.id);
		case MODEM_REQUEST_MESSAGE_LIST:
			return _request_attention_message_list(modem);
		case MODEM_REQUEST_MESSAGE_DELETE:
			return _request_attention_message_delete(modem,
					request->message_delete.id);
		case MODEM_REQUEST_MESSAGE_SEND:
			return _request_attention_message_send(modem,
					request->message_send.number,
					request->message_send.encoding,
					request->message_send.length,
					request->message_send.content);
		case MODEM_REQUEST_REGISTRATION:
			return _request_attention_registration(modem,
					request->registration.mode,
					request->registration._operator);
		case MODEM_REQUEST_UNSUPPORTED:
			return _request_attention_unsupported(modem, request);
		default:
			break;
	}
	return NULL;
}

static char * _request_attention_apn(char const * protocol, char const * apn)
{
	char * ret;
	const char cmd[] = "AT+CGDCONT=1,";
	size_t len;

	if(protocol == NULL || apn == NULL)
		return NULL;
	len = sizeof(cmd) + strlen(protocol) + 2 + strlen(apn) + 3;
	if((ret = malloc(len)) == NULL)
		return NULL;
	snprintf(ret, len, "%s\"%s\",\"%s\"", cmd, protocol, apn);
	return ret;
}

static char * _request_attention_call(ModemPlugin * modem,
		ModemRequest * request)
{
	char * ret;
	Hayes * hayes = modem;
	char const * number = request->call.number;
	ModemEvent * event;
	const char cmd[] = "ATD";
	const char anonymous[] = "I";
	const char voice[] = ";";
	size_t len;

	if(request->call.number == NULL)
		request->call.number = "";
	if(request->call.number[0] == '\0')
		number = "L";
	else if(!_is_number(request->call.number))
		return NULL;
	event = &hayes->events[MODEM_EVENT_TYPE_CALL];
	/* XXX should really be set at the time of the call */
	event->call.call_type = request->call.call_type;
	free(hayes->call_number);
	if(request->call.call_type == MODEM_CALL_TYPE_DATA)
		hayes->call_number = NULL;
	else if((hayes->call_number = strdup(request->call.number)) == NULL)
		return NULL;
	event->call.number = hayes->call_number;
	len = sizeof(cmd) + strlen(number) + sizeof(anonymous) + sizeof(voice);
	if((ret = malloc(len)) == NULL)
		return NULL;
	snprintf(ret, len, "%s%s%s%s", cmd, number,
			(request->call.anonymous) ? anonymous : "",
			(request->call.call_type == MODEM_CALL_TYPE_VOICE)
			? voice : "");
	return ret;
}

static char * _request_attention_call_ussd(ModemPlugin * modem,
		ModemRequest * request)
{
	char * ret;
	char const * number = request->call.number;
	const char cmd[] = "AT+CUSD=1,";
	size_t len;

	if(request->call.number == NULL || request->call.number[0] == '\0')
		return NULL;
	len = sizeof(cmd) + strlen(number) + 2;
	if((ret = malloc(len)) == NULL)
		return NULL;
	/* XXX may also require setting dcs */
	snprintf(ret, len, "%s\"%s\"", cmd, number);
	return ret;
}

static char * _request_attention_call_hangup(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];

	/* FIXME check that this works on all phones, including:
	 * - while calling:
	 *   . still ringing => simply inject "\r\n"?
	 *   . in the queue => simply remove?
	 * - while ringing (incoming) */
	if(hayes->mode == HAYES_MODE_DATA)
	{
		event->connection.connected = 0;
		event->connection.in = 0;
		event->connection.out = 0;
		hayes->helper->event(hayes->helper->modem, event);
		_hayes_set_mode(modem, HAYES_MODE_INIT);
		return NULL;
	}
	/* return "ATH" if currently ringing */
	event = &hayes->events[MODEM_EVENT_TYPE_CALL];
	if(event->call.direction == MODEM_CALL_DIRECTION_INCOMING
			&& event->call.status == MODEM_CALL_STATUS_RINGING)
		return strdup("ATH");
	/* force all calls to terminate */
	return strdup("AT+CHUP");
}

static char * _request_attention_connectivity(ModemPlugin * modem,
		unsigned int enabled)
{
	_hayes_request_type(modem, enabled ? HAYES_REQUEST_FUNCTIONAL_ENABLE
		: HAYES_REQUEST_FUNCTIONAL_DISABLE);
	return NULL;
}

static char * _request_attention_contact_delete(ModemPlugin * modem,
		unsigned int id)
{
	Hayes * hayes = modem;
	char const cmd[] = "AT+CPBW=";
	char buf[32];

	/* FIXME store in the command itself */
	hayes->events[MODEM_EVENT_TYPE_CONTACT_DELETED].contact_deleted.id = id;
	snprintf(buf, sizeof(buf), "%s%u%s", cmd, id, ",");
	return strdup(buf);
}

static char * _request_attention_contact_list(ModemRequest * request)
{
	HayesRequestContactList * list = request->plugin.data;
	const char cmd[] = "AT+CPBR=";
	char buf[32];

	if(list->to < list->from)
		list->to = list->from;
	snprintf(buf, sizeof(buf), "%s%u,%u", cmd, list->from, list->to);
	return strdup(buf);
}

static char * _request_attention_dtmf_send(ModemRequest * request)
{
	const char cmd[] = "AT+VTS=";
	char buf[32];
	unsigned int dtmf = request->dtmf_send.dtmf;

	if((dtmf < '0' || dtmf > '9') && (dtmf < 'A' || dtmf > 'D')
			&& dtmf != '*' && dtmf != '#')
		return NULL;
	snprintf(buf, sizeof(buf), "%s%c", cmd, dtmf);
	return strdup(buf);
}

static char * _request_attention_gprs(ModemPlugin * modem,
		char const * username, char const * password)
{
	Hayes * hayes = modem;

	free(hayes->gprs_username);
	hayes->gprs_username = (username != NULL) ? strdup(username) : NULL;
	free(hayes->gprs_password);
	hayes->gprs_password = (password != NULL) ? strdup(password) : NULL;
	return NULL; /* we don't need to issue any command */
}

static char * _request_attention_message(ModemPlugin * modem, unsigned int id)
{
	char const cmd[] = "AT+CMGR=";
	char buf[32];

	/* FIXME force the message format to be in PDU mode? */
	snprintf(buf, sizeof(buf), "%s%u", cmd, id);
	return strdup(buf);
}

static char * _request_attention_message_delete(ModemPlugin * modem,
		unsigned int id)
{
	Hayes * hayes = modem;
	char const cmd[] = "AT+CMGD=";
	char buf[32];

	/* FIXME store in the command itself */
	hayes->events[MODEM_EVENT_TYPE_MESSAGE_DELETED].message_deleted.id = id;
	snprintf(buf, sizeof(buf), "%s%u", cmd, id);
	return strdup(buf);
}

static char * _request_attention_message_list(ModemPlugin * modem)
{
	ModemRequest request;
	HayesRequestMessageData * data;

	memset(&request, 0, sizeof(request));
	/* request received unread messages */
	request.type = HAYES_REQUEST_MESSAGE_LIST_INBOX_UNREAD;
	if((data = malloc(sizeof(*data))) != NULL)
	{
		data->id = 0;
		data->folder = MODEM_MESSAGE_FOLDER_INBOX;
		data->status = MODEM_MESSAGE_STATUS_UNREAD;
	}
	if(_request_do(modem, &request, data) != 0)
		free(data);
	/* request received read messages */
	request.type = HAYES_REQUEST_MESSAGE_LIST_INBOX_READ;
	if((data = malloc(sizeof(*data))) != NULL)
	{
		data->id = 0;
		data->folder = MODEM_MESSAGE_FOLDER_INBOX;
		data->status = MODEM_MESSAGE_STATUS_READ;
	}
	if(_request_do(modem, &request, data) != 0)
		free(data);
	/* request sent unread messages */
	request.type = HAYES_REQUEST_MESSAGE_LIST_SENT_UNREAD;
	if((data = malloc(sizeof(*data))) != NULL)
	{
		data->id = 0;
		data->folder = MODEM_MESSAGE_FOLDER_OUTBOX;
		data->status = MODEM_MESSAGE_STATUS_UNREAD;
	}
	if(_request_do(modem, &request, data) != 0)
		free(data);
	/* request sent read messages */
	request.type = HAYES_REQUEST_MESSAGE_LIST_SENT_READ;
	if((data = malloc(sizeof(*data))) != NULL)
	{
		data->id = 0;
		data->folder = MODEM_MESSAGE_FOLDER_OUTBOX;
		data->status = MODEM_MESSAGE_STATUS_READ;
	}
	if(_request_do(modem, &request, data) != 0)
		free(data);
	return NULL;
}

static char * _request_attention_message_send(ModemPlugin * modem,
		char const * number, ModemMessageEncoding encoding,
		size_t length, char const * content)
{
	Hayes * hayes = modem;
	char * ret;
	char const cmd[] = "AT+CMGS=";
	char * pdu;
	size_t pdulen;
	size_t len;

	if(_hayes_request_type(modem, HAYES_REQUEST_MESSAGE_FORMAT_PDU) != 0)
		return NULL;
	if((pdu = _hayes_message_to_pdu(modem, number, encoding, length,
					content)) == NULL)
		return NULL;
	pdulen = strlen(pdu);
	len = sizeof(cmd) + 10 + pdulen + 1;
	ret = malloc(len);
	if(hayes->quirks & HAYES_QUIRK_WANT_SMSC_IN_PDU)
		pdulen-=2;
	/* FIXME really issue using two separate commands */
	if(ret != NULL)
		snprintf(ret, len, "%s%lu\r\n%s", cmd, ((unsigned long)pdulen
					- 1) / 2, pdu);
	free(pdu);
	return ret;
}

static char * _request_attention_registration(ModemPlugin * modem,
		ModemRegistrationMode mode, char const * _operator)
{
	char const cops[] = "AT+COPS=";
	size_t len = sizeof(cops) + 5;
	char * p;

	switch(mode)
	{
		case MODEM_REGISTRATION_MODE_AUTOMATIC:
			_hayes_request_type(modem,
					HAYES_REQUEST_REGISTRATION_AUTOMATIC);
			break;
		case MODEM_REGISTRATION_MODE_DISABLED:
			_hayes_request_type(modem,
					HAYES_REQUEST_REGISTRATION_DISABLED);
			break;
		case MODEM_REGISTRATION_MODE_MANUAL:
			if(_operator == NULL)
				return NULL;
			len += strlen(_operator);
			if((p = malloc(len)) == NULL)
				return NULL;
			snprintf(p, len, "%s=1,0,%s", cops, _operator);
			return p;
		case MODEM_REGISTRATION_MODE_UNKNOWN:
			break;
	}
	return NULL;
}

static char * _request_attention_sim_pin(ModemPlugin * modem,
		char const * password)
{
	char * ret;
	Hayes * hayes = modem;
	const char cmd[] = "AT+CPIN=";
	size_t len;
	char const * format;

	if(password == NULL)
		return NULL;
	len = sizeof(cmd) + strlen(password) + 2;
	if((ret = malloc(len)) == NULL)
		return NULL;
	format = (hayes->quirks & HAYES_QUIRK_CPIN_QUOTES) ? "%s\"%s\""
		: "%s%s";
	snprintf(ret, len, format, cmd, password);
	return ret;
}

static char * _request_attention_sim_puk(ModemPlugin * modem,
		char const * password)
{
	char * ret;
	Hayes * hayes = modem;
	const char cmd[] = "AT+CPIN=";
	size_t len;
	char const * format;

	if(password == NULL)
		return NULL;
	len = sizeof(cmd) + strlen(password) + 3;
	if((ret = malloc(len)) == NULL)
		return NULL;
	format = (hayes->quirks & HAYES_QUIRK_CPIN_QUOTES) ? "%s\"%s\","
		: "%s%s,";
	snprintf(ret, len, format, cmd, password);
	return ret;
}

static char * _request_attention_unsupported(ModemPlugin * modem,
		ModemRequest * request)
{
	HayesRequest * hrequest = request->unsupported.request;

	if(strcmp(request->unsupported.modem, plugin.name) != 0)
		return NULL;
	if(request->unsupported.size != sizeof(*hrequest))
		return NULL;
	switch(request->unsupported.request_type)
	{
		case HAYES_REQUEST_COMMAND_QUEUE:
			return strdup(hrequest->command_queue.command);
		default:
			return NULL;
	}
}


/* hayes_start */
static int _hayes_start(ModemPlugin * modem, unsigned int retry)
{
	_hayes_reset_start(modem, retry);
	return 0;
}


/* hayes_stop */
static int _hayes_stop(ModemPlugin * modem)
{
	_hayes_reset_stop(modem);
	return 0;
}


/* hayes_trigger */
static int _hayes_trigger(ModemPlugin * modem, ModemEventType event)
{
	int ret = 0;
	Hayes * hayes = modem;
	ModemEvent * e;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, event);
#endif
	switch(event)
	{
		case MODEM_EVENT_TYPE_BATTERY_LEVEL: /* use the existing data */
		case MODEM_EVENT_TYPE_CALL:
		case MODEM_EVENT_TYPE_CONNECTION:
		case MODEM_EVENT_TYPE_STATUS:
			e = &hayes->events[event];
			hayes->helper->event(hayes->helper->modem, e);
			break;
		case MODEM_EVENT_TYPE_AUTHENTICATION:
			return _hayes_request_type(modem,
					HAYES_REQUEST_SIM_PIN_VALID);
		case MODEM_EVENT_TYPE_CONTACT:
			return _hayes_request_type(modem,
					MODEM_REQUEST_CONTACT_LIST);
		case MODEM_EVENT_TYPE_MESSAGE:
			return _hayes_request_type(modem,
					MODEM_REQUEST_MESSAGE_LIST);
		case MODEM_EVENT_TYPE_MODEL:
			ret |= _hayes_request_type(modem, HAYES_REQUEST_VENDOR);
			ret |= _hayes_request_type(modem, HAYES_REQUEST_MODEL);
			ret |= _hayes_request_type(modem,
					HAYES_REQUEST_VERSION);
			break;
		case MODEM_EVENT_TYPE_REGISTRATION:
			e = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
			if(e->registration.status
					== MODEM_REGISTRATION_STATUS_UNKNOWN)
				ret |= _hayes_request_type(modem,
						HAYES_REQUEST_REGISTRATION);
			else
				hayes->helper->event(hayes->helper->modem, e);
			break;
		case MODEM_EVENT_TYPE_CONTACT_DELETED: /* do not make sense */
		case MODEM_EVENT_TYPE_ERROR:
		case MODEM_EVENT_TYPE_NOTIFICATION:
		case MODEM_EVENT_TYPE_MESSAGE_DELETED:
		case MODEM_EVENT_TYPE_MESSAGE_SENT:
			ret = -1;
			break;
	}
	return ret;
}


/* accessors */
static void _hayes_set_mode(ModemPlugin * modem, HayesMode mode)
{
	Hayes * hayes = modem;
	ModemEvent * event;

	if(hayes->mode == mode)
		return;
	switch(hayes->mode)
	{
		case HAYES_MODE_INIT:
		case HAYES_MODE_COMMAND:
			break; /* nothing to do */
		case HAYES_MODE_DATA:
			if(hayes->rd_ppp_source != 0)
				g_source_remove(hayes->rd_ppp_source);
			hayes->rd_ppp_source = 0;
			if(hayes->wr_ppp_source != 0)
				g_source_remove(hayes->wr_ppp_source);
			hayes->rd_ppp_source = 0;
			/* reset registration media */
			event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
			free(hayes->registration_media);
			hayes->registration_media = NULL;
			event->registration.media = NULL;
			/* reset modem */
			_hayes_reset(modem);
			break;
	}
	switch(mode)
	{
		case HAYES_MODE_INIT:
		case HAYES_MODE_COMMAND:
			break; /* nothing to do */
		case HAYES_MODE_DATA:
			/* report GPRS registration */
			event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
			free(hayes->registration_media);
			hayes->registration_media = strdup("GPRS");
			event->registration.media = hayes->registration_media;
			hayes->helper->event(hayes->helper->modem, event);
			break;
	}
	hayes->mode = mode;
}


/* messages */
/* hayes_message_to_pdu */
static char * _text_to_data(char const * text, size_t length);
static char * _text_to_sept(char const * text, size_t length);

static char * _hayes_message_to_pdu(ModemPlugin * modem, char const * number,
		ModemMessageEncoding encoding, size_t length,
		char const * content)
{
	Hayes * hayes = modem;
	char * ret;
	char * addr;
	char * data;
	char * p = NULL;
	size_t len;
	char const * smsc = "";
	char const prefix[] = "1100";
	char const pid[] = "00";
	char dcs[] = "0X";
	char const vp[] = "AA";

	if(!_is_number(number))
		return NULL;
	switch(encoding)
	{
		case MODEM_MESSAGE_ENCODING_UTF8:
			/* FIXME really support UTF-8 when necessary */
			p = g_convert(content, length, "ISO-8859-1", "UTF-8",
					NULL, NULL, NULL);
			if(p == NULL)
				return NULL;
			content = p;
			length = strlen(content);
		case MODEM_MESSAGE_ENCODING_ASCII:
			dcs[1] = '0';
			data = _text_to_sept(content, length);
			break;
		case MODEM_MESSAGE_ENCODING_DATA:
			dcs[1] = '4';
			data = _text_to_data(content, length);
			break;
		default:
			return NULL;
	}
	addr = _hayes_convert_number_to_address(number);
	len = 2 + sizeof(prefix) + 2 + strlen((addr != NULL) ? addr : "")
		+ sizeof(pid) + sizeof(dcs) + sizeof(vp) + 2
		+ strlen((data != NULL) ? data : "") + 1;
	ret = malloc(len);
	if(addr != NULL && ret != NULL)
	{
		if(hayes->quirks & HAYES_QUIRK_WANT_SMSC_IN_PDU)
			smsc = "00";
		snprintf(ret, len, "%s%s%02lX%s%s%s%s%02lX%s\x1a",
				smsc, prefix, (unsigned long)strlen(number),
				addr, pid, dcs, vp, (unsigned long)length,
				data);
	}
	free(data);
	free(addr);
	free(p);
	return ret;
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
		buf[(i * 2) + 1] = tab[text[i] & 0x0f];
		buf[i * 2] = tab[((text[i] & 0xf0) >> 4) & 0x0f];
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


/* conversions */
/* hayes_convert_char_to_iso */
static unsigned char _hayes_convert_char_to_iso(unsigned char c)
{
	size_t i;

	for(i = 0; i < sizeof(_hayes_gsm_iso) / sizeof(*_hayes_gsm_iso); i++)
		if(_hayes_gsm_iso[i].gsm == c)
			return _hayes_gsm_iso[i].iso;
	return c;
}


/* hayes_convert_number_to_address */
static char * _hayes_convert_number_to_address(char const * number)
{
	char * ret;
	size_t len;
	size_t i;

	len = 2 + strlen(number) + 2;
	if((ret = malloc(len)) == NULL)
		return NULL;
	snprintf(ret, len, "%02X", (number[0] == '+') ? 145 : 129);
	if(number[0] == '+')
		number++;
	for(i = 2; i < len; i+=2)
	{
		if(number[i - 2] == '\0')
			break;
		ret[i] = number[i - 1];
		ret[i + 1] = number[i - 2];
		if(number[i - 1] == '\0')
		{
			ret[i] = 'F';
			i+=2;
			break;
		}
	}
	ret[i] = '\0';
	return ret;
}


/* parser */
/* hayes_parse */
static int _parse_do(ModemPlugin * modem);

static int _hayes_parse(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	int ret = 0;
	size_t i = 0;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() cnt=%lu\n", __func__,
			(unsigned long)hayes->rd_buf_cnt);
#endif
	while(i < hayes->rd_buf_cnt)
	{
		if(hayes->rd_buf[i++] != '\r' && hayes->rd_buf[i - 1] != '\n')
			continue;
		hayes->rd_buf[i - 1] = '\0';
		if(i < hayes->rd_buf_cnt && hayes->rd_buf[i] == '\n')
			i++;
		if(hayes->rd_buf[0] != '\0')
			ret |= _parse_do(modem);
		hayes->rd_buf_cnt -= i;
		memmove(hayes->rd_buf, &hayes->rd_buf[i], hayes->rd_buf_cnt);
		if((p = realloc(hayes->rd_buf, hayes->rd_buf_cnt)) != NULL)
			hayes->rd_buf = p; /* we can ignore errors... */
		else if(hayes->rd_buf_cnt == 0)
			hayes->rd_buf = NULL; /* ...except when it's not one */
		i = 0;
	}
	return ret;
}

static int _parse_do(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	char const * line = hayes->rd_buf;
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	HayesCommandStatus status;

	if(command == NULL || _hayes_command_get_status(command) != HCS_ACTIVE)
		/* this was most likely unsollicited */
		return _hayes_parse_trigger(modem, line, NULL);
	_hayes_parse_trigger(modem, line, command);
	if(_hayes_command_answer_append(command, line) != 0)
		return -1;
	if((status = _hayes_command_get_status(command)) == HCS_ACTIVE)
		_hayes_command_callback(command);
	/* unqueue if complete */
	if((status = _hayes_command_get_status(command)) == HCS_SUCCESS
			|| status == HCS_ERROR || status == HCS_TIMEOUT)
	{
		_hayes_queue_pop(modem);
		_hayes_queue_push(modem);
	}
	return 0;
}


/* hayes_parse_trigger */
static int _hayes_parse_trigger(ModemPlugin * modem, char const * answer,
		HayesCommand * command)
{
	size_t i;
	size_t count = sizeof(_hayes_code_handlers)
		/ sizeof(*_hayes_code_handlers);
	size_t len;
	HayesCodeHandler * hch;
	char const * p;
	int j;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(modem, \"%s\", command)\n", __func__,
			answer);
#endif
	/* if the handler is obvious return directly */
	for(i = 0; i < count; i++)
	{
		hch = &_hayes_code_handlers[i];
		len = strlen(hch->code);
		if(strncmp(hch->code, answer, len) != 0)
			continue;
		if(answer[len] == ':')
		{
			if(answer[++len] == ' ') /* skip the optional space */
				len++;
		}
		else if(answer[len] != '\0')
			continue;
		hch->callback(modem, &answer[len]);
		return 0;
	}
	/* if the answer has no prefix choose it from the command issued */
	if(command == NULL
			|| (p = _hayes_command_get_attention(command)) == NULL
			|| strncmp(p, "AT", 2) != 0)
		return 0;
	for(i = 0; i < count; i++)
	{
		hch = &_hayes_code_handlers[i];
		len = strlen(hch->code);
		if(strncmp(hch->code, &p[2], len) != 0
				|| isalnum((j = p[2 + len])))
			continue;
		hch->callback(modem, answer);
		return 0;
	}
	return 0;
}


/* queue */
/* hayes_queue_command */
static int _hayes_queue_command(ModemPlugin * modem, HayesCommand * command)
{
	Hayes * hayes = modem;
	GSList * queue;

	switch(hayes->mode)
	{
		case HAYES_MODE_INIT:
			/* ignore commands besides initialization */
			if(_hayes_command_get_priority(command)
					!= HCP_IMMEDIATE)
				return -1;
		case HAYES_MODE_COMMAND:
		case HAYES_MODE_DATA:
			_hayes_command_set_status(command, HCS_QUEUED);
			if(_hayes_command_get_status(command) != HCS_QUEUED)
				return -1;
			queue = hayes->queue;
			hayes->queue = g_slist_append(hayes->queue, command);
			if(queue == NULL)
				_hayes_queue_push(modem);
			break;
	}
	return 0;
}


#if 0 /* XXX no longer used */
/* hayes_queue_command_full */
static int _hayes_queue_command_full(ModemPlugin * modem,
		char const * attention, HayesCommandCallback callback)
{
	HayesCommand * command;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, attention);
#endif
	if((command = _hayes_command_new(attention)) == NULL)
		return -hayes->helper->error(hayes->helper->modem, error_get(),
				1);
	_hayes_command_set_callback(command, callback, modem);
	if(_hayes_queue_command(modem, command) != 0)
	{
		_hayes_command_delete(command);
		return -1;
	}
	return 0;
}
#endif


/* hayes_queue_flush */
static void _hayes_queue_flush(ModemPlugin * modem)
{
	Hayes * hayes = modem;

	g_slist_foreach(hayes->queue_timeout, (GFunc)_hayes_command_delete,
			NULL);
	g_slist_free(hayes->queue_timeout);
	hayes->queue_timeout = NULL;
	g_slist_foreach(hayes->queue, (GFunc)_hayes_command_delete, NULL);
	g_slist_free(hayes->queue);
	hayes->queue = NULL;
	free(hayes->rd_buf);
	hayes->rd_buf = NULL;
	hayes->rd_buf_cnt = 0;
	if(hayes->rd_source != 0)
		g_source_remove(hayes->rd_source);
	hayes->rd_source = 0;
	free(hayes->wr_buf);
	hayes->wr_buf = NULL;
	hayes->wr_buf_cnt = 0;
	if(hayes->wr_source != 0)
		g_source_remove(hayes->wr_source);
	hayes->wr_source = 0;
	if(hayes->rd_ppp_source != 0)
		g_source_remove(hayes->rd_ppp_source);
	hayes->rd_ppp_source = 0;
	if(hayes->wr_ppp_source != 0)
		g_source_remove(hayes->wr_ppp_source);
	hayes->wr_ppp_source = 0;
	if(hayes->source != 0)
		g_source_remove(hayes->source);
	hayes->source = 0;
	if(hayes->timeout != 0)
		g_source_remove(hayes->timeout);
	hayes->timeout = 0;
}


/* hayes_queue_pop */
static int _hayes_queue_pop(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	HayesCommand * command;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(hayes->timeout != 0)
		g_source_remove(hayes->timeout);
	hayes->timeout = 0;
	if(hayes->queue == NULL) /* nothing to send */
		return 0;
	command = hayes->queue->data; /* XXX assumes it's valid */
	_hayes_command_delete(command);
	hayes->queue = g_slist_remove(hayes->queue, command);
	return 0;
}


/* hayes_queue_push */
static int _hayes_queue_push(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	HayesCommand * command;
	char const * prefix = "";
	char const * attention;
	const char suffix[2] = "\r\n";
	size_t size;
	char * p;
	guint timeout;

	if(hayes->queue == NULL) /* nothing to send */
		return 0;
	command = hayes->queue->data;
	if(hayes->mode == HAYES_MODE_DATA)
#if 0 /* FIXME does not seem to work (see ATS2, ATS12) */
		prefix = "+++\r\n";
#else
		return 0; /* XXX keep commands in the queue in DATA mode */
#endif
	_hayes_command_set_status(command, HCS_ACTIVE);
	if(_hayes_command_get_status(command) != HCS_ACTIVE)
		/* no longer push the command */
		return 0;
	attention = _hayes_command_get_attention(command);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() pushing \"%s\"\n", __func__, attention);
#endif
	size = strlen(prefix) + strlen(attention) + sizeof(suffix);
	if((p = realloc(hayes->wr_buf, hayes->wr_buf_cnt + size)) == NULL)
		return -hayes->helper->error(hayes->helper->modem, strerror(
					errno), 1);
	hayes->wr_buf = p;
	snprintf(&hayes->wr_buf[hayes->wr_buf_cnt], size, "%s%s%s", prefix,
			attention, suffix);
	hayes->wr_buf_cnt += size;
	if(hayes->channel != NULL && hayes->wr_source == 0)
		hayes->wr_source = g_io_add_watch(hayes->channel, G_IO_OUT,
				_on_watch_can_write, modem);
	if(hayes->timeout != 0)
		g_source_remove(hayes->timeout);
	hayes->timeout = 0;
	if((timeout = _hayes_command_get_timeout(command)) != 0)
		hayes->timeout = g_timeout_add(timeout, _on_timeout, modem);
	return 0;
}


/* hayes_reset */
static void _hayes_reset(ModemPlugin * modem)
{
	Hayes * hayes = modem;

	_hayes_reset_stop(modem);
	_hayes_reset_start(modem, hayes->retry);
}


/* hayes_request_type */
static int _hayes_request_type(ModemPlugin * modem, ModemRequestType type)
{
	ModemRequest request;

	memset(&request, 0, sizeof(request));
	request.type = type;
	return _hayes_request(modem, &request);
}


/* hayes_reset_start */
static void _hayes_reset_start(ModemPlugin * modem, unsigned int retry)
{
	Hayes * hayes = modem;

	hayes->retry = retry;
	hayes->source = g_idle_add(_on_reset, modem);
}


/* hayes_reset_stop */
static void _reset_stop_channel(GIOChannel * channel);
static void _reset_stop_string(char ** string);

static void _hayes_reset_stop(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	ModemEvent * event;
	size_t i;

	/* close everything opened */
	if(hayes->fp != NULL)
		fclose(hayes->fp);
	hayes->fp = NULL;
	_hayes_queue_flush(modem);
	_reset_stop_channel(hayes->channel);
	hayes->channel = NULL;
	_reset_stop_channel(hayes->rd_ppp_channel);
	hayes->rd_ppp_channel = NULL;
	_reset_stop_channel(hayes->wr_ppp_channel);
	hayes->wr_ppp_channel = NULL;
	/* report disconnection if already connected */
	event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];
	if(event->connection.connected)
	{
		event->connection.connected = 0;
		event->connection.in = 0;
		event->connection.out = 0;
		hayes->helper->event(hayes->helper->modem, event);
	}
	/* reset battery information */
	event = &hayes->events[MODEM_EVENT_TYPE_BATTERY_LEVEL];
	if(event->battery_level.status != MODEM_BATTERY_STATUS_UNKNOWN)
	{
		event->battery_level.status = MODEM_BATTERY_STATUS_UNKNOWN;
		event->battery_level.level = 0.0 / 0.0;
		event->battery_level.charging = 0;
		hayes->helper->event(hayes->helper->modem, event);
	}
	/* remove internal data */
	_reset_stop_string(&hayes->authentication_name);
	_reset_stop_string(&hayes->call_number);
	_reset_stop_string(&hayes->contact_name);
	_reset_stop_string(&hayes->contact_number);
	_reset_stop_string(&hayes->gprs_username);
	_reset_stop_string(&hayes->gprs_password);
	_reset_stop_string(&hayes->message_number);
	_reset_stop_string(&hayes->model_name);
	_reset_stop_string(&hayes->model_vendor);
	_reset_stop_string(&hayes->model_version);
	_reset_stop_string(&hayes->registration_media);
	_reset_stop_string(&hayes->registration_operator);
	/* reset events */
	memset(&hayes->events, 0, sizeof(hayes->events));
	for(i = 0; i < sizeof(hayes->events) / sizeof(*hayes->events); i++)
		hayes->events[i].type = i;
	/* reset mode */
	hayes->mode = HAYES_MODE_INIT;
}

static void _reset_stop_channel(GIOChannel * channel)
{
	GError * error = NULL;

	if(channel == NULL)
		return;
	/* XXX should the file descriptor also be closed? */
	if(g_io_channel_shutdown(channel, TRUE, &error) == G_IO_STATUS_ERROR)
		/* XXX report error */
		g_error_free(error);
	g_io_channel_unref(channel);
}

static void _reset_stop_string(char ** string)
{
	free(*string);
	*string = NULL;
}


/* commands */
/* hayes_command_new */
static HayesCommand * _hayes_command_new(char const * attention)
{
	HayesCommand * command;

	if((command = object_new(sizeof(*command))) == NULL)
		return NULL;
	command->priority = HCP_NORMAL;
	command->status = HCS_PENDING;
	command->attention = string_new(attention);
	command->timeout = 30000;
	command->callback = NULL;
	command->priv = NULL;
	command->answer = NULL;
	command->data = NULL;
	if(command->attention == NULL)
	{
		_hayes_command_delete(command);
		return NULL;
	}
	return command;
}


/* hayes_command_delete */
static void _hayes_command_delete(HayesCommand * command)
{
	string_delete(command->attention);
	string_delete(command->answer);
	object_delete(command);
}


/* hayes_command_get_answer */
static char const * _hayes_command_get_answer(HayesCommand * command)
{
	return command->answer;
}


/* hayes_command_get_attention */
static char const * _hayes_command_get_attention(HayesCommand * command)
{
	return command->attention;
}


/* hayes_command_get_data */
static void * _hayes_command_get_data(HayesCommand * command)
{
	return command->data;
}


#if 0 /* XXX no longer being used */
/* hayes_command_get_line */
static char * _hayes_command_get_line(HayesCommand * command,
		char const * prefix)
{
	/* FIXME also return the other lines matching */
	char * ret;
	char const * answer = command->answer;
	size_t len;
	char * p;

	if(prefix == NULL)
		return NULL;
	len = strlen(prefix);
	while(answer != NULL)
		if(strncmp(answer, prefix, len) == 0 && strncmp(&answer[len],
					": ", 2) == 0)
		{
			if((ret = string_new(&answer[len + 2])) != NULL
					&& (p = strchr(ret, '\n')) != NULL)
				*p = '\0';
			return ret;
		}
		else if((answer = strchr(answer, '\n')) != NULL)
			answer++;
	return NULL;
}
#endif


/* hayes_command_get_priority */
static HayesCommandPriority _hayes_command_get_priority(HayesCommand * command)
{
	return command->priority;
}


/* hayes_command_get_status */
static HayesCommandStatus _hayes_command_get_status(HayesCommand * command)
{
	return command->status;
}


/* hayes_command_get_timeout */
static unsigned int _hayes_command_get_timeout(HayesCommand * command)
{
	return command->timeout;
}


/* hayes_command_set_callback */
static void _hayes_command_set_callback(HayesCommand * command,
		HayesCommandCallback callback, void * priv)
{
	command->callback = callback;
	command->priv = priv;
}


/* hayes_command_set_id */
static void _hayes_command_set_data(HayesCommand * command, void * data)
{
	command->data = data;
}


/* hayes_command_set_priority */
static void _hayes_command_set_priority(HayesCommand * command,
		HayesCommandPriority priority)
{
	command->priority = priority;
}


/* hayes_command_set_status */
static void _hayes_command_set_status(HayesCommand * command,
		HayesCommandStatus status)
{
	command->status = status;
	_hayes_command_callback(command);
}


/* hayes_command_set_timeout */
static void _hayes_command_set_timeout(HayesCommand * command,
		unsigned int timeout)
{
	command->timeout = timeout;
}


/* hayes_command_answer_append */
static int _hayes_command_answer_append(HayesCommand * command,
		char const * answer)
{
	char * p;

	if(answer == NULL)
		return 0;
	if(command->answer == NULL)
		p = string_new(answer);
	else
		p = string_new_append(command->answer, "\n", answer, NULL);
	if(p == NULL)
		return -1;
	free(command->answer);
	command->answer = p;
	return 0;
}


/* hayes_command_callback */
static HayesCommandStatus _hayes_command_callback(HayesCommand * command)
{
	if(command->callback != NULL)
		command->status = command->callback(command, command->status,
				command->priv);
	return command->status;
}


/* callbacks */
/* on_queue_timeout */
static gboolean _on_queue_timeout(gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	HayesCommand * command;

	hayes->source = 0;
	if(hayes->queue_timeout == NULL) /* nothing to send */
		return FALSE;
	command = hayes->queue_timeout->data;
	_hayes_queue_command(modem, command);
	hayes->queue_timeout = g_slist_remove(hayes->queue_timeout, command);
	if(hayes->queue_timeout != NULL)
		hayes->source = g_timeout_add(1000, _on_queue_timeout, modem);
	else
		/* XXX check the registration again to be safe */
		_hayes_request_type(modem, HAYES_REQUEST_REGISTRATION);
	return FALSE;
}


/* on_reset */
static int _reset_open(ModemPlugin * modem);
static int _reset_configure(ModemPlugin * modem, char const * device, int fd);
static unsigned int _reset_configure_baudrate(ModemPlugin * modem,
		unsigned int baudrate);
static gboolean _reset_settle(gpointer data);
static HayesCommandStatus _on_reset_callback(HayesCommand * command,
		HayesCommandStatus status, void * priv);

static gboolean _on_reset(gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_STATUS];
	GError * error = NULL;
	int fd;
	char const * logfile;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_hayes_reset_stop(modem);
	if((fd = _reset_open(modem)) < 0)
	{
		if(event->status.status != MODEM_STATUS_UNAVAILABLE)
		{
			event->status.status = MODEM_STATUS_UNAVAILABLE;
			hayes->helper->event(hayes->helper->modem, event);
		}
		hayes->helper->error(NULL, error_get(), 1);
		if(hayes->retry > 0)
			hayes->source = g_timeout_add(hayes->retry, _on_reset,
					modem);
		return FALSE;
	}
	event->status.status = MODEM_STATUS_UNKNOWN;
	/* logging */
	logfile = helper->config_get(helper->modem, "logfile");
	if(logfile != NULL && (hayes->fp = fopen(logfile, "w")) == NULL)
		hayes->helper->error(NULL, strerror(errno), 1);
	else if(hayes->fp != NULL)
		setvbuf(hayes->fp, NULL, _IONBF, BUFSIZ);
	hayes->channel = g_io_channel_unix_new(fd);
	if(g_io_channel_set_encoding(hayes->channel, NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		hayes->helper->error(hayes->helper->modem, error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(hayes->channel, FALSE);
	hayes->rd_source = g_io_add_watch(hayes->channel, G_IO_IN,
			_on_watch_can_read, modem);
	_reset_settle(modem);
	return FALSE;
}

static int _reset_open(ModemPlugin * modem)
{
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	char const * device;
	int fd;

	if((device = helper->config_get(helper->modem, "device")) == NULL)
		device = "/dev/modem";
	if((fd = open(device, O_RDWR | O_NONBLOCK)) < 0)
		return -error_set_code(1, "%s: %s", device, strerror(errno));
	if(_reset_configure(modem, device, fd) != 0)
	{
		close(fd);
		return -1;
	}
	return fd;
}

static int _reset_configure(ModemPlugin * modem, char const * device, int fd)
{
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	unsigned int baudrate;
	unsigned int hwflow;
	struct stat st;
	int fl;
	struct termios term;
	char const * p;

	/* baud rate */
	if((p = helper->config_get(helper->modem, "baudrate")) == 0
			|| strtoul(p, NULL, 10) == 0)
		baudrate = 115200;
	baudrate = _reset_configure_baudrate(modem, baudrate);
	/* hardware flow */
	if((p = helper->config_get(helper->modem, "hwflow")) == 0
			|| strtoul(p, NULL, 10) == 0)
		hwflow = 1;
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
		if(hwflow != 0)
			term.c_cflag |= CRTSCTS;
		else
			term.c_cflag &= ~CRTSCTS;
		term.c_iflag = (IGNPAR | IGNBRK);
		term.c_lflag = 0;
		term.c_oflag = 0;
		term.c_cc[VMIN] = 1;
		term.c_cc[VTIME] = 0;
		if(cfsetispeed(&term, 0) != 0) /* same speed as output speed */
			error_set("%s", device); /* go on anyway */
		if(cfsetospeed(&term, baudrate) != 0)
			error_set("%s", device); /* go on anyway */
		if(tcsetattr(fd, TCSAFLUSH, &term) != 0)
			return 1;
	}
	return 0;
}

static unsigned int _reset_configure_baudrate(ModemPlugin * modem,
		unsigned int baudrate)
{
	Hayes * hayes = modem;

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
		case 230400:
			return B230400;
#ifdef B460800
		case 460800:
			return B460800;
#endif
#ifdef B921600
		case 921600:
			return B921600;
#endif
		default:
			error_set("%u%s", baudrate,
					"Unsupported baudrate (using 115200)");
			hayes->helper->error(NULL, error_get(), 1);
			return B115200;
	}
}

static gboolean _reset_settle(gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	HayesCommand * command;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((command = _hayes_command_new("ATZE0V1")) == NULL)
	{
		hayes->helper->error(hayes->helper->modem, error_get(), 1);
		return FALSE;
	}
	_hayes_command_set_callback(command, _on_reset_callback, modem);
	_hayes_command_set_priority(command, HCP_IMMEDIATE);
	_hayes_command_set_timeout(command, 500);
	if(_hayes_queue_command(modem, command) != 0)
	{
		hayes->helper->error(hayes->helper->modem, error_get(), 1);
		_hayes_command_delete(command);
	}
	return FALSE;
}

static HayesCommandStatus _on_reset_callback(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, status);
#endif
	status = _on_request_generic(command, status, priv);
	switch(status)
	{
		case HCS_PENDING: /* ignore */
		case HCS_QUEUED:
			break;
		case HCS_ACTIVE: /* give it another chance */
			break;
		case HCS_SUCCESS: /* we can initialize */
			_hayes_set_mode(modem, HAYES_MODE_COMMAND);
			_hayes_request_type(modem,
					HAYES_REQUEST_LOCAL_ECHO_DISABLE);
			_hayes_request_type(modem,
					HAYES_REQUEST_VERBOSE_ENABLE);
			_hayes_request_type(modem, HAYES_REQUEST_MODEL);
			_hayes_request_type(modem,
					HAYES_REQUEST_EXTENDED_ERRORS);
			_hayes_request_type(modem, HAYES_REQUEST_FUNCTIONAL);
			break;
		case HCS_TIMEOUT: /* try again */
		case HCS_ERROR:
			_reset_settle(modem);
			break;
	}
	return status;
}


/* on_timeout */
static gboolean _on_timeout(gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	HayesCommand * command;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	hayes->timeout = 0;
	if(hayes->queue == NULL || (command = hayes->queue->data) == NULL)
		return FALSE;
	_hayes_command_set_status(command, HCS_TIMEOUT);
	_hayes_queue_pop(modem);
	_hayes_queue_push(modem);
	return FALSE;
}


/* on_watch_can_read */
static gboolean _on_watch_can_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_IN || source != hayes->channel)
		return FALSE; /* should not happen */
	if((p = realloc(hayes->rd_buf, hayes->rd_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	hayes->rd_buf = p;
	status = g_io_channel_read_chars(source,
			&hayes->rd_buf[hayes->rd_buf_cnt], 256, &cnt, &error);
	/* logging */
	if(hayes->fp != NULL && (fputs("\nMODEM: ", hayes->fp) == EOF
				|| fwrite(&hayes->rd_buf[hayes->rd_buf_cnt],
					sizeof(*p), cnt, hayes->fp) < cnt))
	{
		helper->error(NULL, strerror(errno), 1);
		fclose(hayes->fp);
		hayes->fp = NULL;
	}
	hayes->rd_buf_cnt += cnt;
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->modem, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default: /* should not happen... */
			if(hayes->retry > 0)
				_hayes_reset(modem);
			hayes->rd_source = 0;
			return FALSE;
	}
	switch(hayes->mode)
	{
		case HAYES_MODE_INIT:
		case HAYES_MODE_COMMAND:
			_hayes_parse(modem);
			break;
		case HAYES_MODE_DATA:
			if(hayes->wr_ppp_channel == NULL
					|| hayes->wr_ppp_source != 0)
				break;
			hayes->wr_ppp_source = g_io_add_watch(
					hayes->wr_ppp_channel, G_IO_OUT,
					_on_watch_can_write_ppp, modem);
			break;
	}
	return TRUE;
}


/* on_watch_can_read_ppp */
static gboolean _on_watch_can_read_ppp(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_IN || source != hayes->rd_ppp_channel)
		return FALSE; /* should not happen */
	if((p = realloc(hayes->wr_buf, hayes->wr_buf_cnt + 256)) == NULL)
		return TRUE; /* XXX retries immediately (delay?) */
	hayes->wr_buf = p;
	status = g_io_channel_read_chars(source,
			&hayes->wr_buf[hayes->wr_buf_cnt], 256, &cnt, &error);
	hayes->wr_buf_cnt += cnt;
	event->connection.out += cnt;
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->modem, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default:
			hayes->rd_ppp_source = 0;
			event->connection.connected = 0;
			helper->event(helper->modem, event);
			_hayes_set_mode(modem, HAYES_MODE_INIT);
			return FALSE;
	}
	if(hayes->channel != NULL && hayes->wr_source == 0)
		hayes->wr_source = g_io_add_watch(hayes->channel, G_IO_OUT,
				_on_watch_can_write, modem);
	return TRUE;
}


/* on_watch_can_write */
static gboolean _on_watch_can_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_OUT || source != hayes->channel)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, hayes->wr_buf,
			hayes->wr_buf_cnt, &cnt, &error);
	/* logging */
	if(hayes->fp != NULL && (fputs("\nPHONE: ", hayes->fp) == EOF
				|| fwrite(hayes->wr_buf, sizeof(*p), cnt,
					hayes->fp) < cnt))
	{
		hayes->helper->error(NULL, strerror(errno), 1);
		fclose(hayes->fp);
		hayes->fp = NULL;
	}
	if(cnt != 0) /* some data may have been written anyway */
	{
		hayes->wr_buf_cnt -= cnt;
		memmove(hayes->wr_buf, &hayes->wr_buf[cnt], hayes->wr_buf_cnt);
		if((p = realloc(hayes->wr_buf, hayes->wr_buf_cnt)) != NULL)
			hayes->wr_buf = p; /* we can ignore errors... */
		else if(hayes->wr_buf_cnt == 0)
			hayes->wr_buf = NULL; /* ...except when it's not one */
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->modem, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default: /* should not happen */
			hayes->wr_source = 0;
			if(hayes->retry > 0)
				_hayes_reset(modem);
			return FALSE;
	}
	if(hayes->wr_buf_cnt > 0) /* there is more data to write */
		return TRUE;
	hayes->wr_source = 0;
	return FALSE;
}


/* on_watch_can_write_ppp */
static gboolean _on_watch_can_write_ppp(GIOChannel * source,
		GIOCondition condition, gpointer data)
{
	ModemPlugin * modem = data;
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];
	gsize cnt = 0;
	GError * error = NULL;
	GIOStatus status;
	char * p;

	if(condition != G_IO_OUT || source != hayes->wr_ppp_channel)
		return FALSE; /* should not happen */
	status = g_io_channel_write_chars(source, hayes->rd_buf,
			hayes->rd_buf_cnt, &cnt, &error);
	event->connection.in += cnt;
	if(cnt != 0) /* some data may have been written anyway */
	{
		hayes->rd_buf_cnt -= cnt;
		memmove(hayes->rd_buf, &hayes->rd_buf[cnt], hayes->rd_buf_cnt);
		if((p = realloc(hayes->rd_buf, hayes->rd_buf_cnt)) != NULL)
			hayes->rd_buf = p; /* we can ignore errors... */
		else if(hayes->rd_buf_cnt == 0)
			hayes->rd_buf = NULL; /* ...except when it's not one */
	}
	switch(status)
	{
		case G_IO_STATUS_NORMAL:
			break;
		case G_IO_STATUS_ERROR:
			helper->error(helper->modem, error->message, 1);
			g_error_free(error);
		case G_IO_STATUS_EOF:
		default:
			hayes->wr_ppp_source = 0;
			event->connection.connected = 0;
			helper->event(helper->modem, event);
			_hayes_set_mode(modem, HAYES_MODE_INIT);
			return FALSE;
	}
	if(hayes->rd_buf_cnt > 0) /* there is more data to write */
		return TRUE;
	hayes->wr_ppp_source = 0;
	return FALSE;
}


/* on_request_authenticate */
static HayesCommandStatus _on_request_authenticate(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_AUTHENTICATION];

	switch((status = _on_request_generic(command, status, priv)))
	{
		case HCS_SUCCESS:
			break;
		case HCS_ERROR:
			event->authentication.status
				= MODEM_AUTHENTICATION_STATUS_ERROR;
			hayes->helper->event(hayes->helper->modem, event);
		default:
			return status;
	}
	/* XXX it should be bound to the request instead */
	if(event->authentication.name != NULL && (strcmp("SIM PIN",
					event->authentication.name) == 0
				|| strcmp("SIM PUK",
					event->authentication.name) == 0))
		/* verify that it really worked */
		_hayes_request_type(modem, HAYES_REQUEST_SIM_PIN_VALID);
	else
	{
		event->authentication.status = MODEM_AUTHENTICATION_STATUS_OK;
		hayes->helper->event(hayes->helper->modem, event);
	}
	return status;
}


/* on_request_battery_level */
static HayesCommandStatus _on_request_battery_level(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_BATTERY_LEVEL];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_call */
static HayesCommandStatus _on_request_call(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_call_incoming */
static HayesCommandStatus _on_request_call_incoming(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS
			&& status != HCS_ERROR)
		return status;
	event->call.direction = MODEM_CALL_DIRECTION_INCOMING;
	event->call.status = (status == HCS_SUCCESS)
		? MODEM_CALL_STATUS_ACTIVE : MODEM_CALL_STATUS_NONE;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_call_outgoing */
static HayesCommandStatus _on_request_call_outgoing(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS
			&& status != HCS_ERROR)
		return status;
	event->call.direction = MODEM_CALL_DIRECTION_OUTGOING;
	event->call.status = (status == HCS_SUCCESS)
		? MODEM_CALL_STATUS_ACTIVE : MODEM_CALL_STATUS_NONE;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_call_status */
static HayesCommandStatus _on_request_call_status(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS
			&& status != HCS_ERROR)
		return status;
	_hayes_request_type(modem, HAYES_REQUEST_PHONE_ACTIVE);
	return status;
}


/* on_request_contact_delete */
static HayesCommandStatus _on_request_contact_delete(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONTACT_DELETED];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_functional */
static HayesCommandStatus _on_request_functional(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

	switch((status = _on_request_generic(command, status, priv)))
	{
		case HCS_ERROR:
			/* try to enable */
			_hayes_request_type(modem,
					HAYES_REQUEST_FUNCTIONAL_ENABLE);
			break;
		default:
			break;
	}
	return status;
}


/* on_request_functional_enable */
static HayesCommandStatus _on_request_functional_enable(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

	switch((status = _on_request_generic(command, status, priv)))
	{
		case HCS_ERROR:
#if 0 /* XXX ignore for now (may simply be missing the PIN code) */
			/* force a reset */
			_hayes_request_type(modem,
					HAYES_REQUEST_FUNCTIONAL_ENABLE_RESET);
#endif
			break;
		case HCS_SUCCESS:
			_on_code_cfun(modem, "1"); /* XXX ugly workaround */
			break;
		case HCS_TIMEOUT:
			/* repeat request */
			_hayes_request_type(modem,
					HAYES_REQUEST_FUNCTIONAL_ENABLE);
			break;
		default:
			break;
	}
	return status;
}


/* on_request_functional_enable_reset */
static HayesCommandStatus _on_request_functional_enable_reset(
		HayesCommand * command, HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

	switch((status = _on_request_generic(command, status, priv)))
	{
		case HCS_SUCCESS:
			_on_code_cfun(modem, "1"); /* XXX ugly workaround */
			break;
		case HCS_TIMEOUT:
			/* repeat request */
			_hayes_request_type(modem,
					HAYES_REQUEST_FUNCTIONAL_ENABLE);
			break;
		default:
			break;
	}
	return status;
}


/* on_request_generic */
static HayesCommandStatus _on_request_generic(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	char const * answer;
	char const * p;

	if(status != HCS_ACTIVE)
		return status;
	if((answer = _hayes_command_get_answer(command)) == NULL)
		return status;
	/* look for the last line */
	while((p = strchr(answer, '\n')) != NULL)
		answer = ++p;
	if(strcmp(answer, "OK") == 0)
		return HCS_SUCCESS;
	else if(strcmp(answer, "ERROR") == 0)
		return HCS_ERROR;
	return status;
}


/* on_request_message */
static HayesCommandStatus _on_request_message(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	HayesRequestMessageData * data;

	if((status = _on_request_generic(command, status, priv)) == HCS_SUCCESS
			|| status == HCS_ERROR || status == HCS_TIMEOUT)
		if((data = _hayes_command_get_data(command)) != NULL)
		{
			free(data);
			_hayes_command_set_data(command, NULL);
		}
	return status;
}


/* on_request_message_delete */
static HayesCommandStatus _on_request_message_delete(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MESSAGE_DELETED];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_message_list */
static HayesCommandStatus _on_request_message_list(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	HayesRequestMessageData * data;

	if((status = _on_request_generic(command, status, priv)) == HCS_SUCCESS
			|| status == HCS_ERROR || status == HCS_TIMEOUT)
		if((data = _hayes_command_get_data(command)) != NULL)
		{
			free(data);
			_hayes_command_set_data(command, NULL);
		}
	return status;
}


/* on_request_model */
static HayesCommandStatus _on_request_model(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MODEL];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	return status;
}


/* on_request_registration */
static HayesCommandStatus _on_request_registration(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	/* force a registration status */
	_hayes_request_type(modem, HAYES_REQUEST_REGISTRATION);
	return status;
}


/* on_request_registration_automatic */
static HayesCommandStatus _on_request_registration_automatic(
		HayesCommand * command, HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];

	status = _on_request_generic(command, status, priv);
	switch(status)
	{
		case HCS_PENDING:
		case HCS_QUEUED:
			break;
		case HCS_ACTIVE:
			event->registration.mode
				= MODEM_REGISTRATION_MODE_AUTOMATIC;
			event->registration.status
				= MODEM_REGISTRATION_STATUS_SEARCHING;
			hayes->helper->event(hayes->helper->modem, event);
			break;
		case HCS_ERROR:
		case HCS_TIMEOUT:
			event->registration.mode
				= MODEM_REGISTRATION_MODE_UNKNOWN;
			event->registration.status
				= MODEM_REGISTRATION_STATUS_UNKNOWN;
			hayes->helper->event(hayes->helper->modem, event);
			break;
		case HCS_SUCCESS:
			/* force a registration status */
			_hayes_request_type(modem, HAYES_REQUEST_REGISTRATION);
			break;
	}
	return status;
}


/* on_request_registration_disabled */
static HayesCommandStatus _on_request_registration_disabled(
		HayesCommand * command, HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];

	if((status = _on_request_generic(command, status, priv)) != HCS_SUCCESS)
		return status;
	event->registration.mode = MODEM_REGISTRATION_MODE_DISABLED;
	/* force a registration status */
	_hayes_request_type(modem, HAYES_REQUEST_REGISTRATION);
	return status;
}


/* on_request_sim_pin_valid */
static HayesCommandStatus _on_request_sim_pin_valid(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	ModemPlugin * modem = priv;
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_AUTHENTICATION];
	ModemRequest request;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, status);
#endif
	if((status = _on_request_generic(command, status, priv)) == HCS_ERROR
			|| status == HCS_TIMEOUT)
	{
		event->authentication.status
			= MODEM_AUTHENTICATION_STATUS_ERROR;
		hayes->helper->event(hayes->helper->modem, event);
		return status;
	}
	else if(status != HCS_SUCCESS)
		return status;
	hayes->helper->event(hayes->helper->modem, event);
	/* return if not successful */
	if(event->authentication.status != MODEM_AUTHENTICATION_STATUS_OK)
		return status;
	/* apply useful settings */
	_hayes_request_type(modem, HAYES_REQUEST_EXTENDED_ERRORS);
	_hayes_request_type(modem, HAYES_REQUEST_EXTENDED_RING_REPORTS);
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL_PRESENTATION;
	request.call_presentation.enabled = 1;
	_hayes_request(modem, &request);
	_hayes_request_type(modem,
			HAYES_REQUEST_CALL_WAITING_UNSOLLICITED_ENABLE);
	_hayes_request_type(modem, HAYES_REQUEST_CONNECTED_LINE_ENABLE);
	/* report new messages */
	_hayes_request_type(modem, HAYES_REQUEST_MESSAGE_UNSOLLICITED_ENABLE);
	/* report new notifications */
	_hayes_request_type(modem,
			HAYES_REQUEST_SUPPLEMENTARY_SERVICE_DATA_ENABLE);
	/* refresh the registration status */
	_hayes_request_type(modem,
			HAYES_REQUEST_REGISTRATION_UNSOLLICITED_ENABLE);
	/* refresh the current call status */
	_hayes_trigger(modem, MODEM_EVENT_TYPE_CALL);
	/* refresh the contact list */
	_hayes_request_type(modem, MODEM_REQUEST_CONTACT_LIST);
	/* refresh the message list */
	_hayes_request_type(modem, MODEM_REQUEST_MESSAGE_LIST);
	return status;
}


/* on_request_unsupported */
static HayesCommandStatus _on_request_unsupported(HayesCommand * command,
		HayesCommandStatus status, void * priv)
{
	/* FIXME report an unsupported event with the result of the command */
	return _on_request_generic(command, status, priv);
}


/* on_code_call_error */
static void _on_code_call_error(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;

	if(command != NULL)
		_hayes_command_set_status(command, HCS_ERROR);
	_hayes_request_type(modem, HAYES_REQUEST_PHONE_ACTIVE);
}


/* on_code_cbc */
static void _on_code_cbc(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_BATTERY_LEVEL];
	int res;
	unsigned int u;
	unsigned int v;
	double f;

	if((res = sscanf(answer, "%u,%u", &u, &v)) != 2)
		return;
	event->battery_level.status = MODEM_BATTERY_STATUS_UNKNOWN;
	event->battery_level.charging = 0;
	if(u == 0)
		u = MODEM_BATTERY_STATUS_CONNECTED;
	else if(u == 1)
		u = MODEM_BATTERY_STATUS_CHARGING;
	else if(u == 2)
		u = MODEM_BATTERY_STATUS_NONE;
	else if(u == 3)
		u = MODEM_BATTERY_STATUS_ERROR;
	else
		u = MODEM_BATTERY_STATUS_UNKNOWN;
	switch((event->battery_level.status = u))
	{
		case MODEM_BATTERY_STATUS_CHARGING:
			event->battery_level.charging = 1;
		case MODEM_BATTERY_STATUS_CONNECTED:
			f = v;
			if(hayes->quirks & HAYES_QUIRK_BATTERY_70)
				f /= 70.0;
			else
				f /= 100.0;
			f = max(f, 0.0);
			event->battery_level.level = min(f, 1.0);
			break;
		default:
			event->battery_level.level = 0.0 / 0.0;
			break;
	}
}


/* on_code_cfun */
static void _on_code_cfun(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_STATUS];
	unsigned int u;

	if(sscanf(answer, "%u", &u) != 1)
		return;
	switch(u)
	{
		case 1:
			/* report being online */
			event = &hayes->events[MODEM_EVENT_TYPE_STATUS];
			event->status.status = MODEM_STATUS_ONLINE;
			hayes->helper->event(hayes->helper->modem, event);
			break;
		case 4: /* antennas disabled */
		case 0: /* telephony disabled */
		default:
			/* FIXME this is maybe not the right event type */
			event->status.status = MODEM_STATUS_OFFLINE;
			hayes->helper->event(hayes->helper->modem, event);
			break;
	}
}


/* on_code_cgatt */
static void _on_code_cgatt(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
	unsigned int u;

	if(sscanf(answer, "%u", &u) != 1)
		return;
	free(hayes->registration_media);
	if(u == 1)
		hayes->registration_media = strdup("GPRS");
	else
		hayes->registration_media = NULL;
	event->registration.media = hayes->registration_media;
	/* this is usually worth an event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cgmi */
static void _on_code_cgmi(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MODEL];
	char * p;

	if((p = strdup(answer)) == NULL)
		return; /* XXX report error? */
	free(hayes->model_vendor);
	hayes->model_vendor = p;
	event->model.vendor = p;
}


/* on_code_cgmm */
static void _on_code_cgmm(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MODEL];
	char * p;
	size_t i;

	if((p = strdup(answer)) == NULL)
		return; /* XXX report error? */
	free(hayes->model_name);
	hayes->model_name = p;
	event->model.name = p;
	/* determine known quirks */
	for(i = 0; _hayes_quirks[i].model != NULL; i++)
		if(strcmp(_hayes_quirks[i].model, p) == 0)
		{
			hayes->quirks = _hayes_quirks[i].quirks;
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s() quirks=%u\n", __func__,
					hayes->quirks);
#endif
			break;
		}
}


/* on_code_cgmr */
static void _on_code_cgmr(ModemPlugin * modem, char const * answer)
	/* FIXME the output may be multi-line */
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MODEL];
	char * p;

	if((p = strdup(answer)) == NULL)
		return; /* XXX report error? */
	free(hayes->model_version);
	hayes->model_version = p;
	event->model.version = p;
}


/* on_code_clip */
static void _on_code_clip(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];
	char buf[32];
	unsigned int u;

	if(sscanf(answer, "\"%31[^\"]\",%u", buf, &u) != 2)
		return;
	buf[sizeof(buf) - 1] = '\0';
	free(hayes->call_number);
	switch(u)
	{
		case 145:
			if((hayes->call_number = malloc(sizeof(buf) + 1))
					== NULL)
				break;
			snprintf(hayes->call_number, sizeof(buf) + 1, "%s%s",
					"+", buf);
			break;
		default:
			hayes->call_number = strdup(buf);
			break;
	}
	/* this is always an unsollicited event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cme_error */
static void _on_code_cme_error(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	/* XXX ugly */
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	unsigned int u;
	HayesCommand * p;
	ModemEvent * event;

	if(command != NULL)
		_hayes_command_set_status(command, HCS_ERROR);
	if(sscanf(answer, "%u", &u) != 1)
		return;
	switch(u)
	{
		case 11: /* SIM PIN required */
			_on_code_cpin(modem, "SIM PIN");
			_hayes_trigger(modem, MODEM_EVENT_TYPE_AUTHENTICATION);
			break;
		case 12: /* SIM PUK required */
			_on_code_cpin(modem, "SIM PUK");
			_hayes_trigger(modem, MODEM_EVENT_TYPE_AUTHENTICATION);
			break;
		case 100: /* unknown error */
			if((hayes->quirks & HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR)
					== 0)
				break;
		case 14: /* SIM busy */
			/* repeat the command */
			if(command == NULL)
				break;
			if((p = _hayes_command_new(command->attention)) == NULL)
				break;
			_hayes_command_set_callback(p, command->callback,
					command->priv);
			_hayes_command_set_data(p, command->data);
			_hayes_command_set_data(command, NULL);
			hayes->queue_timeout = g_slist_append(
					hayes->queue_timeout, p);
			if(hayes->source == 0)
				hayes->source = g_timeout_add(5000,
						_on_queue_timeout, modem);
			break;
		case 32: /* emergency calls only */
			event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
			free(hayes->registration_media);
			hayes->registration_media = NULL;
			event->registration.media = NULL;
			free(hayes->registration_operator);
			hayes->registration_operator = strdup("SOS");
			event->registration._operator
				= hayes->registration_operator;
			event->registration.status
				= MODEM_REGISTRATION_STATUS_REGISTERED;
			helper->event(helper->modem, event);
			/* verify the SIM card */
			_hayes_request_type(modem, HAYES_REQUEST_SIM_PIN_VALID);
			break;
		default: /* FIXME implement the rest */
		case 3:  /* operation not allowed */
		case 4:  /* operation not supported */
		case 16: /* Incorrect SIM PIN/PUK */
		case 20: /* Memory full */
			break;
	}
}


/* on_code_cmgl */
static void _on_code_cmgl(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	/* XXX ugly */
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	ModemRequest request;
	unsigned int id;
	unsigned int u;
	HayesRequestMessageData * data;
	ModemMessageFolder folder = MODEM_MESSAGE_FOLDER_UNKNOWN;
	ModemMessageStatus status = MODEM_MESSAGE_STATUS_READ;

	/* XXX we could already be reading the message at this point */
	if(sscanf(answer, "%u,%u,%u,%u", &id, &u, &u, &u) != 4
			&& sscanf(answer, "%u,%u,,%u", &id, &u, &u) != 3)
		/* XXX we may be stuck in PDU mode at this point */
		return;
	request.type = MODEM_REQUEST_MESSAGE;
	request.message.id = id;
	if(command != NULL && (data = _hayes_command_get_data(command)) != NULL)
	{
		folder = data->folder;
		status = data->status;
	}
	if((data = malloc(sizeof(*data))) != NULL)
	{
		data->id = id;
		data->folder = folder;
		data->status = status;
	}
	if(_request_do(modem, &request, data) != 0)
		free(data);
}


/* on_code_cmgr */
static char * _cmgr_pdu_parse(char const * pdu, time_t * timestamp,
		char * number, ModemMessageEncoding * encoding,
		size_t * length);
static char * _cmgr_pdu_parse_encoding_data(char const * pdu, size_t len,
		size_t i, size_t hdr, ModemMessageEncoding * encoding,
		size_t * length);
static char * _cmgr_pdu_parse_encoding_default(char const * pdu, size_t len,
                size_t i, size_t hdr, ModemMessageEncoding * encoding,
		size_t * length);
static void _cmgr_pdu_parse_number(unsigned int type, char const * number,
		size_t length, char * buf);
static time_t _cmgr_pdu_parse_timestamp(char const * timestamp);

static void _on_code_cmgr(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	/* XXX ugly */
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MESSAGE];
	char buf[32];
	char number[32];
	char date[32];
	struct tm t;
	unsigned int mbox;
	unsigned int alpha = 0;
	unsigned int length;
	char * p;
	HayesRequestMessageData * data;

	/* text mode support */
	if(sscanf(answer, "\"%31[^\"]\",\"%31[^\"]\",,\"%31[^\"]\"", buf,
				number, date) == 3)
	{
		number[sizeof(number) - 1] = '\0';
		string_delete(hayes->message_number);
		hayes->message_number = strdup(number);
		event->message.number = hayes->message_number;
		date[sizeof(date) - 1] = '\0';
		if(strptime(date, "%y/%m/%d,%H:%M:%S", &t) == NULL)
			/* XXX also parse the timezone? */
			localtime_r(NULL, &t);
		event->message.date = mktime(&t);
		event->message.length = 0;
		return; /* we need to wait for the next line */
	}
	/* PDU mode support */
	if(sscanf(answer, "%u,%u,%u", &mbox, &alpha, &length) == 3
			|| sscanf(answer, "%u,,%u", &mbox, &length) == 2)
		return; /* we need to wait for the next line */
	/* message content */
	if(event->message.length == 0) /* XXX assumes this is text mode */
	{
		/* FIXME guarantee this would not happen */
		if(command == NULL || (data = _hayes_command_get_data(command))
				== NULL)
			return;
		event->message.id = data->id;
		event->message.folder = data->folder;
		event->message.status = data->status;
		event->message.encoding = MODEM_MESSAGE_ENCODING_UTF8;
		event->message.content = answer;
		event->message.length = strlen(answer);
		hayes->helper->event(hayes->helper->modem, event);
		return;
	}
	if((p = _cmgr_pdu_parse(answer, &event->message.date, number,
					&event->message.encoding,
					&event->message.length)) == NULL)
		return;
	/* FIXME guarantee this would not happen */
	if(command == NULL || (data = _hayes_command_get_data(command)) == NULL)
		return;
	event->message.id = data->id;
	event->message.folder = data->folder;
	event->message.status = data->status;
	event->message.number = number; /* XXX */
	event->message.content = p;
	hayes->helper->event(hayes->helper->modem, event);
	free(p);
}

static char * _cmgr_pdu_parse(char const * pdu, time_t * timestamp,
		char * number, ModemMessageEncoding * encoding, size_t * length)
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

static char * _cmgr_pdu_parse_encoding_data(char const * pdu, size_t len,
		size_t i, size_t hdr, ModemMessageEncoding * encoding,
		size_t * length)
{
	unsigned char * p;
	size_t j;
	unsigned int u;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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
	*encoding = MODEM_MESSAGE_ENCODING_DATA;
	*length = j;
	p[j] = '\0';
	return (char *)p;
}

static char * _cmgr_pdu_parse_encoding_default(char const * pdu, size_t len,
                size_t i, size_t hdr, ModemMessageEncoding * encoding,
		size_t * length)
{
	unsigned char * p;
	size_t j;
	unsigned char rest;
	int shift = 0;
	char const * q;
	unsigned int u;
	unsigned char byte;
	char * r;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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
		p[j] = _hayes_convert_char_to_iso(p[j]);
		j++;
		rest = (byte >> (7 - shift)) & 0x7f;
		if(++shift == 7)
		{
			shift = 0;
			p[j++] = rest;
			rest = 0;
		}
	}
	*encoding = MODEM_MESSAGE_ENCODING_UTF8;
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


static void _cmgr_pdu_parse_number(unsigned int type, char const * number,
		size_t length, char * buf)
{
	char * b = buf;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, timestamp);
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


/* on_code_cmgs */
static void _on_code_cmgs(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_MESSAGE_SENT];
	unsigned int u;

	if(sscanf(answer, "%u", &u) != 1)
		return;
	event->message_sent.error = NULL;
	event->message_sent.id = u;
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cms_error */
static void _on_code_cms_error(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	unsigned int u;
	HayesCommand * p;

	if(command != NULL)
		_hayes_command_set_status(command, HCS_ERROR);
	if(sscanf(answer, "%u", &u) != 1)
		return;
	switch(u)
	{
		case 311: /* SIM PIN required */
			_on_code_cpin(modem, "SIM PIN");
			_hayes_trigger(modem, MODEM_EVENT_TYPE_AUTHENTICATION);
			break;
		case 316: /* SIM PUK required */
			_on_code_cpin(modem, "SIM PUK");
			_hayes_trigger(modem, MODEM_EVENT_TYPE_AUTHENTICATION);
			break;
		case 500: /* unknown error */
			if((hayes->quirks & HAYES_QUIRK_REPEAT_ON_UNKNOWN_ERROR)
					== 0)
				break;
		case 314: /* SIM busy */
			/* repeat the command */
			/* FIXME duplicated from _on_code_cme_error() */
			if(command == NULL)
				break;
			if((p = _hayes_command_new(command->attention)) == NULL)
				break;
			_hayes_command_set_callback(p, command->callback,
					command->priv);
			_hayes_command_set_data(p, command->data);
			_hayes_command_set_data(command, NULL);
			hayes->queue_timeout = g_slist_append(
					hayes->queue_timeout, p);
			if(hayes->source == 0)
				hayes->source = g_timeout_add(5000,
						_on_queue_timeout, modem);
			break;
		case 321: /* invalid memory index */
		default: /* FIXME implement the rest */
			break;
	}
}


/* on_code_cmti */
static void _on_code_cmti(ModemPlugin * modem, char const * answer)
{
	char buf[32];
	unsigned int u;
	ModemRequest request;

	if(sscanf(answer, "\"%31[^\"]\",%u", buf, &u) != 2)
		return;
	buf[sizeof(buf) - 1] = '\0';
	/* fetch the new message directly */
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_MESSAGE;
	request.message.id = u;
	_hayes_request(modem, &request);
}


/* on_code_connect */
static void _on_code_connect(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	char * argv[] = { "/usr/sbin/pppd", "pppd", "call", "phone",
		"user", "", "password", "", NULL };
	GSpawnFlags flags = G_SPAWN_FILE_AND_ARGV_ZERO;
	int wfd;
	int rfd;
	GError * error = NULL;

	if(command != NULL) /* XXX else report error? */
		_hayes_command_set_status(command, HCS_SUCCESS);
	_hayes_set_mode(modem, HAYES_MODE_DATA);
	if(hayes->gprs_username != NULL)
		argv[5] = hayes->gprs_username;
	if(hayes->gprs_password != NULL)
		argv[7] = hayes->gprs_password;
	if(g_spawn_async_with_pipes(NULL, argv, NULL, flags, NULL, NULL, NULL,
				&wfd, &rfd, NULL, &error) == FALSE)
	{
		hayes->helper->error(NULL, error->message, 1);
		g_error_free(error);
		_hayes_reset(modem);
		return;
	}
	hayes->rd_ppp_channel = g_io_channel_unix_new(rfd);
	g_io_channel_set_encoding(hayes->rd_ppp_channel, NULL, &error);
	g_io_channel_set_buffered(hayes->rd_ppp_channel, FALSE);
	hayes->rd_ppp_source = g_io_add_watch(hayes->rd_ppp_channel, G_IO_IN,
			_on_watch_can_read_ppp, modem);
	hayes->wr_ppp_channel = g_io_channel_unix_new(wfd);
	g_io_channel_set_encoding(hayes->wr_ppp_channel, NULL, &error);
	g_io_channel_set_buffered(hayes->wr_ppp_channel, FALSE);
	hayes->wr_ppp_source = 0;
	event->connection.connected = 1;
	event->connection.in = 0;
	event->connection.out = 0;
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_colp */
static void _on_code_colp(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];
	char buf[32];
	unsigned int u;

	if(sscanf(answer, "\"%31[^\"]\",%u", buf, &u) != 2)
		return; /* FIXME there may be different or more information */
	buf[sizeof(buf) - 1] = '\0';
	free(hayes->call_number);
	switch(u)
	{
		case 145:
			if((hayes->call_number = malloc(sizeof(buf) + 1))
					== NULL)
				break;
			snprintf(hayes->call_number, sizeof(buf) + 1, "%s%s",
					"+", buf);
			break;
		default:
			hayes->call_number = strdup(buf);
			break;
	}
	event->call.number = hayes->call_number;
}


/* on_code_cops */
static void _on_code_cops(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
	unsigned int u;
	unsigned int v = 0;
	char buf[32] = "";
	unsigned int w;

	if(sscanf(answer, "%u,%u,\"%31[^\"]\",%u", &u, &v, buf, &w) < 1)
		return;
	switch(u)
	{
		case 0:
			u = MODEM_REGISTRATION_MODE_AUTOMATIC;
			break;
		case 1:
			u = MODEM_REGISTRATION_MODE_MANUAL;
			break;
		case 2:
			u = MODEM_REGISTRATION_MODE_DISABLED;
			break;
		case 3: /* only for setting the format */
		default:
			u = event->registration.mode;
			break;
	}
	event->registration.mode = u;
	free(hayes->registration_operator);
	hayes->registration_operator = NULL;
	event->registration._operator = NULL;
	if(v != 0)
		/* force alphanumeric format */
		_hayes_request_type(modem, HAYES_REQUEST_OPERATOR_FORMAT_LONG);
	else
	{
		buf[sizeof(buf) - 1] = '\0';
		hayes->registration_operator = strdup(buf);
		event->registration._operator = hayes->registration_operator;
	}
	/* refresh registration data */
	_hayes_request_type(modem, MODEM_REQUEST_SIGNAL_LEVEL);
	_hayes_request_type(modem, HAYES_REQUEST_GPRS_ATTACHED);
	/* this is usually worth an event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cpas */
static void _on_code_cpas(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemPluginHelper * helper = hayes->helper;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];
	unsigned int u;

	if(sscanf(answer, "%u", &u) != 1)
		return;
	switch(u)
	{
		case 0:
			event->call.status = MODEM_CALL_STATUS_NONE;
			event->call.direction = MODEM_CALL_DIRECTION_NONE;
			/* report connection status */
			event = &hayes->events[MODEM_EVENT_TYPE_CONNECTION];
			event->connection.connected = 0;
			event->connection.in = 0;
			event->connection.out = 0;
			helper->event(helper->modem, event);
			break;
		case 3:
			event->call.status = MODEM_CALL_STATUS_RINGING;
			/* report event */
			helper->event(helper->modem, event);
			break;
		case 4:
			event->call.status = MODEM_CALL_STATUS_ACTIVE;
			event->call.direction = MODEM_CALL_DIRECTION_NONE;
			break;
		case 2: /* XXX unknown */
		default:
			break;
	}
}


/* on_code_cpbr */
static void _on_code_cpbr(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemRequest request;
	HayesRequestContactList list;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CONTACT];
	unsigned int u;
	unsigned int v;
	char number[32];
	char name[32];
	char * p;

	if(sscanf(answer, "(%u-%u)", &u, &v) == 2)
	{
		memset(&request, 0, sizeof(request));
		request.type = HAYES_REQUEST_CONTACT_LIST;
		list.from = u;
		list.to = v;
		request.plugin.data = &list;
		_hayes_request(modem, &request);
		return;
	}
	if(sscanf(answer, "%u,\"%31[^\"]\",%u,\"%31[^\"]\"",
				&event->contact.id, number, &u, name) != 4)
		return;
	switch(u)
	{
		case 145:
			/* FIXME could it be in some cases that the "+" is
			 * already there? (huawei) */
			memmove(&number[1], number, sizeof(number) - 1);
			number[0] = '+';
			break;
	}
	number[sizeof(number) - 1] = '\0';
	free(hayes->contact_number);
	hayes->contact_number = strdup(number);
	event->contact.number = hayes->contact_number;
	name[sizeof(name) - 1] = '\0';
#if 1 /* FIXME is it really always in ISO-8859-1? */
	if((p = g_convert(name, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL))
			                        != NULL)
	{
		snprintf(name, sizeof(name), "%s", p);
		g_free(p);
	}
#endif
	free(hayes->contact_name);
	hayes->contact_name = strdup(name);
	event->contact.name = hayes->contact_name;
	event->contact.status = MODEM_CONTACT_STATUS_OFFLINE;
	/* send event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cpin */
static void _on_code_cpin(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_AUTHENTICATION];
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, answer);
#endif
	if(strcmp(answer, "READY") == 0)
		event->authentication.status = MODEM_AUTHENTICATION_STATUS_OK;
	else if(strcmp(answer, "SIM PIN") == 0
			|| strcmp(answer, "SIM PUK") == 0)
	{
		free(hayes->authentication_name);
		p = strdup(answer);
		hayes->authentication_name = p;
		event->authentication.name = p;
		event->authentication.method = MODEM_AUTHENTICATION_METHOD_PIN;
		event->authentication.status
			= MODEM_AUTHENTICATION_STATUS_REQUIRED;
		/* FIXME also provide remaining retries */
	}
}


/* on_code_creg */
static void _on_code_creg(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
	int res;
	unsigned int u[4] = { 0, 0, 0, 0 };

	res = sscanf(answer, "%u,%u,%X,%X", &u[0], &u[1], &u[2], &u[3]);
	if(res == 1)
		res = sscanf(answer, "%u,\"%X\",\"%X\"", &u[1], &u[2], &u[3]);
	else if(res == 2)
		res = sscanf(answer, "%u,%u,\"%X\",\"%X\"", &u[0], &u[1], &u[2],
				&u[3]);
	else if(res == 3)
		res = sscanf(answer, "%u,%X,%X", &u[1], &u[2], &u[3]);
	if(res == 0)
		return;
	u[0] = event->registration.mode;
	event->registration.roaming = 0;
	switch(u[1])
	{
		case 0:
			u[0] = MODEM_REGISTRATION_MODE_DISABLED;
			u[1] = MODEM_REGISTRATION_STATUS_NOT_SEARCHING;
			break;
		case 1:
			if(u[0] != MODEM_REGISTRATION_MODE_MANUAL)
				u[0] = MODEM_REGISTRATION_MODE_AUTOMATIC;
			u[1] = MODEM_REGISTRATION_STATUS_REGISTERED;
			break;
		case 2:
			if(u[0] != MODEM_REGISTRATION_MODE_MANUAL)
				u[0] = MODEM_REGISTRATION_MODE_AUTOMATIC;
			u[1] = MODEM_REGISTRATION_STATUS_SEARCHING;
			break;
		case 3:
			u[1] = MODEM_REGISTRATION_STATUS_DENIED;
			break;
		case 5:
			if(u[0] != MODEM_REGISTRATION_MODE_MANUAL)
				u[0] = MODEM_REGISTRATION_MODE_AUTOMATIC;
			u[1] = MODEM_REGISTRATION_STATUS_REGISTERED;
			event->registration.roaming = 1;
			break;
		case 4: /* unknown */
		default:
#ifdef DEBUG
			if(u[1] != 4)
				fprintf(stderr, "DEBUG: %s() Unknown CREG %u\n",
						__func__, u[1]);
#endif
			u[0] = MODEM_REGISTRATION_MODE_UNKNOWN;
			u[1] = MODEM_REGISTRATION_STATUS_UNKNOWN;
			break;
	}
	event->registration.mode = u[0];
	switch((event->registration.status = u[1]))
	{
		case MODEM_REGISTRATION_STATUS_REGISTERED:
			/* refresh registration data */
			_hayes_request_type(modem, HAYES_REQUEST_OPERATOR);
			break;
		default:
			free(hayes->registration_media);
			hayes->registration_media = NULL;
			event->registration.media = NULL;
			free(hayes->registration_operator);
			hayes->registration_operator = NULL;
			event->registration._operator = NULL;
			event->registration.signal = 0.0 / 0.0;
			break;
	}
	/* this is usually an unsollicited event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cring */
static void _on_code_cring(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_CALL];

	if(strcmp(answer, "VOICE") == 0)
		event->call.call_type = MODEM_CALL_TYPE_VOICE;
	event->call.status = MODEM_CALL_STATUS_RINGING;
	event->call.direction = MODEM_CALL_DIRECTION_INCOMING;
	event->call.number = "";
	/* this is always an unsollicited event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_csq */
static void _on_code_csq(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_REGISTRATION];
	unsigned int u;
	unsigned int v;

	if(sscanf(answer, "%u,%u", &u, &v) != 2)
		return;
	if(u > 31)
		event->registration.signal = 0.0 / 0.0;
	else if(u <= 12)
		event->registration.signal = 1.0;
	else
		event->registration.signal = (32.0 - u) / 20.0;
	/* this is usually worth an event */
	hayes->helper->event(hayes->helper->modem, event);
}


/* on_code_cusd */
static void _on_code_cusd(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	ModemEvent * event = &hayes->events[MODEM_EVENT_TYPE_NOTIFICATION];
	unsigned int u;
	char buf[32];

	/* FIXME really implement */
	if(sscanf(answer, "%u\"%31[^\"]\",%u", &u, buf, &u) >= 2)
	{
		event->notification.content = buf;
		hayes->helper->event(hayes->helper->modem, event);
	}
}


/* on_code_ext_error */
static void _on_code_ext_error(ModemPlugin * modem, char const * answer)
{
	Hayes * hayes = modem;
	/* XXX ugly */
	HayesCommand * command = (hayes->queue != NULL) ? hayes->queue->data
		: NULL;
	unsigned int u;

	if(command != NULL)
		_hayes_command_set_status(command, HCS_ERROR);
	if(sscanf(answer, "%u", &u) != 1)
		return;
	switch(u)
	{
		case 0:
		default: /* FIXME implement */
			break;
	}
}


/* helpers */
/* is_figure */
static int _is_figure(int c)
{
	if(c >= '0' && c <= '9')
		return 1;
	if(c == '*' || c == '+' || c == '#')
		return 1;
	if(c >= 'A' && c <= 'D')
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
