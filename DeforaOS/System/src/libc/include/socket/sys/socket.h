/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
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



#ifndef LIBSOCKET_SYS_SOCKET_H
# define LIBSOCKET_SYS_SOCKET_H

# include <sys/uio.h>
# include "compat/sys/socket.h"


/* types */
# ifndef socklen_t
#  define socklen_t socklen_t
typedef int socklen_t;
# endif

#ifndef sockaddr_storage
# define sockaddr_storage sockaddr_storage
typedef struct _sockaddr_storage
{
	sa_family_t ss_family;
} sockaddr_storage;
# endif


/* constants */
# define SOCK_STREAM	1
# define SOCK_DGRAM	2
# define SOCK_RAW	3
# define SOCK_SEQPACKET	5

# define SO_ACCEPTCONN	0x0002
# define SO_BROADCAST	0x0020
# define SO_DEBUG	0x0001
# define SO_DONTROUTE	0x0010
# define SO_ERROR	0x1007
# define SO_KEEPALIVE	0x0008
# define SO_LINGER	0x0080
# define SO_OOBINLINE	0x0100
# define SO_RCVBUF	0x1002
# define SO_RCVLOWAT	0x1004
# define SO_RCVTIMEO	0x1006
# define SO_REUSEADDR	0x0004
# define SO_SNDBUF	0x1001
# define SO_SNDLOWAT	0x1003
# define SO_SNDTIMEO	0x1005
# define SO_TYPE	0x1008

# define AF_UNSPEC	0
# define AF_UNIX	1
# define AF_INET	2
# define AF_INET6	24


/* functions */
int accept(int fd, struct sockaddr * addr, socklen_t * len);
int bind(int fd, const struct sockaddr * addr, socklen_t len);
int connect(int fd, const struct sockaddr * addr, socklen_t len);
int getsockname(int fd, struct sockaddr * addr, socklen_t * len);
int getsockopt(int fd, int level, int name, const void * value,
		socklen_t * len);
int listen(int fd, int backlog);
ssize_t recv(int fd, void * buf, size_t len, int flags);
ssize_t recvfrom(int fd, void * buf, size_t len, int flags,
		struct sockaddr * addr, socklen_t * addrlen);
ssize_t send(int fd, const void * buf, size_t len, int flags);
ssize_t sendto(int fd, const void * buf, size_t len, int flags,
		struct sockaddr * addr, socklen_t addrlen);
int setsockopt(int fd, int level, int name, const void * value,
		socklen_t len);
int shutdown(int fd, int how);
int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int fds[2]);

#endif /* !LIBSOCKET_SYS_SOCKET_H */
