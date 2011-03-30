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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>


/* types */
typedef enum _pwd_flag
{
	PWD_P = 1,
	PWD_LP = 2
} pwd_flag;


/* pwd */
static int _pwd_error(char const * message, int ret);

static int _pwd(pwd_flag pf)
{
	char * pwd;
#ifdef PATH_MAX
	char buf[PATH_MAX];
#else
	char buf[1024];
#endif
	char * p;

	pwd = getenv("PWD");
	if(pf == PWD_LP && pwd != NULL && *pwd == '/')
	{
		for(p = pwd; *p != '\0'; p++)
		{
			if(*p != '/')
				continue;
			if(*++p != '.')
				continue;
			if(*++p == '\0' || *p == '/')
			{
				p--;
				break;
			}
			if(*p != '.')
				continue;
			if(*++p == '\0' || *p == '/')
			{
				p--;
				break;
			}
		}
		if(*p == '\0')
		{
			printf("%s\n", pwd);
			return 0;
		}
	}
	if(pwd != NULL)
	{
		/* FIXME check for symlinks */
	}
	else
	{
		if(getcwd(buf, sizeof(buf) - 1) == NULL)
			return _pwd_error("getcwd", 1);
		buf[sizeof(buf) - 1] = '\0';
		pwd = buf;
	}
	printf("%s\n", pwd);
	return 0;
}

static int _pwd_error(char const * message, int ret)
{
	fputs("pwd: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: pwd [-L|-P]\n\
  -L	Avoid \".\" or \"..\" filenames\n\
  -P	Avoid symbolic links\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	pwd_flag pf = PWD_LP;
	int o;

	while((o = getopt(argc, argv, "LP")) != -1)
		switch(o)
		{
			case 'L':
				pf = PWD_LP;
				break;
			case 'P':
				pf = PWD_P;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _pwd(pf) == 0 ? 0 : 2;
}
