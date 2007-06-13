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



#include <libgen.h>
#include <stdio.h>
#include <string.h>


/* basename */
static int _basename(char * arg, char * suf)
{
	char * str;
	int slen;
	int alen;

	str = basename(arg);
	if(suf != NULL)
	{
		slen = strlen(str);
		alen = strlen(suf);
		if(alen < slen && strcmp(suf, &str[slen - alen]) == 0)
			str[slen - alen] = '\0';
	}
	printf("%s\n", str);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: basename string [suffix]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc == 2)
		return _basename(argv[1], NULL);
	if(argc == 3)
		return _basename(argv[1], argv[2]);
	return _usage();
}
