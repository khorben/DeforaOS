/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include "../syscalls.h"
#include "sys/time.h"


/* functions */
/* getitimer */
#ifndef SYS_getitimer
# warning Unsupported platform: getitimer() is missing
# include "errno.h"
int getitimer(int which, struct itimerval * value)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* gettimeofday */
#ifndef SYS_gettimeofday
# warning Unsupported platform: gettimeofday() is missing
# include "errno.h"
int gettimeofday(struct timeval * tv, void * null)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* setitimer */
#ifndef SYS_setitimer
# warning Unsupported platform: setitimer() is missing
# include "errno.h"
int setitimer(int which, const struct itimerval * value,
		struct itimerval * ovalue)
{
	errno = ENOSYS;
	return -1;
}
#endif


/* utimes */
#ifndef SYS_utimes
# warning Unsupported platform: utimes() is missing
#endif
