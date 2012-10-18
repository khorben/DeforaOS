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
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <limits.h>
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
static int _tcp_callback_connect(int fd, TCP * tcp);
static int _tcp_callback_read(int fd, TCP * tcp);


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
static int _init_address(char const * name, struct sockaddr_in * sa);
static int _init_client(TCP * tcp, char const * name);
static int _init_server(TCP * tcp, char const * name);
static int _init_socket(TCP * tcp);

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

static int _init_address(char const * name, struct sockaddr_in * sa)
{
	long l;
	char const * p;
	char * q;

	/* obtain the port number */
	if((p = strrchr(name, ':')) == NULL)
		return -error_set_code(1, "%s", strerror(EINVAL));
	l = strtol(++p, &q, 10);
	if(p[0] == '\0' || *q != '\0' || l < 0 || l > 65535)
		return -error_set_code(1, "%s", strerror(EINVAL));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(l);
	/* FIXME hard-coded */
	sa->sin_addr.s_addr = htons(INADDR_LOOPBACK);
	return 0;
}

static int _init_client(TCP * tcp, char const * name)
{
	struct sockaddr_in sa;

	/* obtain the remote address */
	if(_init_address(name, &sa) != 0)
		return -1;
	/* create the socket */
	if(_init_socket(tcp) != 0)
		return -1;
	/* connect to the remote host */
	if(connect(tcp->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
	{
		if(errno != EINPROGRESS)
			return -_tcp_error("socket", 1);
		else
			event_register_io_write(tcp->helper->event, tcp->fd,
					(EventIOFunc)_tcp_callback_connect,
					tcp);
	}
	else
		event_register_io_read(tcp->helper->event, tcp->fd,
				(EventIOFunc)_tcp_callback_read, tcp);
	return 0;
}

static int _init_server(TCP * tcp, char const * name)
{
	struct sockaddr_in sa;

	/* obtain the local address */
	if(_init_address(name, &sa) != 0)
		return -1;
	/* create the socket */
	if(_init_socket(tcp) != 0)
		return -1;
	/* accept incoming connections */
	if(bind(tcp->fd, (struct sockaddr *)&sa, sizeof(sa)) != 0)
		return -_tcp_error("bind", 1);
	if(listen(tcp->fd, 5) != 0)
		return -_tcp_error("listen", 1);
	event_register_io_read(tcp->helper->event, tcp->fd,
			(EventIOFunc)_tcp_callback_accept, tcp);
	/* FIXME implement */
	return -1;
}

static int _init_socket(TCP * tcp)
{
	int flags;

	if((tcp->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -_tcp_error("socket", 1);
	/* set the socket as non-blocking */
	if((flags = fcntl(tcp->fd, F_GETFL)) == -1)
		return -_tcp_error("fcntl", 1);
	if((flags & O_NONBLOCK) == 0)
		if(fcntl(tcp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
			return -_tcp_error("fcntl", 1);
	return 0;
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

	/* check parameters */
	if(tcp->fd != fd)
		return -1;
	if((newfd = accept(fd, (struct sockaddr *)&sa, &sa_size)) < 0)
		return _tcp_error("accept", 1);
	/* FIXME really implement */
	close(newfd);
	return 0;
}


/* tcp_callback_connect */
static int _tcp_callback_connect(int fd, TCP * tcp)
{
	int res;
	socklen_t s = sizeof(res);

	/* check parameters */
	if(tcp->fd != fd)
		return -1;
	if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &res, &s) != 0)
	{
		close(fd);
		tcp->fd = -1;
		/* FIXME report error */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %s\n", __func__, strerror(errno));
#endif
		return -1;
	}
	if(res != 0)
	{
		close(fd);
		tcp->fd = -1;
		/* FIXME report error */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() %s\n", __func__, strerror(errno));
#endif
		return -1;
	}
	/* listen for any incoming message */
	event_register_io_read(tcp->helper->event, tcp->fd,
			(EventIOFunc)_tcp_callback_read, tcp);
	/* XXX check the return value (stop polling on writes) */
	return 1;
}


/* tcp_callback_read */
static int _tcp_callback_read(int fd, TCP * tcp)
{
	/* check parameters */
	if(tcp->fd != fd)
		return -1;
	/* FIXME really implement */
	close(fd);
	tcp->fd = -1;
	return -1;
}
