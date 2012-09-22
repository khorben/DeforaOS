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
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


/* common_socket_set_nodelay */
int common_socket_set_nodelay(int fd, int nodelay)
{
	int ret = -1;
#ifdef TCP_NODELAY
	int optval = nodelay ? 1 : 0;

	ret = setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &optval, sizeof(optval));
# ifdef DEBUG
	if(ret != 0)
		fprintf(stderr, "%s%s", "libApp: TCP_NODELAY: ",
				strerror(errno));
# endif
	return (ret == 0) ? 0 : -1;
#else
	errno = ENOSYS;
	return ret;
#endif
}
