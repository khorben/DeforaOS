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


/* cpp */
static int _cpp_error(char const * message, int ret);
static int _cpp(char const * filename)
{
	FILE * fp;
	char buf[BUFSIZ];
	size_t len;
	int ret = 0;

	if((fp = fopen(filename, "r")) == NULL)
		return _cpp_error(filename, 1);
	while((len = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
		fwrite(buf, sizeof(char), len, stdout);
	if(len == 0 && !feof(fp))
		ret = _cpp_error(filename, 1);
	fclose(fp);
	return ret;
}

static int _cpp_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "cpp: ");
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: cpp filename\n");
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
