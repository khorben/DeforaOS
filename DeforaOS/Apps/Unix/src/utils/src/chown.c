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
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


/* types */
typedef int Prefs;
#define CHOWN_PREFS_h 1
#define CHOWN_PREFS_R 2
#define CHOWN_PREFS_H 4
#define CHOWN_PREFS_L 8
#define CHOWN_PREFS_P 12


/* chown */
static int _chown_error(char * message, int ret);
static int _chown_owner(char * owner, uid_t * uid, gid_t * gid);
static int _chown_do_recursive(Prefs * prefs, uid_t uid, gid_t gid,
		char * file);
static int _chown_do(Prefs * prefs, uid_t uid, gid_t gid, char * file);
static int _chown(Prefs * prefs, char * owner, int argc, char * argv[])
{
	int ret = 0;
	uid_t uid = 0;
	gid_t gid = 0;
	int i;

	if(_chown_owner(owner, &uid, &gid) != 0)
		return 2;
	if(*prefs & CHOWN_PREFS_R)
	{
		for(i = 0; i < argc; i++)
			ret |= _chown_do_recursive(prefs, uid, gid, argv[i]);
		return ret;
	}
	for(i = 0; i < argc; i++)
		ret |= _chown_do(prefs, uid, gid, argv[i]);
	return ret;
}

static int _chown_error(char * message, int ret)
{
	fputs("chown: ", stderr);
	perror(message);
	return ret;
}

/* PRE
 * POST
 * 		0	success
 * 		else	failure */
static uid_t _chown_uid(char * owner);
static gid_t _chown_gid(char * group);
static int _chown_owner(char * owner, uid_t * uid, gid_t * gid)
{
	int i;

	for(i = 0; owner[i] != 0; i++)
		if(owner[i] == ':')
		{
			owner[i++] = '\0';
			break;
		}
	if((*uid = _chown_uid(owner)) == (uid_t)-1)
		return 1;
	if(owner[i] != '\0' && (*gid = _chown_gid(&owner[i])) == (gid_t)-1)
		return 1;
	return 0;
}

static int _chown_id_error(char * message, char * unknown, int ret);
static uid_t _chown_uid(char * owner)
{
	struct passwd * pwd;
	char * p;

	if((pwd = getpwnam(owner)) != NULL)
		return pwd->pw_uid;
	for(p = owner; *p != '\0' && isdigit(*p); p++);
	if(*p != '\0' || *owner == '\0')
		return _chown_id_error(owner, "user", -1);
	return strtol(owner, NULL, 10);
}

static int _chown_id_error(char * message, char * unknown, int ret)
{
	if(errno != 0)
		return _chown_error(message, ret);
	fprintf(stderr, "%s%s%s%s%s", "chown: ", message, ": Unknown ", unknown,
			"\n");
	return ret;
}

static gid_t _chown_gid(char * group)
{
	struct group * grp;
	char * p;

	if((grp = getgrnam(group)) != NULL)
		return grp->gr_gid;
	for(p = group; *p != '\0' && isdigit(*p); p++);
	if(*p != '\0' || *group == '\0')
		return _chown_id_error(group, "group", -1);
	return strtol(group, NULL, 10);
}

static int _chown_do_recursive_do(Prefs * p, uid_t uid, gid_t gid, char * file);
static int _chown_do_recursive(Prefs * p, uid_t uid, gid_t gid, char * file)
{
	struct stat st;

	if(lstat(file, &st) != 0)
		return _chown_error(file, 1);
	if(!S_ISDIR(st.st_mode))
		return _chown_do(p, uid, gid, file);
	if(!S_ISLNK(st.st_mode))
		return _chown_do_recursive_do(p, uid, gid, file);
	return 0;
}

static int _chown_do_recursive_do(Prefs * prefs, uid_t uid, gid_t gid,
		char * file)
{
	DIR * dir;
	struct dirent * de;
	int len;
	char * s;
	char * p;

	if((dir = opendir(file)) == NULL)
		return _chown_error(file, 1);
	readdir(dir);
	readdir(dir);
	len = strlen(file);
	len += (len && file[len-1] == '/') ? 1 : 2;
	if((s = malloc(len)) == NULL)
	{
		closedir(dir);
		return _chown_error(file, 1);
	}
	strcpy(s, file);
	s[len-2] = '/';
	s[len-1] = '\0';
	while((de = readdir(dir)) != NULL)
	{
		if((p = realloc(s, len + strlen(de->d_name))) == NULL)
		{
			_chown_error("malloc", 0);
			continue;
		}
		s = p;
		strcat(s, de->d_name);
		_chown_do_recursive(prefs, uid, gid, s);
		s[len-1] = '\0';
	}
	free(s);
	closedir(dir);
	return 0;
}

static int _chown_do(Prefs * prefs, uid_t uid, gid_t gid, char * file)
{
	int res;

	if((*prefs & CHOWN_PREFS_h) == CHOWN_PREFS_h)
		res = lchown(file, uid, gid);
	else
		res = chown(file, uid, gid);
	if(res != 0)
		return _chown_error(file, 1);
	return 2;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: chown [-hR] owner[:group] file ...\n\
       chown -R [-H | -L | -P] owner[:group] file ...\n\
  -h	Set the user and group IDs on symbolic links\n\
  -R	Recursively change file user and group IDs\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "hRHLP")) != -1)
		switch(o)
		{
			case 'h':
				prefs |= CHOWN_PREFS_h;
				break;
			case 'R':
				prefs |= CHOWN_PREFS_R;
				break;
			case 'H':
			case 'L':
			case 'P':
				fprintf(stderr, "%s%c%s", "chown: -", o,
						": Not yet implemented\n");
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	return (_chown(&prefs, argv[optind], argc - optind - 1,
			&argv[optind + 1]) == 0) ? 0 : 2;
}
