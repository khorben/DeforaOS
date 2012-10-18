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


/* TCP */
/* private */
/* types */
typedef struct _AppTransportPlugin TCP;

struct _AppTransportPlugin
{
	AppTransportPluginHelper * helper;
	int fd;
};


/* protected */
/* prototypes */
/* plug-in */
static TCP * _tcp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _tcp_destroy(TCP * tcp);

/* useful */
static int _tcp_error(char const * message, int code);

/* callbacks */
static int _tcp_callback_accept(int fd, TCP * tcp);


/* public */
/* constants */
/* plug-in */
AppTransportPluginDefinition definition =
{
	"TCP",
	NULL,
	_tcp_init,
	_tcp_destroy,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* tcp_init */
static int _init_client(TCP * tcp, char const * name);
static int _init_server(TCP * tcp, char const * name);

static TCP * _tcp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	TCP * tcp;
	int res = -1;

	if((tcp = object_new(sizeof(*tcp))) == NULL)
		return NULL;
	tcp->helper = helper;
	tcp->fd = -1;
	switch(mode)
	{
		case ATM_CLIENT:
			res = _init_client(tcp, name);
			break;
		case ATM_SERVER:
			res = _init_server(tcp, name);
			break;
	}
	/* check for errors */
	if(res != 0)
	{
		_tcp_destroy(tcp);
		return NULL;
	}
	return tcp;
}

static int _init_client(TCP * tcp, char const * name)
{
	if((tcp->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -_tcp_error("socket", 1);
	/* FIXME really implement */
	return -1;
}

static int _init_server(TCP * tcp, char const * name)
{
	struct sockaddr_in sa;

	if((tcp->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -_tcp_error("socket", 1);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(4241); /* XXX hard-coded */
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(tcp->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return -_tcp_error("bind", 1);
	if(listen(tcp->fd, 5) != 0)
		return -_tcp_error("listen", 1);
	event_register_io_read(tcp->helper->event, tcp->fd,
			_tcp_callback_accept, tcp);
	/* FIXME implement */
	return -1;
}


/* tcp_destroy */
static void _tcp_destroy(TCP * tcp)
{
	/* FIXME really implement */
	if(tcp->fd >= 0)
		close(tcp->fd);
	object_delete(tcp);
}


/* useful */
/* tcp_error */
static int _tcp_error(char const * message, int code)
{
	return error_set_code(code, "%s%s%s", (message != NULL) ? message : "",
			(message != NULL) ? ": " : "", strerror(errno));
}


/* callbacks */
/* tcp_callback_accept */
static int _tcp_callback_accept(int fd, TCP * tcp)
{
	struct sockaddr_in sa;
	socklen_t sa_size = sizeof(sa);
	int newfd;

	if((newfd = accept(fd, (struct sockaddr *)&sa, &sa_size)) < 0)
		return _tcp_error("accept", 1);
	/* FIXME really implement */
	close(newfd);
	return 0;
}
