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



#include <sys/utsname.h>
#include <unistd.h>
#include <stdio.h>


/* uname */
static int _uname(int m, int n, int r, int s, int v)
{
	struct utsname buf;
	int spacing = 0;

	if(uname(&buf) != 0)
	{
		perror("uname");
		return 1;
	}
	if(s && (spacing = 1))
		printf("%s", buf.sysname);
	if(n)
		printf("%s%s", spacing++ ? " " : "", buf.nodename);
	if(r)
		printf("%s%s", spacing++ ? " " : "", buf.release);
	if(v)
		printf("%s%s", spacing++ ? " " : "", buf.version);
	if(m)
		printf("%s%s", spacing++ ? " " : "", buf.machine);
	if(spacing == 0)
		printf("%s", buf.sysname);
	putchar('\n');
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: uname [-snrvma]\n\
  -s    operating system name\n\
  -n    name of this node on the network\n\
  -r    operating system release name\n\
  -v    operating system version\n\
  -m    hardware type\n\
  -a    all the options above\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int flagm = 0;
	int flagn = 0;
	int flagr = 0;
	int flags = 0;
	int flagv = 0;
	int o;

	while((o = getopt(argc, argv, "amnrsv")) != -1)
		switch(o)
		{
			case 'a':
				flagm = 1;
				flagn = 1;
				flagr = 1;
				flags = 1;
				flagv = 1;
				break;
			case 'm':
				flagm = 1;
				break;
			case 'n':
				flagn = 1;
				break;
			case 'r':
				flagr = 1;
				break;
			case 's':
				flags = 1;
				break;
			case 'v':
				flagv = 1;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _uname(flagm, flagn, flagr, flags, flagv) == 0 ? 0 : 2;
}
