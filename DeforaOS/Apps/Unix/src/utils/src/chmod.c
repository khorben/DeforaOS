/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix utils */
/* utils is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * utils is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with utils; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define COMMON_MODE
#include "common.c"


/* types */
typedef int Prefs;
#define CHMOD_PREFS_R 1


/* chmod */
static int _chmod_error(char const * message, int ret);
static int _chmod_do(mode_t mode, char * file);
static int _chmod_do_recursive(Prefs prefs, mode_t mode, char * file);

static int _chmod(Prefs prefs, mode_t mode, int filec, char * filev[])
{
	int ret = 0;
	int i;

	for(i = 0; i < filec; i++)
		if(prefs & CHMOD_PREFS_R)
			ret += _chmod_do_recursive(prefs, mode, filev[i]);
		else
			ret += _chmod_do(mode, filev[i]);
	return (ret == 0) ? 0 : 2;
}

static int _chmod_error(char const * message, int ret)
{
	fputs("chmod: ", stderr);
	perror(message);
	return ret;
}

static int _chmod_do(mode_t mode, char * file)
{
	if(chmod(file, mode) != 0)
		return _chmod_error(file, 1);
	return 0;
}

static int _chmod_do_recursive_do(Prefs prefs, mode_t mode, char * file);
static int _chmod_do_recursive(Prefs prefs, mode_t mode, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0)
		return _chmod_error(file, 1);
	if(!S_ISDIR(st.st_mode))
		return _chmod_do(mode, file);
	if(!S_ISLNK(st.st_mode))
		return _chmod_do_recursive_do(prefs, mode, file);
	return 0;
}

static int _chmod_do_recursive_do(Prefs prefs, mode_t mode, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _chmod_error(file, 1);
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len - 1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		closedir(dir);
		return _chmod_error(file, 1);
	}
	strcpy(s, file);
	s[len - 2] = '/';
	s[len - 1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			_chmod_error("malloc", 0);
			continue;
		}
		s = p;
		strcat(s, de->d_name);
		_chmod_do_recursive(prefs, mode, s);
		s[len - 1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: chmod [-R] mode file\n\
  -R	Recursively change file mode bits\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	mode_t mode = 0;
	int o;

	while((o = getopt(argc, argv, "R")) != -1)
		switch(o)
		{
			case 'R':
				prefs = CHMOD_PREFS_R;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	if(_mode(argv[optind], &mode) != 0)
		return _usage();
	return _chmod(prefs, mode, argc - optind - 1, &argv[optind + 1]);
}
