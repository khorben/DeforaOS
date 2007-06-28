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



#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <utmpx.h>


/* uptime */
static int _uptime_error(char const * message, int ret);
static int _uptime(void)
{
	struct timeval tv;
	time_t sec;
	struct tm * tm;
	char time[9];
	unsigned int nusers;
	struct utmpx * ut;
	double loadavg[3];

	if((gettimeofday(&tv, NULL)) != 0)
		return _uptime_error("gettimeofday", 1);
	sec = tv.tv_sec;
	if((tm = gmtime(&sec)) == NULL)
		return _uptime_error("gmtime", 1);
	if(strftime(time, sizeof(time), "%X", tm) == 0)
		return _uptime_error("strftime", 1);
	/* FIXME uptime is not portable afaik */
	for(nusers = 0; (ut = getutxent()) != NULL;)
		if(ut->ut_type == USER_PROCESS)
			nusers++;
	if(getloadavg(loadavg, 3) != 3)
		return _uptime_error("getloadavg", 1);
	printf(" %s up %3d%s%.2f, %.2f, %.2f\n", time, nusers,
			" users, load average: ", loadavg[0], loadavg[1],
			loadavg[2]);
	return 0;
}

static int _uptime_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "uptime: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: uptime\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _uptime() == 0 ? 0 : 2;
}
