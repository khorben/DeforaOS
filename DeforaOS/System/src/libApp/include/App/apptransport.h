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



#ifndef LIBAPP_APP_APPTRANSPORT_H
# define LIBAPP_APP_APPTRANSPORT_H

# include <System/event.h>
# include "app.h"


/* AppTransport */
/* types */
typedef struct _AppTransport AppTransport;

typedef struct _AppTransportClient * AppTransportClient;

typedef enum _AppTransportMode
{
	ATM_SERVER = 0,
	ATM_CLIENT
} AppTransportMode;

typedef enum _AppTransportStatus
{
	ATS_INIT = 0,
	ATS_CONNECTED,
	ATS_INFO,
	ATS_WARNING,
	ATS_ERROR,
	ATS_ERROR_FATAL
} AppTransportStatus;

typedef struct _AppTransportPlugin AppTransportPlugin;

typedef struct _AppTransportPluginDefinition AppTransportPluginDefinition;

typedef struct _AppTransportPluginHelper
{
	AppTransport * apptransport;
	Event * event;
	/* callbacks */
	int (*status)(AppTransport * transport, AppTransportStatus status,
			unsigned int code, char const * message);
	/* clients */
	AppTransportClient * (*client_new)(AppTransport * transport);
	void (*client_delete)(AppTransport * transport,
			AppTransportClient * client);
	int (*client_receive)(AppTransport * transport,
			AppTransportClient * client, AppMessage * message);
} AppTransportPluginHelper;

struct _AppTransportPluginDefinition
{
	char const * name;
	char const * description;
	AppTransportPlugin * (*init)(AppTransportPluginHelper * helper,
			AppTransportMode mode, char const * name);
	void (*destroy)(AppTransportPlugin * transport);
	int (*send)(AppTransportPlugin * transport, AppMessage * message,
			int acknowledge);
};

#endif /* !LIBAPP_APP_APPTRANSPORT_H */
