/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdio.h>
#include "cpp.h"


/* cpp */
static int _cpp_liberror(char const * message, int ret);

static int _cpp(char const * filename)
{
	int ret = 0;
	Cpp * cpp;
	ssize_t len;
	char buf[BUFSIZ];

	if((cpp = cpp_new()) == NULL)
		return _cpp_liberror("Internal error", 1);
	if(cpp_parse(cpp, filename) != 0)
		ret = _cpp_liberror(cpp_get_error(cpp), 1);
	else
	{
		while((len = cpp_read(cpp, buf, sizeof(buf))) > 0)
			fwrite(buf, sizeof(char), len, stdout);
		if(len < 0)
			ret = _cpp_liberror(cpp_get_error(cpp), 1);
	}
	cpp_delete(cpp);
	return ret;
}

static int _cpp_liberror(char const * message, int ret)
{
	fprintf(stderr, "%s%s", "cpp: ", message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: cpp filename\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(argc - optind != 1)
		return _usage();
	return _cpp(argv[optind]) == 0 ? 0 : 2;
}
