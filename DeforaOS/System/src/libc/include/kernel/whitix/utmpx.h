/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBC_KERNEL_WHITIX_UTMPX_H
# define LIBC_KERNEL_WHITIX_UTMPX_H


/* types */
# ifndef id_t
#  define id_t id_t
typedef unsigned int id_t;
# endif
# ifndef pid_t
#  define pid_t pid_t
typedef signed int pid_t;
# endif
# ifndef suseconds_t
#  define suseconds_t suseconds_t
typedef int suseconds_t;
# endif
#ifndef timeval
# define timeval timeval
struct timeval
{
	time_t tv_sec;
	suseconds_t tv_usec;
};
#endif

# define _UTMPX_HOST_SIZE	256
# define _UTMPX_ID_SIZE		4
# define _UTMPX_LINE_SIZE	32
# define _UTMPX_USER_SIZE	32
struct utmpx
{
	short int ut_type;
	pid_t ut_pid;
	char ut_line[_UTMPX_LINE_SIZE];
	char ut_id[_UTMPX_ID_SIZE];
	char ut_user[_UTMPX_USER_SIZE];
	char ut_host[_UTMPX_HOST_SIZE];
	char _padding1[4]; /* FIXME struct exit_status */
	long int _padding2;
	struct timeval ut_tv;
	char _padding3[36];
};

#endif /* !LIBC_KERNEL_WHITIX_UTMPX_H */
