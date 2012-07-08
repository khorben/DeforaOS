/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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



#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <osmocom/core/select.h>
#include <osmocom/core/serial.h>
#include <osmocom/core/timer.h>
#include <glib.h>
#include <System.h>
#include <Phone/modem.h>
#include "../../config.h"


/* Osmocom */
/* private */
/* types */
struct tool_server
{
	struct osmo_fd bfd;
	uint8_t dlci;
	struct llist_head connections;
};

enum dnload_state
{
	WAITING_PROMPT1,
	WAITING_PROMPT2,
	DOWNLOADING
};

enum romload_state
{
	WAITING_IDENTIFICATION,
	WAITING_PARAM_ACK,
	SENDING_BLOCKS,
	SENDING_LAST_BLOCK,
	LAST_BLOCK_SENT,
	WAITING_BLOCK_ACK,
	WAITING_CHECKSUM_ACK,
	WAITING_BRANCH_ACK,
	FINISHED
};

enum mtk_state
{
	MTK_INIT_1,
	MTK_INIT_2,
	MTK_INIT_3,
	MTK_INIT_4,
	MTK_WAIT_WRITE_ACK,
	MTK_WAIT_ADDR_ACK,
	MTK_WAIT_SIZE_ACK,
	MTK_SENDING_BLOCKS,
	MTK_WAIT_BRANCH_CMD_ACK,
	MTK_WAIT_BRANCH_ADDR_ACK,
	MTK_FINISHED
};

enum dnload_mode
{
	MODE_C123,
	MODE_C123xor,
	MODE_C140,
	MODE_C140xor,
	MODE_C155,
	MODE_ROMLOAD,
	MODE_MTK,
	MODE_INVALID
};

typedef struct _OsmocomDnload
{
	enum dnload_state state;
	enum romload_state romload_state;
	enum mtk_state mtk_state;
	enum dnload_mode mode, previous_mode;
	struct osmo_fd serial_fd;
	char *filename, *previous_filename;
	char *chainload_filename;

	int expect_hdlc;

	int dump_rx;
	int dump_tx;
	int beacon_interval;

	/* data to be downloaded */
	uint8_t *data;
	int data_len;

	uint8_t *write_ptr;

	/* romload: block to be downloaded */
	uint8_t *block;
	int block_len;
	uint8_t block_number;
	uint16_t block_payload_size;
	int romload_dl_checksum;
	uint8_t *block_ptr;
	uint8_t load_address[4];

	uint8_t mtk_send_size[4];
	int block_count;
	int echo_bytecount;

	struct tool_server layer2_server;
	struct tool_server loader_server;
} OsmocomDnload;

typedef struct _ModemPlugin
{
	ModemPluginHelper * helper;

	guint reset;

	/* modem */
	struct osmo_fd fd;
	guint source;
	OsmocomDnload dnload;
	struct osmo_timer_list tick_timer;
} Osmocom;


/* constants */
#define ROMLOAD_INIT_BAUDRATE	B19200
#define ROMLOAD_DL_BAUDRATE	B115200
#define ROMLOAD_BLOCK_HDR_LEN	10
#define ROMLOAD_ADDRESS		0x820000

#define MTK_INIT_BAUDRATE	B19200
#define MTK_ADDRESS		0x40001400
#define MTK_BLOCK_SIZE		1024

static const uint8_t romload_ident_cmd[] = { 0x3c, 0x69 }; /* <i */

/* MTK romloader specific */
static const uint8_t mtk_init_cmd[] = { 0xa0, 0x0a, 0x50, 0x05 };
static const uint8_t mtk_init_resp[] = { 0x5f, 0xf5, 0xaf, 0xfa };
static const uint8_t mtk_command[] = { 0xa1, 0xa2, 0xa4, 0xa8 };


/* variables */
/* FIXME ugly hack to circumvent API limitations in Osmocom */
Osmocom * osmocom;

static ModemConfig _osmocom_config[] =
{
	{ "device",	"Device",		MCT_FILENAME	},
	{ "baudrate",	"Baudrate",		MCT_UINT32	},
	{ "hwflow",	"Hardware flow control",MCT_BOOLEAN	},
	{ NULL,		NULL,			MCT_NONE	}
};


/* prototypes */
static ModemPlugin * _osmocom_init(ModemPluginHelper * helper);
static void _osmocom_destroy(ModemPlugin * modem);
static int _osmocom_start(ModemPlugin * modem, unsigned int retry);
static int _osmocom_stop(ModemPlugin * modem);
static int _osmocom_request(ModemPlugin * modem, ModemRequest * request);

/* callbacks */
static gboolean _osmocom_on_idle(gpointer data);
static gboolean _osmocom_on_reset(gpointer data);

