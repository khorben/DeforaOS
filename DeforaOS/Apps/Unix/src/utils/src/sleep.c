/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
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
#include <stdlib.h>
#include <stdio.h>


/* sleep */
static int _sleep(unsigned int time)
{
	sleep(time);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: sleep time\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	long time;
	char * p;

	if(argc != 2)
		return _usage();
	time = strtol(argv[1], &p, 10);
	if(argv[1][0] == '\0' || *p != '\0' || time < 0)
		return _usage();
	return _sleep(time);
}
