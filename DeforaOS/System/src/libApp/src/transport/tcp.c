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
static TCP * _tcp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	TCP * tcp;

	if((tcp = object_new(sizeof(*tcp))) == NULL)
		return NULL;
	tcp->helper = helper;
	tcp->fd = -1;
	/* FIXME really implement */
	return tcp;
}


/* tcp_destroy */
static void _tcp_destroy(TCP * tcp)
{
	/* FIXME really implement */
	object_delete(tcp);
}