/* XXX re-write both to be non-blocking instead */
static void _osmocom_on_beacon_timer(void * data);
static void _osmocom_on_mtk_timer(void * data);

static int _osmocom_on_serial_read(struct osmo_fd * fd, unsigned int flags);


/* public */
/* variables */
ModemPluginDefinition plugin =
{
	"Osmocom",
	NULL,
	_osmocom_config,
	_osmocom_init,
	_osmocom_destroy,
	_osmocom_start,
	_osmocom_stop,
	_osmocom_request,
	NULL
};


/* private */
/* functions */
/* osmocom_init */
static ModemPlugin * _osmocom_init(ModemPluginHelper * helper)
{
	if((osmocom = object_new(sizeof(*osmocom))) == NULL)
		return NULL;
	memset(osmocom, 0, sizeof(*osmocom));
	osmocom->helper = helper;
	return osmocom;
}


/* osmocom_destroy */
static void _osmocom_destroy(ModemPlugin * modem)
{
	_osmocom_stop(modem);
	object_delete(osmocom);
	osmocom = NULL;
}


/* osmocom_reset */
static int _reset_open(ModemPlugin * modem);
static unsigned int _reset_baudrate(ModemPlugin * modem, unsigned int baudrate);

static int _osmocom_reset(ModemPlugin * modem, unsigned int retry)
{
	Osmocom * osmocom = modem;

	_osmocom_stop(modem);
	if(_reset_open(modem) != 0)
	{
		if(retry > 0)
			osmocom->reset = g_timeout_add(retry,
					_osmocom_on_reset, modem);
		return -1;
	}
#if 0
	osmocom->channel = g_io_channel_unix_new(fd);
	if(g_io_channel_set_encoding(osmocom->channel, NULL, &error)
			                        != G_IO_STATUS_NORMAL)
	{
		modem->helper->error(modem->helper->modem, error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(osmocom->channel, FALSE);
	osmocom->
#else
	osmocom->source = g_idle_add(_osmocom_on_idle, modem);
#endif
	return 0;
}

static int _reset_open(ModemPlugin * modem)
{
	Osmocom * osmocom = modem;
	ModemPluginHelper * helper = osmocom->helper;
	char const * device;
	unsigned int baudrate;
	int flags;
	uint32_t tmpaddr = ROMLOAD_ADDRESS;
	char const * p;

	if((device = helper->config_get(helper->modem, "device")) == NULL)
		device = "/dev/modem";
	if((p = helper->config_get(helper->modem, "baudrate")) == NULL
			|| (baudrate = strtoul(p, NULL, 10)) == 0)
		baudrate = 115200;
	baudrate = _reset_baudrate(modem, baudrate);
	if((osmocom->fd.fd = osmo_serial_init(device, baudrate)) < 0)
	{
		/* XXX report error */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s: %s\n", device, strerror(errno));
#endif
		return -1;
	}
	if(osmo_fd_register(&osmocom->fd) != 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s: %s\n", device, strerror(errno));
#endif
		/* XXX report error */
		return -1;
	}
	/* Set serial socket to non-blocking mode of operation */
	if((flags = fcntl(osmocom->fd.fd, F_GETFL)) != -1)
	{
		flags |= O_NONBLOCK;
		fcntl(osmocom->fd.fd, F_SETFL, flags);
	}
	osmocom->dnload.serial_fd.when = BSC_FD_READ;
	osmocom->dnload.serial_fd.cb = _osmocom_on_serial_read;
	if(osmocom->dnload.mode == MODE_ROMLOAD)
	{
		tmpaddr = ROMLOAD_ADDRESS;
		osmo_serial_set_baudrate(osmocom->fd.fd, ROMLOAD_INIT_BAUDRATE);
		osmocom->tick_timer.cb = &_osmocom_on_beacon_timer;
		osmocom->tick_timer.data = modem;
		osmo_timer_schedule(&osmocom->tick_timer, 0,
				osmocom->dnload.beacon_interval);
	}
	else
	{
		tmpaddr = MTK_ADDRESS;
		osmo_serial_set_baudrate(osmocom->fd.fd, MTK_INIT_BAUDRATE);
		osmocom->tick_timer.cb = &_osmocom_on_mtk_timer;
		osmocom->tick_timer.data = modem;
		osmo_timer_schedule(&osmocom->tick_timer, 0,
				osmocom->dnload.beacon_interval);
	}
	/* FIXME not endian proof */
	osmocom->dnload.load_address[0] = (tmpaddr >> 24) & 0xff;
	osmocom->dnload.load_address[1] = (tmpaddr >> 16) & 0xff;
	osmocom->dnload.load_address[2] = (tmpaddr >> 8) & 0xff;
	osmocom->dnload.load_address[3] = tmpaddr & 0xff;
	return 0;
}

static unsigned int _reset_baudrate(ModemPlugin * modem, unsigned int baudrate)
{
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
		case 460800:
			return B460800;
		case 921600:
			return B921600;
		default:
			error_set("%u%s", baudrate,
					"Unsupported baudrate (using 115200)");
			modem->helper->error(NULL, error_get(), 1);
			return B115200;
	}
}


/* osmocom_start */
static int _osmocom_start(ModemPlugin * modem, unsigned int retry)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_osmocom_reset(modem, retry);
	return 0;
}


