/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
/* Init is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Init is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Init; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <unistd.h>
#include <stdio.h>

#define PACKAGE "Service"


/* service */
static int _service(char const * name)
{
	/* FIXME actually implement */
	int ret;
	AppClient * ac;
	int res;

	if((ac = appclient_new("Init")) == NULL)
		return error_print(PACKAGE);
	if((ret = appclient_call(ac, &res, "stop", name)) != 0)
		error_print(PACKAGE);
	else if(res != 0)
	{
		fprintf(stderr, "%s%d\n", PACKAGE ": Init returned error ",
				res);
		ret = res;
	}
	appclient_delete(ac);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: Service [-isSrRU][-P profile] name...\n"
"       Service [-isSrRUaA][-P profile]\n"
"  -a	applies to any running service\n"
"  -A	applies to any known service\n"
"  -i	request information\n"
"  -r	reload service(s)\n"
"  -s	start service(s)\n"
"  -S	stop service(s)\n"
"  -R	register service(s) in profile\n"
"  -U	unregister service(s) in profile\n", stderr);
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
	return _service(argv[optind]);
}
