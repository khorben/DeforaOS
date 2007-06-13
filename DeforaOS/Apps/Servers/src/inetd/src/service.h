/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
/* inetd is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * inetd is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * inetd; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



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
