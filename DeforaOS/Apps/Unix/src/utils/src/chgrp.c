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



#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* chgrp */
/* types */
typedef int Prefs;
#define CHGRP_PREFS_h 1
#define CHGRP_PREFS_R 2
#define CHGRP_PREFS_H 4
#define CHGRP_PREFS_L 8
#define CHGRP_PREFS_P 12


/* functions */
/* chgrp */
static int _chgrp_grp_error(char * group);
static int _chgrp_do_recursive(int opts, gid_t gid, char * file);
static int _chgrp_do(int opts, gid_t gid, char * file);

static int _chgrp(Prefs opts, char * group, int argc, char * argv[])
{
	struct group * grp;
	int res = 0;
	int i;

	if((grp = getgrnam(group)) == NULL)
		return _chgrp_grp_error(group);
	if((opts & CHGRP_PREFS_R) == CHGRP_PREFS_R)
	{
		for(i = 0; i < argc; i++)
			res += _chgrp_do_recursive(opts, grp->gr_gid, argv[i]);
		return res;
	}
	for(i = 0; i < argc; i++)
		res += _chgrp_do(opts, grp->gr_gid, argv[i]);
	return res;
}

static int _chgrp_error(char * message, int ret);
static int _chgrp_grp_error(char * group)
{
	if(errno == 0)
	{
		fprintf(stderr, "%s%s%s", "chgrp: ", group,
				": Unknown group\n");
		return 2;
	}
	return _chgrp_error(group, 2);
}

static int _chgrp_error(char * message, int ret)
{
	fputs("chgrp: ", stderr);
	perror(message);
	return ret;
}

static int _chgrp_do_recursive_do(int opts, gid_t gid, char * file);
static int _chgrp_do_recursive(int opts, gid_t gid, char * file)
{
	struct stat st;

	if((lstat(file, &st)) != 0)
		return _chgrp_error(file, 1);
	if(!S_ISDIR(st.st_mode))
		return _chgrp_do(opts, gid, file);
	if(!S_ISLNK(st.st_mode))
		return _chgrp_do_recursive_do(opts, gid, file);
	return 0;
}

static int _chgrp_do_recursive_do(int opts, gid_t gid, char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _chgrp_error(file, 1);
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len - 1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		closedir(dir);
		return _chgrp_error(file, 1);
	}
	strcpy(s, file);
	s[len - 2] = '/';
	s[len - 1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			_chgrp_error("malloc", 0);
			continue;
		}
		s = p;
		strcat(s, de->d_name);
		_chgrp_do_recursive(opts, gid, s);
		s[len - 1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
}

static int _chgrp_do(int opts, gid_t gid, char * file)
{
	int res;

	if((opts & CHGRP_PREFS_h) == CHGRP_PREFS_h)
		res = lchown(file, -1, gid);
	else
		res = chown(file, -1, gid);
	if(res != 0)
		return _chgrp_error(file, 1);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: chgrp [-hR] group file ...\n\
       chgrp -R [-H | -L | -P] group file ...\n\
  -h    Set the group IDs on symbolic links\n\
  -R    Recursively change file group IDs\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs opts = 0;

	while((o = getopt(argc, argv, "hRHLP")) != -1)
	{
		switch(o)
		{
			case 'h':
				opts |= CHGRP_PREFS_h;
				break;
			case 'R':
				opts |= CHGRP_PREFS_R;
				break;
			case 'H':
			case 'L':
			case 'P':
				/* FIXME implement */
				fprintf(stderr, "%s%c%s", "chgrp: -", o,
						": Not yet implemented\n");
			default:
				return _usage();
		}
	}
	if(argc - optind < 2)
		return _usage();
	return _chgrp(opts, argv[optind], argc - optind - 1,
			&argv[optind + 1]);
}
