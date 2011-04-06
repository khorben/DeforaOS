/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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



#include <unistd.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "locker.h"


/* usage */
static int _usage(void)
{
	fputs("Usage: locker [-s]\n"
"  -s	Suspend automatically when locked\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int suspend = 0;
	Locker * locker;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "s")) != -1)
		switch(o)
		{
			case 's':
				suspend = 1;
				break;
			default:
				return _usage();
		}
	if((locker = locker_new(suspend)) == NULL)
		return 2;
	gtk_main();
	locker_delete(locker);
	return 0;
}
