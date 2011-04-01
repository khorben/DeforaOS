/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>


/* ps */
static int _ps_error(char const * message, int ret);

static int _ps(void)
{
	char const proc[] = "/proc";
	char buf[256];
	DIR * dir;
	struct dirent * de;
	long pid;
	char * p;
	FILE * fp;
	size_t s;

	if((dir = opendir(proc)) == NULL)
		return -_ps_error(proc, 1);
	printf("%5s %s\n", "PID", "CMD");
	while((de = readdir(dir)) != NULL)
	{
		pid = strtol(de->d_name, &p, 10);
		if(de->d_name[0] == '\0' || *p != '\0' || pid < 0)
			continue;
		snprintf(buf, sizeof(buf), "%s/%s/%s", proc, de->d_name,
				"cmdline");
		if((fp = fopen(buf, "r")) != NULL)
		{
			s = fread(buf, sizeof(*buf), sizeof(buf) - 1, fp);
			fclose(fp);
			buf[s] = '\0';
		}
		else
			buf[0] = '\0';
		printf("%5lu %s\n", pid, buf);
	}
	closedir(dir);
	return 0;
}

static int _ps_error(char const * message, int ret)
{
	fputs("ps: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ps\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_ps() == 0) ? 0 : 2;
}
