/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* mktemp */
static int _mktemp_error(char * message, int ret);
static int _mktemp(char * template)
{
	int fd;
	struct timeval tv;

	if((template = strdup(template)) == NULL)
		return _mktemp_error("strdup", 1);
	if(gettimeofday(&tv, NULL) != 0)
		return _mktemp_error("gettimeofday", 1);
	srand(tv.tv_sec ^ tv.tv_usec ^ getuid() ^ (getpid() << 16));
	if((fd = mkstemp(template)) == -1)
		return _mktemp_error(template, 1);
	if(close(fd) != 0)
		return _mktemp_error(template, 1);
	printf("%s\n", template);
	free(template);
	return 0;
}

static int _mktemp_error(char * message, int ret)
{
	fputs("mktemp: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mktemp [-d] [template]\n\
  -d	make a directory instead of a file\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * template = "/tmp/tmp.XXXXXX";

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			case '?':
				return _usage();
		}
	if(optind < argc - 1)
		return _usage();
	if(optind == argc - 1)
		return _mktemp(argv[optind]);
	return _mktemp(template) == 0 ? 0 : 2;
}
