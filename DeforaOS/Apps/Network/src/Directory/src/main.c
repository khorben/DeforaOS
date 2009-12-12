/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of Directory */
/* Directory is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Directory is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Directory; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <unistd.h>
#include <stdio.h>
#include "directory.h"
#include "../config.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: " PACKAGE " [-L|-R]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	AppServerOptions options = ASO_LOCAL;
	Event * event;
	Directory * directory;

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
	if((event = event_new()) == NULL)
		return error_print(PACKAGE) ? 2 : 2;
	if((directory = directory_new(options, event)) == NULL)
	{
		error_print(PACKAGE);
		event_delete(event);
		return 2;
	}
	event_loop(event);
	event_delete(event);
	return 0;
}
