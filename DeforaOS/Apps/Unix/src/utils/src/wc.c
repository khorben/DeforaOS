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
#include <ctype.h>
#include <stdio.h>


/* macros */
#define IS_SET(flags, bit) ((flags & bit) == bit)


/* types */
typedef enum _wc_flag
{
	WF_ALL = 0,
	WF_C = 1,
	WF_M = 2,
	WF_L = 4,
	WF_W = 8
} wc_flag;


/* wc */
static int _wc_error(char const * message, int ret);
static int _wc_do(int flags, unsigned int * cm, unsigned int * l,
		unsigned int * w, char const * filename);
static void _wc_print(int flags, unsigned int cm, unsigned int l,
		unsigned int w, char const * filename);

static int _wc(int flags, int argc, char * argv[])
{
	int ret = 0;
	unsigned int cm = 0;
	unsigned int l = 0;
	unsigned int w = 0;
	int i;

	if(argc == 0)
		return _wc_do(flags, &cm, &l, &w, NULL);
	if(argc == 1)
		return _wc_do(flags, &cm, &l, &w, argv[0]);
	for(i = 0; i < argc; i++)
		ret |= _wc_do(flags, &cm, &l, &w, argv[i]);
	_wc_print(flags, cm, l, w, "total");
	return ret;
}

static int _wc_error(char const * message, int ret)
{
	fputs("wc: ", stderr);
	perror(message);
	return ret;
}

static int _wc_do(int flags, unsigned int * cm, unsigned int * l,
		unsigned int * w, char const * filename)
{
	FILE * fp;
	unsigned int lcm = 0;
	unsigned int ll = 0;
	unsigned int lw = 0;
	int c;
	int oldc = ' ';

	if(filename == NULL)
		fp = stdin;
	else if((fp = fopen(filename, "r")) == NULL)
		return _wc_error(filename, 1);
	while((c = fgetc(fp)) != EOF)
	{
		if(c == '\n')
			ll++;
		if(isspace(oldc) && isalnum(c))
			lw++;
		oldc = c;
		lcm++; /* FIXME */
	}
	_wc_print(flags, lcm, ll, lw, filename);
	if(filename != NULL && fclose(fp) != 0)
		return _wc_error(filename, 1);
	*cm += lcm;
	*l += ll;
	*w += lw;
	return 0;
}

static void _wc_print(int flags, unsigned int cm, unsigned int l,
		unsigned int w, char const * filename)
{
	if(flags == WF_ALL)
		printf("%d %d %d", l, w, cm);
	if(IS_SET(flags, WF_C) || IS_SET(flags, WF_M))
		printf("%d", cm);
	if(IS_SET(flags, WF_L))
		printf("%s%d", (IS_SET(flags, WF_C) || IS_SET(flags, WF_M))
				? " " : "", l);
	if(IS_SET(flags, WF_W))
		printf("%s%d", flags != WF_W ? " " : "", w);
	if(filename != NULL)
		printf(" %s", filename);
	fputc('\n', stdout);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: wc [-c|-m][-lw][file...]\n\
  -c	Write the number of bytes\n\
  -m	Write the number of characters\n\
  -l	Write the number of lines\n\
  -w	Write the number of words\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int flags = WF_ALL;

	while((o = getopt(argc, argv, "cmlw")) != -1)
		switch(o)
		{
			case 'c':
				if(IS_SET(flags, WF_M))
					flags = flags - WF_M;
				flags = flags | WF_C;
				break;
			case 'm':
				if(IS_SET(flags, WF_C))
					flags = flags - WF_C;
				flags = flags | WF_M;
				break;
			case 'l':
				flags = flags | WF_L;
				break;
			case 'w':
				flags = flags | WF_W;
				break;
			default:
				return _usage();
		}
	return _wc(flags, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
