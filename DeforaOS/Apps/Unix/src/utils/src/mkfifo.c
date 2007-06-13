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



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define COMMON_MODE
#include "common.c"


/* mkfifo */
static int _mkfifo_error(char * message, int ret);
static int _mkfifo(mode_t mode, int argc, char * argv[])
{
	int ret = 0;
	int i;

	for(i = 0; i < argc; i++)
		if(mkfifo(argv[i], mode) == -1)
			ret+=_mkfifo_error(argv[i], 1);
	return ret;
}

static int _mkfifo_error(char * message, int ret)
{
	fputs("mkfifo: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mkfifo [-m mode] file...\n\
  -m	create fifo with the specified mode value\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int o;

	while((o = getopt(argc, argv, "m:")) != -1)
		switch(o)
		{
			case 'm':
				if(_mode(optarg, &mode) != 0)
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc == optind)
		return _usage();
	return _mkfifo(mode, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
