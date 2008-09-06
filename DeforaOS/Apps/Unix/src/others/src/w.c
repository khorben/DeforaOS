/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include "utmpx.c"


/* w */
static int _w_error(char * message, int ret);
static int _print_idle(struct timeval * tv, char * device);
static int _print_what(pid_t pid);

static int _w(void)
{
	struct utmpx * u;
	struct timeval tv;

	if(gettimeofday(&tv, NULL) != 0)
		return _w_error("gettimeofday", 2);
	printf("%-8s %-8s %-16s %-5s %5s %s\n", "USER", "TTY", "FROM",
			"LOGIN@", "IDLE", "WHAT");
	while((u = getutxent()) != NULL)
	{
		if(u->ut_type != USER_PROCESS)
			continue;
		printf("%-8s %-8s %-16s  %2ld:%02ld ", u->ut_user, u->ut_line,
				u->ut_host,
				(u->ut_tv.tv_sec - u->ut_tv.tv_sec % 3600) % 24,
				(u->ut_tv.tv_sec - u->ut_tv.tv_sec % 60) / 60
				% 60);
		_print_idle(&tv, u->ut_line);
		_print_what(u->ut_pid);
	}
	return 0;
}

static int _w_error(char * message, int ret)
{
	fputs("w: ", stderr);
	perror(message);
	return ret;
}

static int _print_idle(struct timeval * tv, char * device)
{
	char dev[16];
	struct stat st;
	unsigned long idle;

	if(snprintf(dev, sizeof(dev), "%s%s", "/dev/", device)
			> (int)sizeof(dev))
		return printf("%5s ", "?");
	if(lstat(dev, &st) != 0)
		return printf("%5s ", "?");
	idle = tv->tv_sec - st.st_atime;
	if(idle / 3600 >= 24)
		return printf("%4lud ", idle / 3600 / 24);
	else if(idle / 60 >= 60)
		return printf("%02lu:%02lu ", idle / 3600, idle % 3600 / 60);
	return printf("%02lum%02lu ", idle / 60, idle % 60);
}

static int _print_what(pid_t pid)
{
	char proc[33];
	FILE * fp;
	int len;

	if(pid < 0)
		return putchar('\n');
	if(snprintf(proc, sizeof(proc), "%s%d%s", "/proc/", pid, "/cmdline")
			> (int)sizeof(proc))
		return printf(" %d\n", pid);
	if((fp = fopen(proc, "r")) == NULL)
	{
		printf(" %d\n", pid);
		return _w_error(proc, 0);
	}
	if((len = fread(proc, sizeof(char), sizeof(proc) - 1, fp)) < 0)
	{
		fclose(fp);
		printf(" %d\n", pid);
		return _w_error(proc, 0);
	}
	proc[len] = '\0';
	printf("%s\n", proc);
	return fclose(fp);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: w\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		return _usage();
	return _w() == 0 ? 0 : 2;
}
