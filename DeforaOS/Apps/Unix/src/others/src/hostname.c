/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
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
#include <string.h>


/* hostname */
static int _hostname_error(char const * message, int ret);

static int _hostname(char const * name)
{
	char buf[128];

	if(name != NULL)
	{
		if(sethostname(name, strlen(name)) != 0)
			return _hostname_error("sethostname", 1);
		return 0;
	}
	if(gethostname(buf, sizeof(buf)) != 0)
		return _hostname_error("gethostname", 1);
	puts(buf);
	return 0;
}


/* hostname_error */
static int _hostname_error(char const * message, int ret)
{
	fputs("hostname: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: hostname [name]\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * name = NULL;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind + 1 == argc)
		name = argv[optind];
	else if(optind != argc)
		return _usage();
	return (_hostname(name) == 0) ? 0 : 2;
}
