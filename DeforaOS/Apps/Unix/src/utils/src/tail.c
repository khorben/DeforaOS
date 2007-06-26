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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


/* tail */
/* types */
typedef struct _Prefs
{
	int flags;
	int bytes;
	int lines;
} Prefs;
#define PREFS_f	0x1

static int _tail_error(char const * message, int ret);
static int _tail_do(Prefs * prefs, FILE * fp, char const * filename);

static int _tail(Prefs * prefs, char const * filename)
{
	int ret;
	FILE * fp = stdin;

	if(filename != NULL && (fp = fopen(filename, "r")) == NULL)
		return _tail_error(filename, 1);
	ret = _tail_do(prefs, fp, filename != NULL ? filename : "stdin");
	if(filename != NULL && fclose(fp) != 0)
		return _tail_error(filename, 1);
	return ret;
}

static int _tail_error(char const * message, int ret)
{
	fputs("tail: ", stderr);
	perror(message);
	return ret;
}

static int _tail_do(Prefs * prefs, FILE * fp, char const * filename)
{
	/* FIXME implement */
	return 1;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: tail [-f][-c number|-n number][file]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "fc:n:")) != -1)
		switch(o)
		{
			case 'f':
				prefs.flags |= PREFS_f;
				break;
			case 'c':
				prefs.bytes = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'n':
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(optind != argc && optind + 1 != argc)
		return _usage();
	return _tail(&prefs, argv[optind]) ? 0 : 2;
}
