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


/* OpenSSL */
/* private */
/* types */
typedef struct _AppTransportPlugin OpenSSL;

struct _AppTransportPlugin
{
	AppTransportPluginHelper * helper;
	int fd;
};


/* protected */
/* prototypes */
/* plug-in */
static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name);
static void _openssl_destroy(OpenSSL * openssl);


/* public */
/* constants */
/* plug-in */
AppTransportPluginDefinition definition =
{
	"OpenSSL",
	NULL,
	_openssl_init,
	_openssl_destroy,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* openssl_init */
static OpenSSL * _openssl_init(AppTransportPluginHelper * helper,
		AppTransportMode mode, char const * name)
{
	OpenSSL * openssl;

	if((openssl = object_new(sizeof(*openssl))) == NULL)
		return NULL;
	openssl->helper = helper;
	openssl->fd = -1;
	/* FIXME really implement */
	return openssl;
}


/* openssl_destroy */
static void _openssl_destroy(OpenSSL * openssl)
{
	/* FIXME really implement */
	object_delete(openssl);
}
