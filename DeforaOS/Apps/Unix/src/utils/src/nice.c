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



#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/* nice */
static int _nice_error(char const * message, int ret);

static int _nice(int nice, char * argv[])
{
	if(setpriority(PRIO_PROCESS, 0, nice) != 0)
		return _nice_error("Unable to set priority", 1);
	execvp(argv[0], argv);
	return _nice_error(argv[0], 1);
}

static int _nice_error(char const * message, int ret)
{
	fputs("nice: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: nice [-n increment] utility [argument...]\n\
  -n	Priority to set\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int nice = 0;
	int o;
	char * p;

	while((o = getopt(argc, argv, "n:")) != -1)
		switch(o)
		{
			case 'n':
				nice = strtol(optarg, &p, 10);
				if(*optarg == '\0' || *p != '\0')
					return _usage();
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	return (_nice(nice, &argv[optind]) == 0) ? 0 : 2;
}
