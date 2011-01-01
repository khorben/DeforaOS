/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
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



#ifndef __SERVICE_H
# define __SERVICE_H

# include <sys/types.h>
# include <sys/socket.h>
# include <stdint.h>
# include <netinet/in.h>


/* Service */
/* types */
typedef enum _ServiceSocket { SS_STREAM, SS_DGRAM } ServiceSocket;
typedef enum _ServiceProtocol { SP_TCP, SP_UDP } ServiceProtocol;
typedef enum _ServiceWait { SW_WAIT, SW_NOWAIT } ServiceWait;
typedef struct _ServiceId
{
	uid_t uid;
	gid_t gid;
} ServiceId;

typedef struct _Service
{
	char * name;
	ServiceSocket socket;
	ServiceProtocol proto;
	ServiceWait wait;
	ServiceId id;
	char ** program;
	int fd;
	unsigned short port;
	pid_t pid;
} Service;


/* functions */
Service * service_new(char * name, ServiceSocket socket, ServiceProtocol proto,
		ServiceWait wait, ServiceId id, char ** program);
void service_delete(Service * service);

/* useful */
int service_listen(Service * service);
int service_exec(Service * service);

#endif /* !__SERVICE_H */
