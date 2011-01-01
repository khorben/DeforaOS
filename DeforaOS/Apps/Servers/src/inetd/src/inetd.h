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



#ifndef __INETD_H
# define __INETD_H

# include <sys/select.h>
# include "config.h"


/* types */
typedef struct _InetdState
{
	int debug;
	int queue;
	char const * filename;
	Config * config;
	fd_set rfds;
	int fdmax;
} InetdState;


/* variables */
extern InetdState * inetd_state;


/* functions */
int inetd_error(char const * message, int ret);

#endif /* !__INETD_H */
