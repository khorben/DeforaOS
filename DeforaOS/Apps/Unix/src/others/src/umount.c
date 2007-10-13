/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix others */
/* others is not free software; you can redistribute it and/or modify it under
 * the terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * others is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with others; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/mount.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* umount */
/* types */
typedef int Prefs;
#define PREFS_a 0x1


/* functions */
/* umount */
static int _umount_all(void);
static int _umount_do(char const * pathname);

static int _umount(Prefs prefs, int pathc, char * pathv[])
{
	int ret = 0;
	int i;

	if(prefs & PREFS_a && pathc == 0)
		return _umount_all();
	for(i = 0; i < pathc; i++)
		ret |= _umount_do(pathv[i]);
	return ret;
}

static int _umount_error(char const * message, int ret)
{
	fputs("umount: ", stderr);
	perror(message);
	return ret;
}

#ifdef ST_WAIT
static int _umount_all(void)
{
	int ret = 0;
	int cnt;
	struct statvfs * f;
	int i;

	if((cnt = getvfsstat(NULL, 0, ST_WAIT)) < 0)
		return _umount_error("getvfsstat", 1);
	if((f = malloc(sizeof(*f) * cnt)) == NULL)
		return _umount_error("malloc", 1);
	if(getvfsstat(f, sizeof(*f) * cnt, ST_WAIT) != cnt)
	{
		free(f);
		return _umount_error("getvfsstat", 1);
	}
	for(i = cnt - 1; i >= 0; i--)
		if(strcmp("/", f[i].f_mntonname) == 0)
			continue;
		else
			ret |= _umount_do(f[i].f_mntonname);
	free(f);
	return ret;
#else /* FIXME workaround when getvfsstat() is missing */
# include <errno.h>
static int _umount_all(void)
{
	errno = ENOSYS;
	perror("umount");
	return 1;
#endif
}

static int _umount_do(char const * pathname)
{
	if(unmount(pathname, 0) == 0)
		return 0;
	return _umount_error(pathname, 1);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: umount -a\n"
"       umount special | node ...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	prefs = 0;
	while((o = getopt(argc, argv, "a")) != -1)
		switch(o)
		{
			case 'a':
				prefs |= PREFS_a;
				break;
			default:
				return _usage();
		}
	if(optind == argc && (prefs & PREFS_a) != PREFS_a)
		return _usage();
	return _umount(prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
