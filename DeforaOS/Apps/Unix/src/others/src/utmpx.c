/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifdef __FreeBSD__
# include <utmp.h>
#else
# include <utmpx.h>
#endif


#ifndef _PATH_UTMPX
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

/* getutxent */
struct utmpx * getutxent(void)
	/* FIXME implement */
{
	static FILE * fp = NULL;
	static struct utmpx ut;
	struct utmp buf;

	if(fp == NULL && (fp = fopen(_PATH_UTMP, "r")) == NULL)
		return NULL; /* FIXME report error */
	if(fread(&buf, sizeof(buf), 1, fp) != 1)
		return NULL;
	memcpy(ut.ut_name, buf.ut_name, sizeof(ut.ut_name));
	memcpy(ut.ut_line, buf.ut_line, sizeof(ut.ut_line));
	memcpy(ut.ut_host, buf.ut_host, sizeof(ut.ut_host));
	ut.ut_type = buf.ut_name[0] == '\0' ? EMPTY :  USER_PROCESS;
	ut.ut_pid = -1;
	ut.ut_tv.tv_sec = buf.ut_time;
	ut.ut_tv.tv_usec = 0;
	return &ut;
}
#endif
