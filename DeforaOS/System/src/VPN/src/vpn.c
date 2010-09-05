/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VPN */
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



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "VPN.h"
#include "../data/VPN.h"
#include "../config.h"


/* VPN */
/* private */
/* types */
typedef struct _VPNClient
{
	void * id;
	int32_t * sockets;
	size_t sockets_cnt;
} VPNClient;


/* variables */
static AppServer * _appserver;
static VPNClient * _clients;
static size_t _clients_cnt;


/* prototypes */
/* client management */
static void _client_init(void);
static void _client_destroy(void);

/* accessors */
static VPNClient * _client_get(void);
static VPNClient * _client_check(VPNClient * client, int32_t fd);

/* useful */
static VPNClient * _client_add(VPNClient * client);
static VPNClient * _client_add_socket(VPNClient * client, int32_t fd);
static VPNClient * _client_remove_socket(VPNClient * client, int32_t fd);


/* public */
/* functions */
/* vpn */
int vpn(AppServerOptions options)
{
	if((_appserver = appserver_new("VPN", options)) == NULL)
	{
		error_print(PACKAGE);
		return 1;
	}
	_client_init();
	appserver_loop(_appserver);
	_client_destroy();
	appserver_delete(_appserver);
	return 0;
}


/* interface */
/* VPN_close */
int32_t VPN_close(int32_t fd)
{
	VPNClient * client;
	int32_t ret;

	if((client = _client_check(NULL, fd)) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	if((ret = close(fd)) == 0)
		_client_remove_socket(client, fd);
	return ret;
}


/* VPN_connect */
int32_t VPN_connect(uint32_t protocol, String const * uri)
{
	int32_t ret;
	VPNProtocol vprotocol = protocol;
	int sdomain;
	int stype;
	int sprotocol = 0;
	struct sockaddr * sockaddr;
	socklen_t ssize;
	struct sockaddr_in sa_in;

	switch(vprotocol)
	{
		case VPN_PROTOCOL_IP_TCP:
			sdomain = PF_INET;
			stype = SOCK_STREAM;
			sa_in.sin_family = AF_INET;
			/* FIXME parse uri to initialize sa_in */
			sa_in.sin_addr.s_addr = htonl(0x7f000001);
			sa_in.sin_port = htons(80);
			sockaddr = (struct sockaddr*)&sa_in;
			ssize = sizeof(sa_in);
			break;
		default:
			return -1;
	}
	if((ret = socket(sdomain, stype, sprotocol)) == -1)
		return -1;
	if(connect(ret, sockaddr, ssize) != 0
			|| _client_add_socket(NULL, ret) == NULL)
	{
		close(ret); /* XXX necessary when connect() failed? */
		return -1;
	}
	return ret;
}


/* VPN_recv */
int32_t VPN_recv(int32_t fd, Buffer * buffer, uint32_t size, uint32_t flags)
{
	int32_t ret;

	if(_client_check(NULL, fd) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, buf, %u, %u)\n", __func__, fd, size,
			flags);
#endif
	if(buffer_set_size(buffer, size) != 0)
		return -1;
	/* FIXME implement flags */
	ret = recv(fd, buffer_get_data(buffer), size, 0);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, buf, %u, %u) => %d\n", __func__, fd,
			size, flags, ret);
#endif
	if(buffer_set_size(buffer, (ret < 0) ? 0 : ret) != 0)
	{
		memset(buffer_get_data(buffer), 0, size);
		return -1;
	}
	return ret;
}


/* VPN_send */
int32_t VPN_send(int32_t fd, Buffer * buffer, uint32_t size, uint32_t flags)
{
	if(_client_check(NULL, fd) == NULL)
		return -1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, buf, %u, %u)\n", __func__, fd, size,
			flags);
#endif
	if(buffer_get_size(buffer) < size)
		return -error_set_code(1, "%s", strerror(EINVAL));
	/* FIXME implement flags */
	return send(fd, buffer_get_data(buffer), size, 0);
}


/* private */
/* functions */
/* client management */
/* clients_init */
static void _client_init(void)
{
	_clients = NULL;
	_clients_cnt = 0;
}


/* clients_destroy */
static void _client_destroy(void)
{
	free(_clients);
	_clients = NULL;
	_clients_cnt = 0;
}


/* accessors */
/* client_get */
static VPNClient * _client_get(void)
{
	void * id;
	size_t i;

	if((id = appserver_get_client_id(_appserver)) == NULL)
	{
		error_print(PACKAGE);
		return NULL;
	}
	for(i = 0; i < _clients_cnt; i++)
		if(_clients[i].id == id)
			return &_clients[i];
	return NULL;
}


/* client_check */
static VPNClient * _client_check(VPNClient * client, int32_t fd)
{
	size_t i;

	if(client == NULL && (client = _client_get()) == NULL)
		return NULL;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, fd);
#endif
	for(i = 0; i < client->sockets_cnt; i++)
		if(client->sockets[i] == fd)
			return client;
	return NULL;
}


/* useful */
/* client_add */
static VPNClient * _client_add(VPNClient * client)
{
	void * id;
	VPNClient * p;

	if(client == NULL && (client = _client_get()) != NULL)
		return client;
	if((id = appserver_get_client_id(_appserver)) == NULL)
	{
		error_print(PACKAGE);
		return NULL;
	}
	if((p = realloc(_clients, sizeof(*p) * (_clients_cnt + 1))) == NULL)
	{
		error_set_print(PACKAGE, 1, "%s", strerror(errno));
		return NULL;
	}
	_clients = p;
	p = &_clients[_clients_cnt++];
	p->id = id;
	p->sockets = NULL;
	p->sockets_cnt = 0;
	return p;
}


/* client_add_socket */
static VPNClient * _client_add_socket(VPNClient * client, int32_t fd)
{
	int32_t * p;

	if((client = _client_add(client)) == NULL)
		return NULL;
	if((p = realloc(client->sockets, sizeof(*p)
					* (client->sockets_cnt + 1))) == NULL)
	{
		error_set_print(PACKAGE, 1, "%s", strerror(errno));
		return NULL;
	}
	client->sockets = p;
	client->sockets[client->sockets_cnt++] = fd;
	return client;
}


/* client_remove_socket */
static VPNClient * _client_remove_socket(VPNClient * client, int32_t fd)
{
	size_t i;
	int32_t * p;

	if(fd < 0) /* XXX should never happen */
	{
		error_set_print(PACKAGE, 1, "%s", strerror(EINVAL));
		return NULL;
	}
	if(client == NULL && (client = _client_get()) == NULL)
		return NULL;
	for(i = 0; i < client->sockets_cnt; i++)
		if(client->sockets[i] == fd)
			break;
	if(i == client->sockets_cnt)
		return client;
	p = &client->sockets[i];
	memmove(p, p + 1, (--client->sockets_cnt - i) * sizeof(*p));
	if((p = realloc(client->sockets, sizeof(*p) * client->sockets_cnt))
			!= NULL || client->sockets_cnt == 0)
		client->sockets = p; /* we can ignore errors */
	return client;
}
