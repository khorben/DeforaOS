/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
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



#ifdef __FreeBSD__
# include <utmp.h>
#else
# include <utmpx.h>
#endif


#if !defined(_PATH_UTMPX) && !defined(UT_NAMESIZE) && !defined(EMPTY)
# include <sys/time.h>
# include <string.h>

/* types */
struct utmpx
{
	char ut_name[UT_NAMESIZE];
	char ut_line[UT_LINESIZE];
	char ut_host[UT_HOSTSIZE];
	int ut_type;
	pid_t ut_pid;
	struct timeval ut_tv;
};
# define ut_user ut_name

/* constants */
#define EMPTY		0
#define USER_PROCESS	1


/* macros */
#ifndef min
# define min(a, b) ((a) < (b) ? (a) : (b))
#endif


/* functions */
/* getutxent */
struct utmpx * getutxent(void)
	/* FIXME implement */
{
	static FILE * fp = NULL;
	static struct utmpx utx;
	struct utmp ut;

	if(fp == NULL && (fp = fopen(_PATH_UTMP, "r")) == NULL)
		return NULL; /* FIXME report error */
	if(fread(&ut, sizeof(ut), 1, fp) != 1)
		return NULL;
	memcpy(utx.ut_name, ut.ut_name, min(sizeof(utx.ut_name),
				sizeof(ut.ut_name)));
	memcpy(utx.ut_line, ut.ut_line, min(sizeof(utx.ut_line),
				sizeof(ut.ut_line)));
	memcpy(utx.ut_host, ut.ut_host, min(sizeof(utx.ut_host),
				sizeof(ut.ut_host)));
	utx.ut_type = (ut.ut_name[0] == '\0') ? EMPTY :  USER_PROCESS;
	utx.ut_pid = -1;
	utx.ut_tv.tv_sec = ut.ut_time;
	utx.ut_tv.tv_usec = 0;
	return &utx;
}
#endif
