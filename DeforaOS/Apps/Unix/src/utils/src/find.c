/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* types */
typedef int Prefs;
#define PREFS_H 1
#define PREFS_L 2


/* find */
static int _find_error(char const * message, int ret);
static int _find_do(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[]);

static int _find(Prefs * prefs, int argc, char * argv[])
{
	int ret = 0;
	int i;
	int filec;

	for(i = 0; i < argc; i++)
		if(argv[i][0] == '-' || argv[i][0] == '!' || argv[i][0] == '(')
			break;
	filec = i;
	for(i = 0; i < filec; i++)
		ret |= _find_do(prefs, argv[i], argc - filec, &argv[filec]);
	return ret;
}

static int _find_error(char const * message, int ret)
{
	fputs("find: ", stderr);
	perror(message);
	return ret;
}

/* find_do */
static int _do_dir(Prefs * prefs, char const * pathname);

static int _find_do(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[])
{
	struct stat st;

	if(lstat(pathname, &st) != 0) /* XXX TOCTOU */
		return _find_error(pathname, 1);
	if(S_ISDIR(st.st_mode))
		return _do_dir(prefs, pathname);
	else
		printf("%s\n", pathname);
	return 0;
}

static int _do_dir(Prefs * prefs, char const * pathname)
{
	int ret = 0;
	DIR * dir;
	struct dirent * de;
	size_t len = strlen(pathname) + 2;
	char * path;
	char * p;

	if((dir = opendir(pathname)) == NULL)
		return _find_error(pathname, 1);
	if((path = malloc(len)) == NULL)
	{
		closedir(dir);
		return _find_error(pathname, 1);
	}
	sprintf(path, "%s/", pathname);
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(path, len + strlen(de->d_name))) == NULL)
			break;
		path = p;
		strcpy(&path[len - 1], de->d_name);
		ret |= _find_do(prefs, path, 0, NULL); /* FIXME commands */
	}
	free(path);
	if(de != NULL)
		ret |= _find_error(path, 1);
	if(closedir(dir) != 0)
		ret |= _find_error(path, 1);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: find\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "HL")) != -1)
		switch(o)
		{
			case 'H':
				prefs &= ~PREFS_L;
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs &= ~PREFS_H;
				prefs |= PREFS_L;
				break;
			default:
				return _usage();
		}
	if(argc - optind == 0)
		return _usage();
	return _find(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
