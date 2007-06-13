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



#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define COMMON_MODE
#include "common.c"


/* mkdir */
static int _mkdir_error(char * message, int ret);
static int _mkdir_p(mode_t mode, char * pathname);
static int _mkdir(int flagp, mode_t mode, int argc, char * argv[])
{
	int res = 0;
	int i;

	for(i = 0; i < argc; i++)
	{
		if(flagp == 1)
		{
			if(_mkdir_p(mode, argv[i]) != 0)
				res = 2;
		}
		else if(mkdir(argv[i], mode) != 0)
			res = _mkdir_error(argv[i], 2);
	}
	return res;
}

static int _mkdir_error(char * message, int ret)
{
	fputs("mkdir: ", stderr);
	perror(message);
	return ret;
}

static int _mkdir_p(mode_t mode, char * pathname)
{
	char * p;
	struct stat st;

	if(pathname[0] == '\0')
		return 1;
	for(p = &pathname[1]; *p != '\0'; p++)
	{
		if(*p != '/')
			continue;
		*p = '\0';
		if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
				&& mkdir(pathname, mode) == -1)
			return _mkdir_error(pathname, 1);
		for(*p++ = '/'; *p == '/'; p++);
		if(*p == '\0')
			return 0;
	}
	if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
			&& mkdir(pathname, mode) == -1)
		return _mkdir_error(pathname, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mkdir [-p][-m mode] dir...\n\
  -p    create any missing intermediate pathname components\n\
  -m    file permission bits of the newly-created directory\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int flagp = 0;
	int o;

	while((o = getopt(argc, argv, "pm:")) != -1)
		switch(o)
		{
			case 'm':
				if(_mode(optarg, &mode) != 0)
					return _usage();
				break;
			case 'p':
				flagp = 1;
				break;
			default:
				return _usage();
		}
	if(argc == optind)
		return _usage();
	return _mkdir(flagp, mode, argc - optind, &argv[optind]);
}
