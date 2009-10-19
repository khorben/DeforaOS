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



#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef enum _OutputType
{
	OT_NONE,
	OT_LONG,
	OT_DEFAULT
} OutputType;


/* cmp */
static int _cmp_error(char const * message, int ret);
static int _cmp_files(OutputType ot, char const * file1, char const * file2,
		FILE * fp1, FILE * fp2);
static int _cmp(OutputType ot, char const * file1, char const * file2)
{
	FILE * fp1;
	FILE * fp2;

	if(strcmp("-", file1) == 0)
		fp1 = stdin;
	else if((fp1 = fopen(file1, "r")) == NULL)
		return _cmp_error(file1, 1);
	if(strcmp("-", file2) == 0)
		fp2 = stdin;
	else if((fp2 = fopen(file2, "r")) == NULL)
	{
		_cmp_error(file2, 0);
		if(fp1 != stdin && fclose(fp1) != 0)
			return _cmp_error(file1, 1);
		return 1;
	}
	return _cmp_files(ot, file1, file2, fp1, fp2);
}

static int _cmp_error(char const * message, int ret)
{
	fputs("cmp: ", stderr);
	perror(message);
	return ret;
}

static int _cmp_files(OutputType ot, char const * file1, char const * file2,
		FILE * fp1, FILE * fp2)
{
	int ret = 0;
	int c1;
	int c2;
	unsigned int byte = 1;
	unsigned int line = 1;

	while(1)
	{
		c1 = fgetc(fp1);
		c2 = fgetc(fp2);
		if(c1 == EOF && c2 == EOF)
			break;
		if(c1 == EOF || c2 == EOF)
		{
			if(ot != OT_NONE)
				fprintf(stderr, "%s%s\n", "cmp: EOF on ",
						c1 == EOF ? file1 : file2);
			ret = 1;
			break;
		}
		if(c1 != c2)
		{
			ret = 1;
			if(ot == OT_DEFAULT)
			{
				printf("%s %s differ: char %u, line %u\n",
						file1, file2, byte, line);
				break;
			}
			else if(ot == OT_LONG)
				printf("%d %o %o\n", byte, c1, c2);
			else
				break;
		}
		if(c1 == '\n')
			line++;
		byte++;
	}
	if(fp1 != stdin && fclose(fp1) != 0)
		ret |= _cmp_error(file1, 1);
	if(fp2 != stdin && fclose(fp2) != 0)
		ret |= _cmp_error(file2, 1);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: cmp [-l|-s] file1 file2\n\
  -l	Write the byte number and the differing byte for each difference\n\
  -s	Write nothing for differing bytes\n", stderr);
	return 2;
}


/* main */
int main(int argc, char * argv[])
{
	OutputType ot = OT_DEFAULT;
	int o;

	while((o = getopt(argc, argv, "ls")) != -1)
		switch(o)
		{
			case 'l':
				ot = OT_LONG;
				break;
			case 's':
				ot = OT_NONE;
				break;
			default:
				return _usage();
		}
	if(argc - optind != 2)
		return _usage();
	return (_cmp(ot, argv[optind], argv[optind + 1]) == 0) ? 0 : 2;
}