/* osmocom_stop */
static int _osmocom_stop(ModemPlugin * modem)
{
	Osmocom * osmocom = modem;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(osmocom->source != 0)
	{
		g_source_remove(osmocom->source);
		osmocom->source = 0;
	}
	return 0;
}


/* osmocom_request */
static int _request_call(ModemPlugin * modem, ModemRequest * request);
static int _request_message_send(ModemPlugin * modem, ModemRequest * request);

static int _osmocom_request(ModemPlugin * modem, ModemRequest * request)
{
	switch(request->type)
	{
		case MODEM_REQUEST_CALL:
			return _request_call(modem, request);
		case MODEM_REQUEST_MESSAGE_SEND:
			return _request_message_send(modem, request);
#ifdef DEBUG
		default:
			break;
#endif
	}
	return 0;
}

static int _request_call(ModemPlugin * modem, ModemRequest * request)
{
	Osmocom * osmocom = modem;

	/* FIXME implement */
	return -1;
}

static int _request_message_send(ModemPlugin * modem, ModemRequest * request)
{
	Osmocom * osmocom = modem;

	/* FIXME implement */
	return -1;
}


/* callbacks */
/* osmocom_on_idle */
static gboolean _osmocom_on_idle(gpointer data)
{
	ModemPlugin * modem = data;
	Osmocom * osmocom = modem;

	if(osmo_select_main(0) < 0)
	{
		osmocom->source = 0;
		return FALSE;
	}
	return TRUE;
}


/* osmocom_on_reset */
static gboolean _osmocom_on_reset(gpointer data)
{
	ModemPlugin * modem = data;
	Osmocom * osmocom = modem;

	if(_osmocom_reset(modem, 0) == 0)
	{
		osmocom->reset = 0;
		return FALSE;
	}
	return TRUE;
}


/* osmocom_on_beacon_timer */
static void _osmocom_on_beacon_timer(void * data)
{
	ModemPlugin * modem = data;
	Osmocom * osmocom = modem;
	int rc;

	if(osmocom->dnload.romload_state == WAITING_IDENTIFICATION)
	{
		printf("Sending Calypso romloader beacon...\n");
		rc = write(osmocom->dnload.serial_fd.fd, romload_ident_cmd,
				sizeof(romload_ident_cmd));
		if(rc != sizeof(romload_ident_cmd))
			printf("Error sending identification beacon\n");
		osmo_timer_schedule(&osmocom->tick_timer, 0,
				osmocom->dnload.beacon_interval);
	}
}


/* osmocom_on_mtk_timer */
static void _osmocom_on_mtk_timer(void * data)
{
	ModemPlugin * modem = data;
	Osmocom * osmocom = modem;
	int rc;

	if(osmocom->dnload.mtk_state == MTK_INIT_1)
	{
		printf("Sending MTK romloader beacon...\n");
		rc = write(osmocom->dnload.serial_fd.fd, &mtk_init_cmd[0], 1);
		if(rc != 1)
			printf("Error sending identification beacon\n");
		osmo_timer_schedule(&osmocom->tick_timer, 0,
				osmocom->dnload.beacon_interval);
	}
}


/* osmocom_on_serial_read */
static int _read_handle_romload();
static int _read_handle();
static int _read_handle_write();

static int _osmocom_on_serial_read(struct osmo_fd * fd, unsigned int flags)
{
	int rc;

	/* FIXME really implement */
	if(flags & BSC_FD_READ)
	{
#if 0
		switch(osmocom->mode)
		{
			case MODE_ROMLOAD:
				rc = _read_handle_romload();
				break;
			default:
				rc = _read_handle();
				break;
		}
		if(rc == 0)
			/* XXX wtf? */
			exit(2);
#endif
	}
	if(flags & BSC_FD_WRITE)
	{
		rc = _read_handle_write();
		if(rc != 0)
			osmocom->dnload.state = WAITING_PROMPT1;
	}
	return 0;
}

static int _read_handle_romload()
{
	return -1;
}

static int _read_handle()
{
	return -1;
}

static int _read_handle_write()
{
	return -1;
}
