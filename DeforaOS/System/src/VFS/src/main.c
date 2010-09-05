/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System VFS */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "vfs.h"
#include "../config.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: " PACKAGE " [-L|-R][-r root][-u umask]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	AppServerOptions options = ASO_LOCAL;
	char * root = "/";
	mode_t mask;
	int mask_set = 0;
	char * p;

	while((o = getopt(argc, argv, "LRr:u:")) != -1)
		switch(o)
		{
			case 'L':
				options = ASO_LOCAL;
				break;
			case 'R':
				options = ASO_REMOTE;
				break;
			case 'r':
				root = optarg;
				break;
			case 'u':
				mask = strtol(optarg, &p, 0);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				mask_set = 1;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	if(mask_set == 0)
	{
		mask = umask(0);
		umask(mask);
	}
	return (vfs(options, mask, root) == 0) ? 0 : 2;
}
