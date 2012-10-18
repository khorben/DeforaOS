/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libApp */
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



#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <System.h>
#include "App/apptransport.h"


/* UDP */
/* private */
/* types */
typedef struct _AppTransportPlugin UDP;

struct _AppTransportPlugin
{
	AppTransportPluginHelper * helper;
	int fd;
};


/* protected */
/* prototypes */
/* plug-in */
static UDP * _udp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _udp_destroy(UDP * udp);

/* useful */
static int _udp_error(char const * message, int code);


/* public */
/* constants */
/* plug-in */
AppTransportPluginDefinition definition =
{
	"UDP",
	NULL,
	_udp_init,
	_udp_destroy,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* udp_init */
static int _init_client(UDP * udp, char const * name);
static int _init_server(UDP * udp, char const * name);

static UDP * _udp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	UDP * udp;
	int res = -1;

	if((udp = object_new(sizeof(*udp))) == NULL)
		return NULL;
	udp->helper = helper;
	udp->fd = -1;
	switch(mode)
	{
		case ATM_CLIENT:
			res = _init_client(udp, name);
			break;
		case ATM_SERVER:
			res = _init_server(udp, name);
			break;
	}
	/* check for errors */
	if(res != 0)
	{
		_udp_destroy(udp);
		return NULL;
	}
	return udp;
}

static int _init_client(UDP * udp, char const * name)
{
	if((udp->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -_udp_error("socket", 1);
	/* FIXME really implement */
	return -1;
}

static int _init_server(UDP * udp, char const * name)
{
	if((udp->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -_udp_error("socket", 1);
	/* FIXME really implement */
	return -1;
}


/* udp_destroy */
static void _udp_destroy(UDP * udp)
{
	/* FIXME really implement */
	object_delete(udp);
}

/* useful */
static int _udp_error(char const * message, int code)
{
	return error_set_code(code, "%s%s%s", (message != NULL) ? message : "",
			(message != NULL) ? ": " : "", strerror(errno));
}
