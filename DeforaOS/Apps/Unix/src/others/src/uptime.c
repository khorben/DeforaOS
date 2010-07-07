/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "utmpx.c"


/* uptime */
static int _uptime_error(char const * message, int ret);

static int _uptime(void)
{
	struct timeval tv;
	time_t sec;
	struct tm * tm = NULL;
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
#ifdef USER_PROCESS
	for(nusers = 0; (ut = getutxent()) != NULL;)
		if(ut->ut_type == USER_PROCESS)
			nusers++;
#else
# warning Unsupported platform: USER_PROCESS is not supported
	nusers = 0;
#endif
	if(getloadavg(loadavg, 3) != 3)
		return _uptime_error("getloadavg", 1);
	printf(" %s up %s, %2d%s%.2f, %.2f, %.2f\n", time, "unknown", nusers,
			" users, load average: ", loadavg[0], loadavg[1],
			loadavg[2]);
	return 0;
}

static int _uptime_error(char const * message, int ret)
{
	fputs("uptime: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: uptime\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		return _usage();
	if(argc != 1)
		return _usage();
	return _uptime() == 0 ? 0 : 2;
}
