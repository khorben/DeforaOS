/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* renice */
static int _renice_error(char * message, int ret);
static int _renice(int nice, int type, int argc, char * argv[])
{
	int i;
	int ret = 0;
	int id;
	char * p;

	for(i = 0; i < argc; i++)
	{
		id = strtol(argv[i], &p, 10);
		if(argv[i][0] == '\0' || *p != '\0')
		{
			fprintf(stderr, "%s%s%s", "renice: ", argv[i],
					"Invalid ID\n");
			ret = 2;
			continue;
		}
		if(setpriority(type, id, nice) != 0)
			ret = _renice_error(argv[i], 2);
	}
	return ret;
}

static int _renice_error(char * message, int ret)
{
	fputs("renice: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: renice -n increment [-g | -p | -u] ID...\n\
  -n	Priority to set\n\
  -g	Process group IDs\n\
  -p	Integer process IDs\n\
  -u	User IDs\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int nice = 0;
	int type = PRIO_PROCESS;
	int o;
	char * p;

	while((o = getopt(argc, argv, "n:gpu")) != -1)
		switch(o)
		{
			case 'n':
				nice = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			case 'g':
				type = PRIO_PGRP;
				break;
			case 'p':
				type = PRIO_PROCESS;
				break;
			case 'u':
				type = PRIO_USER;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	return _renice(nice, type, argc - optind, &argv[optind]);
}
