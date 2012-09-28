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
static UDP * _udp_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	UDP * udp;

	if((udp = object_new(sizeof(*udp))) == NULL)
		return NULL;
	udp->helper = helper;
	udp->fd = -1;
	/* FIXME really implement */
	return udp;
}


/* udp_destroy */
static void _udp_destroy(UDP * udp)
{
	/* FIXME really implement */
	object_delete(udp);
}
