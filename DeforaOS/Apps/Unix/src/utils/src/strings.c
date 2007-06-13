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
#include <stdlib.h>
#include <stdio.h>



/* strings */
static int _strings_error(char const * message, int ret);
static void _strings_do(int flgn, FILE * fp);
static int _strings(int flgn, int argc, char * argv[])
{
	FILE * fp;
	int i;

	if(argc == 0)
	{
		_strings_do(flgn, stdin);
		return 0;
	}
	for(i = 0; i < argc; i++)
	{
		if((fp = fopen(argv[i], "r")) == NULL)
			return _strings_error(argv[i], 1);
		_strings_do(flgn, fp);
		if(fclose(fp) != 0)
			return _strings_error(argv[i], 1);
	}
	return 0;
}

static int _strings_error(char const * message, int ret)
{
	fputs("strings: ", stderr);
	perror(message);
	return ret;
}

static void _strings_do(int flgn, FILE * fp)
{
	int c;
	char * str;
	char * p;
	int i = 0;
	int n;

	str = malloc(flgn * sizeof(char));
	n = flgn;
	while((c = fgetc(fp)) != EOF)
	{
		if(isprint(c))
		{
			if(i + 1 >= n)
			{
				if((p = realloc(str, (i + 2) * sizeof(char)))
						== NULL)
				{
					_strings_error("malloc", 0);
					return;
				}
				str = p;
				n = i + 2;
			}
			str[i++] = c;
		}
		else if(i >= flgn && (c == '\n' || c == '\0'))
		{
			str[i] = '\0';
			printf("%s\n", str);
			i = 0;
		}
		else
			i = 0;
	}
	free(str);
}


/* usage */
static int _usage(void)
{
	/* FIXME */
	fputs("Usage: strings [-a][-t format][-n number][file...]\n\
  -a	\n\
  -t	\n\
  -n	\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int flgn = 4;
	char * p;

	while((o = getopt(argc, argv, "at:n:")) != -1)
		switch(o)
		{
			case 'a':
				break;
			case 't':
				break;
			case 'n':
				flgn = strtol(optarg, &p, 10);
				if(*(optarg) == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	return _strings(flgn, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
