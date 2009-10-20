/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* utils is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * utils is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with utils; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* kill */
static int _kill(int sig, int argc, char * argv[])
{
	int ret = 0;
	int i;
	pid_t pid;
	char * p;

	for(i = 0; i < argc; i++)
	{
		pid = strtol(argv[i], &p, 10);
		if(argv[i][0] == '\0' || *p != '\0')
		{
			fprintf(stderr, "%s%s%s", "kill: ", argv[i],
					": Invalid process number\n");
			continue;
		}
		if(kill(pid, sig) != 0)
		{
			fputs("kill: ", stderr);
			perror("kill");
			ret |= 1;
		}
	}
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: kill -s signal_name pid...\n\
       kill -l [exit_status]\n\
  -l	Write all signal values supported\n\
  -s	Specify the signal to send\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int sig = SIGTERM;
	int o;
	char * p;

	while((o = getopt(argc, argv, "ls:")) != -1)
	{
		switch(o)
		{
			case 'l':
				fprintf(stderr, "%s%c%s", "kill: -", o,
						": Not yet implemented\n");
				return _usage();
			case 's':
				/* FIXME signal_name expected, NaN... */
				sig = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	}
	if(optind == argc)
		return _usage();
	return (_kill(sig, argc - optind, &argv[optind]) == 0) ? 0 : 2;
}
