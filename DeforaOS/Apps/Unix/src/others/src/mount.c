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



#include <sys/statvfs.h>
#if 0 /* defined(__FreeBSD__) || defined(__NetBSD__) */
# include <sys/param.h>
#endif
#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* mount */
/* types */
typedef struct _Prefs
{
	int flags;
	char * options;
	char * type;
} Prefs;
#define PREFS_a 0x1


/* functions */
/* mount */
static int _mount_all(Prefs * prefs);
static int _mount_print(void);
static int _mount_do(Prefs * prefs, char const * special, char const * node);

static int _mount(Prefs * prefs, char const * special, char const * node)
{
	if(special == NULL && node == NULL)
		return (prefs->flags & PREFS_a) == PREFS_a
			? _mount_all(prefs) : _mount_print();
	return _mount_do(prefs, special, node);
}

static int _mount_error(char const * message, int ret)
{
	fputs("mount: ", stderr);
	perror(message);
	return ret;
}

static int _mount_all(Prefs * prefs)
{
	errno = ENOSYS;
	return _mount_error("-a", 1);
}

static int _mount_print(void)
{
#ifdef ST_WAIT
	int ret = 0;
	int cnt;
	struct statvfs * f;
	int i;

	if((cnt = getvfsstat(NULL, 0, ST_WAIT)) < 0)
		return _mount_error("getvfsstat", 1);
	if((f = malloc(sizeof(*f) * cnt)) == NULL)
		return _mount_error("malloc", 1);
	if(getvfsstat(f, sizeof(*f) * cnt, ST_WAIT) != cnt)
	{
		free(f);
		return _mount_error("getvfsstat", 1);
	}
	for(i = 0; i < cnt; i++)
		printf("%s%s%s%s%s%s%lx%s", f[i].f_mntfromname, " on ",
				f[i].f_mntonname, " type ", f[i].f_fstypename,
				" (", f[i].f_flag, ")\n");
	free(f);
	return 0;
#else /* FIXME workaround when getvfsstat() is missing */
	errno = ENOSYS;
	perror("mount");
	return 1;
#endif
}

static int _mount_do(Prefs * prefs, char const * special, char const * node)
	/* FIXME handle flags */
{
#ifdef __NetBSD_Version__ /* NetBSD */
# if __NetBSD_Version__ >= 499000000
	if(mount(prefs->type, node, 0, NULL, 0) == 0)
# else
	if(mount(prefs->type, node, 0, NULL) == 0)
# endif
#endif
		return 0;
	return _mount_error(node, 1);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mount [-a][-t type]\n"
"       mount [-o options] special | node\n"
"       mount [-o options] special node\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "ao:t:")) != -1)
		switch(o)
		{
			case 'a':
				prefs.flags |= PREFS_a;
				break;
			case 'o':
				prefs.options = optarg;
				break;
			case 't':
				prefs.type = optarg;
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _mount(&prefs, NULL, NULL) == 0 ? 0 : 2;
	if(optind + 1 == argc)
		return _mount(&prefs, argv[optind], NULL) == 0 ? 0 : 2;
	if(optind + 2 == argc)
		return _mount(&prefs, argv[optind], argv[optind + 1]) == 0
			? 0 : 2;
	return _usage();
}
