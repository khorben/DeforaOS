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
#include <stdio.h>
#include <string.h>


/* types */
typedef enum _OutputDelay {
	OD_NONE,
	OD_BUFFER
} OutputDelay;


/* cat */
/* PRE
 * POST	the requested files have been printed on standard output
 * 	returns:
 * 		0	successful
 * 		2	an error occured */
static int _cat_error(char * message, int ret);
static void _cat_file(FILE * fp, OutputDelay od);
static int _cat(OutputDelay od, int argc, char * argv[])
{
	int ret = 0;
	int i;
	FILE * fp;

	if(argc == 0)
	{
		_cat_file(stdin, od);
		return 0;
	}
	for(i = 0; i < argc; i++)
	{
		if(strcmp("-", argv[i]) == 0)
			fp = stdin;
		else if((fp = fopen(argv[i], "r")) == NULL)
		{
			ret |= _cat_error(argv[i], 1);
			continue;
		}
		_cat_file(fp, od);
		if(fp != stdin && fclose(fp) != 0)
			ret |= _cat_error(argv[i], 1);
	}
	return ret;
}

static int _cat_error(char * message, int ret)
{
	fputs("cat: ", stderr);
	perror(message);
	return ret;
}

static int _write_nonbuf(int c);
static int _write_buf(int c);
static void _cat_file(FILE * fp, OutputDelay od)
{
	int c;
	int (*func)(int c);
	func = (od == OD_NONE ? _write_nonbuf : _write_buf);

	while((c = fgetc(fp)) != EOF)
		func(c);
}

static int _write_buf(int c)
{
	return fputc(c, stdout);
}

static int _write_nonbuf(int c)
{
	return write(1, &c, sizeof(char));
}


/* usage */
static int _usage(void)
{
	fputs("Usage: cat [-u][file ...]\n\
  -u	Write without delay\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	OutputDelay flagu = OD_BUFFER;

	while((o = getopt(argc, argv, "u")) != -1)
		switch(o)
		{
			case 'u':
				flagu = OD_NONE;
				break;
			default:
				return _usage();
		}
	return _cat(flagu, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
