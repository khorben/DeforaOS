/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Graphics GServer */



#include <unistd.h>
#include <stdio.h>
#include <System.h>
#include "gserver.h"


/* functions */
/* usage */
static int _usage(void)
{
	fputs("Usage: GServer [-L|-R]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	AppServerOptions options = ASO_LOCAL;
	GServer * gserver;

	while((o = getopt(argc, argv, "LR")) != -1)
		switch(o)
		{
			case 'L':
				options = ASO_LOCAL;
				break;
			case 'R':
				options = ASO_REMOTE;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if((gserver = gserver_new(options, NULL)) == NULL)
		return error_print("GServer");
	while(gserver_loop(gserver) == 0);
	gserver_delete(gserver);
	return 0;
}
