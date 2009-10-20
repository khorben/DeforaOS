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
#include <stdio.h>


/* link */
static int _link_error(char const * message, int ret);

static int _link(char * file1, char * file2)
{
	if(link(file1, file2) == -1)
		return _link_error(file2, 1);
	return 0;
}

static int _link_error(char const * message, int ret)
{
	fputs("link: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: link file1 file2\n", stderr);
	return 1;
}


/* main */
int main(int argc, char* argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc - 2)
		return _usage();
	return (_link(argv[optind], argv[optind + 1]) == 0) ? 0 : 2;
}
