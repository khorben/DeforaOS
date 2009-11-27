/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
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



#include <System.h>
#include <unistd.h>
#include <stdio.h>
#include "init.h"
#include "../config.h"

/* hack to define the interface without colliding with the Init class */
/* FIXME see later if we really need it
#include "init.c" */


/* private */
static int _init(char const * profile);
static int _usage(void);


/* functions */
/* private */
/* init */
static int _init(char const * profile)
{
	int ret = 0;
	Init * init;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, profile);
#endif
	if((init = init_new(profile)) == NULL)
		return error_print(PACKAGE);
	for(;;)
	{
		if(init_loop(init) != 0)
			ret |= error_print(PACKAGE);
		/* do not suck resources */
		sleep(1);
	}
	init_delete(init);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: " PACKAGE " -s [-P profile]\n"
"  -P	Profile to load\n"
"  -s	Force the single-user profile\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int ret;
	int o;
	char const * profile = NULL;
	char * shell[] = { "/bin/sh", NULL };

	while((o = getopt(argc, argv, "P:s")) != -1)
		switch(o)
		{
			case 'P':
				profile = optarg;
				break;
			case 's':
				profile = "single-user";
				break;
			default:
				return _usage();
		}
	if((ret = _init(profile) != 0) && getpid() == 1)
	{
		fputs(PACKAGE ": Spawning a shell\n", stderr);
		execve(shell[0], shell, NULL);
		return 127;
	}
	return ret;
}
