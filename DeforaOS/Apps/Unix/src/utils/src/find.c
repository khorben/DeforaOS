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
/* TODO:
 * - check commands validity before starting?
 * - do all primaries apply to folders? (eg links) */



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
#define FIND_PREFS_H 1
#define FIND_PREFS_L 2


/* find */
/* types */
typedef enum _FindCmd
{
	FC_INVALID = -1,
	FC_NAME = 0,
	FC_NOGROUP,
	FC_NOUSER,
	FC_XDEV,
	FC_PRUNE,
	FC_PERM,
	FC_TYPE,
	FC_LINKS,
	FC_USER,
	FC_GROUP,
	FC_SIZE,
	FC_ATIME,
	FC_CTIME,
	FC_MTIME,
	FC_EXEC,
	FC_OK,
	FC_PRINT,
	FC_NEWER,
	FC_DEPTH
} FindCmd;
#define FC_LAST FC_DEPTH

static int _find_error(char const * message, int ret);
static int _find_error_user(char const * message, char const * error, int ret);
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

static int _find_error_user(char const * message, char const * error, int ret)
{
	fprintf(stderr, "%s%s: %s\n", "find: ", message, error);
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
	struct stat st;

	if(stat(pathname, &st) != 0) /* XXX TOCTOU, danger of infinite loop */
	{
		if(errno != ENOENT || *prefs & FIND_PREFS_L
				|| stat(pathname, &st) != 0)
			return _find_error(pathname, 1);
	}
	if(_do_cmd(prefs, pathname, &st, cmdc, cmdv) != 0)
		return 0;
	if(S_ISDIR(st.st_mode))
		return _do_dir(prefs, pathname, cmdc, cmdv);
	return 0;
}

/* do_cmd */
static FindCmd _cmd_enum(char const * cmd);
static int _cmd_check_arg(int * i, int cmdc, char const * cmd);

static int _do_cmd(Prefs * prefs, char const * pathname, struct stat * st,
		int cmdc, char * cmdv[])
{
	int ret = 0;
	int i;
	char const * filename;
	struct group * grp;
	struct passwd * pw;
	long int l;
	char * p;

	if((filename = strrchr(pathname, '/')) == NULL)
		filename = pathname;
	else
		filename++;
	for(i = 0; i < cmdc; i++)
		switch(_cmd_enum(cmdv[i]))
		{
			case FC_GROUP:
				if(_cmd_check_arg(&i, cmdc, cmdv[i]) != 0)
					return 1;
				if((grp = getgrnam(cmdv[i])) == NULL)
					/* FIXME handle numeric id */
					return _find_error_user(cmdv[i],
							"No such group", 1);
				if(st->st_gid != grp->gr_gid)
					ret = 1;
				break;
			case FC_NAME:
				if(_cmd_check_arg(&i, cmdc, cmdv[i]) != 0)
					return 1;
				if(fnmatch(cmdv[i], filename, 0) != 0)
					ret = 1;
				break;
			case FC_LINKS:
				if(_cmd_check_arg(&i, cmdc, cmdv[i]) != 0)
					return 1;
				l = strtol(cmdv[i], &p, 0);
				/* FIXME input validation */
				if(st->st_nlink != l)
					ret = 1;
				break;
			case FC_NOGROUP:
				if(getgrgid(st->st_gid) != NULL)
					ret = 1;
				break;
			case FC_NOUSER:
				if(getpwuid(st->st_uid) != NULL)
					ret = 1;
				break;
			case FC_PERM:
				if(_cmd_check_arg(&i, cmdc, cmdv[i]) != 0)
					return 1;
				/* FIXME handle "minus" prefix? */
				l = strtol(cmdv[i], &p, 0);
				/* FIXME input validation, mode as expression */
				if(((st->st_mode & 07777) & l) != l)
					ret = 1;
				break;
			case FC_PRINT:
				puts(pathname);
				break;
			case FC_PRUNE:
				if(S_ISDIR(st->st_mode))
					ret = 1;
				break;
			case FC_USER:
				if(_cmd_check_arg(&i, cmdc, cmdv[i]) != 0)
					return 1;
				if((pw = getpwnam(cmdv[i])) == NULL)
					/* FIXME handle numeric id */
					return _find_error_user(cmdv[i],
							"No such user", 1);
				if(st->st_uid != pw->pw_uid)
					return 1;
				break;
			case FC_ATIME:
			case FC_CTIME:
			case FC_DEPTH:
			case FC_EXEC:
			case FC_MTIME:
			case FC_NEWER:
			case FC_OK:
			case FC_SIZE:
			case FC_TYPE:
			case FC_XDEV:
				errno = ENOSYS; /* FIXME not implemented */
				return _find_error(cmdv[i], 1);
			case FC_INVALID:
				errno = EINVAL;
				return _find_error(cmdv[i], 1);
		}
	if(ret == 0 && i == cmdc)
		puts(pathname);
	if(S_ISDIR(st->st_mode))
		return 0;
	return ret;
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
	return FC_INVALID;
}

static int _cmd_check_arg(int * i, int cmdc, char const * cmd)
{
	if(++(*i) == cmdc)
	{
		errno = EINVAL; /* XXX */
		return _find_error(cmd, 1);
	}
	return 0;
}

static int _do_dir(Prefs * prefs, char const * pathname, int cmdc,
		char * cmdv[])
{
	int ret = 0;
	DIR * dir;
	struct dirent * de;
	size_t len;
	char * path;
	char * p;

	if((dir = opendir(pathname)) == NULL)
		return _find_error(pathname, 1);
	len = strlen(pathname) + 2;
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
	fputs("Usage: find [-H|-L] path... [expression...]\n"
"  -H	De-reference links unless dangling or in the command line\n"
"  -L	De-reference links always\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "HL")) != -1)
		switch(o)
		{
			case 'H':
				prefs &= ~FIND_PREFS_L;
				prefs |= FIND_PREFS_H;
				break;
			case 'L':
				prefs &= ~FIND_PREFS_H;
				prefs |= FIND_PREFS_L;
				break;
			default:
				return _usage();
		}
	if(argc - optind == 0)
		return _usage();
	return (_find(&prefs, argc - optind, &argv[optind]) == 0) ? 0 : 2;
}
