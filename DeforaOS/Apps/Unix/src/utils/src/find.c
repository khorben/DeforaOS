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
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_H 1
#define PREFS_L 2


/* find */
/* types */
typedef enum _FindCmd
{
	FC_NAME = 0, FC_NOGROUP, FC_NOUSER, FC_XDEV, FC_PRUNE, FC_PERM, FC_TYPE,
	FC_LINKS, FC_USER, FC_GROUP, FC_SIZE, FC_ATIME, FC_CTIME, FC_MTIME,
	FC_EXEC, FC_OK, FC_PRINT, FC_NEWER, FC_DEPTH
} FindCmd;
#define FC_LAST FC_DEPTH

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
static int _do_cmd(Prefs * prefs, char const * pathname, struct stat * st,
		int cmdc, char * cmdv[]);
static int _do_dir(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[]);

static int _find_do(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[])
{
	int ret;
	struct stat st;

	if(lstat(pathname, &st) != 0) /* XXX TOCTOU */
		return _find_error(pathname, 1);
	ret = _do_cmd(prefs, pathname, &st, cmdc, cmdv);
	if(S_ISDIR(st.st_mode))
		return _do_dir(prefs, pathname, cmdc, cmdv);
	else if(ret == 0)
		printf("%s\n", pathname);
	return ret;
}

/* do_cmd */
static FindCmd _cmd_enum(char const * cmd);

static int _do_cmd(Prefs * prefs, char const * pathname, struct stat * st,
		int cmdc, char * cmdv[])
{
	int i;
	char const * filename;

	if((filename = strrchr(pathname, '/')) == NULL)
		filename = pathname;
	else
		filename++;
	for(i = 0; i < cmdc; i++)
		switch(_cmd_enum(cmdv[i]))
		{
			case FC_NAME:
				if(++i == cmdc)
				{
					errno = EINVAL;
					return _find_error(cmdv[i], 1);
				}
				if(fnmatch(cmdv[i], filename, 0) != 0)
					return 1;
				break;
			case FC_NOGROUP:
				return getgrgid(st->st_gid) == NULL ? 0 : 1;
			case FC_NOUSER:
				return getpwuid(st->st_uid) == NULL ? 0 : 1;
			case -1:
				errno = EINVAL;
				return _find_error(cmdv[i], 1);
			default: /* FIXME not implemented */
				errno = ENOSYS;
				return _find_error(cmdv[i], 1);
		}
	return 0;
}

static FindCmd _cmd_enum(char const * cmd)
{
	const char * cmds[FC_LAST + 1] =
	{
		"-name",
		"-nouser",
		"-nogroup",
		"-xdev",
		"-prune",
		"-perm",
		"-type",
		"-links",
		"-user",
		"-group",
		"-size",
		"-atime",
		"-ctime",
		"-mtime",
		"-exec",
		"-ok",
		"-print",
		"-newer",
		"-depth"
	};
	int i;

	for(i = 0; i < FC_LAST + 1; i++)
		if(strcmp(cmd, cmds[i]) == 0)
			return i;
	return -1;
}

static int _do_dir(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[])
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
		ret |= _find_do(prefs, path, cmdc, cmdv);
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
