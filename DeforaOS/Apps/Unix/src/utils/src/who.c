/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifndef __FreeBSD__
# include <utmpx.h>
#endif


/* types */
typedef int Prefs;
#define PREFS_m 0x1
#define PREFS_T 0x2


/* who */
static int _who_error(char const * message, int ret);
static char * _who_tty(void);

static int _who(Prefs * prefs)
{
	struct utmpx * u;
	struct tm * tm;
	char buf[13];
	char * tty = NULL;
	time_t t;

	memset(&tm, 0, sizeof(tm));
	if(*prefs & PREFS_m && (tty = _who_tty()) == NULL)
		return 1;
#ifdef USER_PROCESS
	for(; (u = getutxent()) != NULL;)
	{
		if(u->ut_type != USER_PROCESS)
			continue;
		if(tty != NULL && strcmp(tty, u->ut_line) != 0)
			continue;
		printf("%-8s", u->ut_user);
		if(*prefs & PREFS_T)
			printf(" %c", '?');
		printf(" %-8s", u->ut_line);
		t = u->ut_tv.tv_sec;
		if((tm = localtime(&t)) == NULL
				|| strftime(buf, sizeof(buf), "%b %e %H:%M", tm)
				== 0)
			strcpy(buf, "n/a");
		printf(" %s\n", buf);
	}
#else
# warning Unsupported platform: USER_PROCESS is not supported
#endif
	return 0;
}

static int _who_error(char const * message, int ret)
{
	fputs("who: ", stderr);
	perror(message);
	return ret;
}

static char * _who_tty(void)
{
	char * tty;

	if((tty = ttyname(0)) == NULL)
	{
		_who_error("ttyname", 1);
		return NULL;
	}
	if(strncmp(tty, "/dev/", 5) != 0)
		return tty;
	return tty + 5;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: who [-mT]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "mTu")) != -1)
		switch(o)
		{
			case 'm':
				prefs |= PREFS_m;
				break;
			case 'T':
				prefs |= PREFS_T;
				break;
			default:
				return _usage();
		}
	return _who(&prefs) == 0 ? 0 : 2;
}
