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



#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define COMMON_MODE
#include "common.c"


/* mkdir */
static int _mkdir_error(char const * message, int ret);
static int _mkdir_p(mode_t mode, char * pathname);

static int _mkdir(int flagp, mode_t mode, int filec, char * filev[])
{
	int ret = 0;
	int i;

	for(i = 0; i < filec; i++)
		if(flagp == 1)
		{
			if(_mkdir_p(mode, filev[i]) != 0)
				ret = 2;
		}
		else if(mkdir(filev[i], mode) != 0)
			ret = _mkdir_error(filev[i], 2);
	return ret;
}

static int _mkdir_error(char const * message, int ret)
{
	fputs("mkdir: ", stderr);
	perror(message);
	return ret;
}

static int _mkdir_p(mode_t mode, char * pathname)
{
	char * p;
	struct stat st;

	if(pathname[0] == '\0')
		return 1;
	for(p = &pathname[1]; *p != '\0'; p++)
	{
		if(*p != '/')
			continue;
		*p = '\0';
		if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
				&& mkdir(pathname, mode) == -1)
			return _mkdir_error(pathname, 1);
		for(*p++ = '/'; *p == '/'; p++);
		if(*p == '\0')
			return 0;
	}
	if(!(stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
			&& mkdir(pathname, mode) == -1)
		return _mkdir_error(pathname, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mkdir [-p][-m mode] dir...\n\
  -p	Create any missing intermediate pathname components\n\
  -m	File permission bits of the newly-created directory\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	mode_t mode = 0777;
	int flagp = 0;
	int o;

	while((o = getopt(argc, argv, "pm:")) != -1)
		switch(o)
		{
			case 'm':
				if(_mode(optarg, &mode) != 0)
					return _usage();
				break;
			case 'p':
				flagp = 1;
				break;
			default:
				return _usage();
		}
	if(argc == optind)
		return _usage();
	return _mkdir(flagp, mode, argc - optind, &argv[optind]);
}
