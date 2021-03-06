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



#ifndef LIBSOCKET_ARPA_INET_H
# define LIBSOCKET_ARPA_INET_H

# include <netinet/in.h>


/* types */
#ifndef uint16_t
# define uint16_t uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
# define uint32_t uint32_t
typedef unsigned int uint32_t;
#endif


/* macros */
# define htonl(host32)	htonl(host32)
# define htons(host16)	htons(host16)
# define ntohl(net32)	ntohl(net32)
# define ntohs(net16)	ntohs(net16)


/* functions */
uint32_t htonl(uint32_t host32);
uint16_t htons(uint16_t host16);
in_addr_t inet_addr(const char *cp);
char * inet_ntoa(struct in_addr in);
uint32_t ntohl(uint32_t net32);
uint16_t ntohs(uint16_t net16);

#endif /* !LIBSOCKET_ARPA_INET_H */
