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



#ifndef __INETD_H
# define __INETD_H

# include <sys/select.h>
# include "config.h"


/* types */
typedef struct _InetdState {
	int debug;
	int queue;
	char * filename;
	Config * config;
	fd_set rfds;
	int fdmax;
} InetdState;


/* variables */
extern InetdState * inetd_state;


/* functions */
int inetd_error(char const * message, int ret);

#endif /* !__INETD_H */
