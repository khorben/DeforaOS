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
/* accessors */
static VPNClient * _client_get(void);
static VPNClient * _client_check(VPNClient * client, int32_t fd);
static int _client_remove_socket(VPNClient * client, int32_t fd);


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


/* private */
/* functions */
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


/* client_remove_socket */
static int _client_remove_socket(VPNClient * client, int32_t fd)
{
	size_t i;
	int32_t * p;

	if(fd < 0) /* XXX should never happen */
		return error_set_print(PACKAGE, 1, "%s", strerror(EINVAL));
	if(client == NULL && (client = _client_get()) == NULL)
		return 1;
	for(i = 0; i < client->sockets_cnt; i++)
		if(client->sockets[i] == fd)
			break;
	if(i == client->sockets_cnt)
		return 0;
	p = &client->sockets[i];
	memmove(p, p + 1, (--client->sockets_cnt - i) * sizeof(*p));
	if((p = realloc(client->sockets, sizeof(*p) * client->sockets_cnt))
			!= NULL || client->sockets_cnt == 0)
		client->sockets = p; /* we can ignore errors */
	return 0;
}
