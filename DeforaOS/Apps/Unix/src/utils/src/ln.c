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
/* TODO
 * - implement and use _ln_error() */



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


/* types */
/* force link */
typedef enum _LinkForce {
	LF_NO,
	LF_YES
} LinkForce;

/* link type */
typedef enum _LinkType {
	LT_HARD = 0,
	LT_SOFT
} LinkType;


/* ln
 * PRE	lf	whether to unlink destination if already exists or not
 * 	lt	whether to link or symlink
 * 	argc	number of source + destination arguments
 * POST
 * 	0	success
 * 	2	error */
static int _ln_error(char const * message, int ret);
static int _ln_is_directory(char * dest);
static int _ln_single(LinkForce lf, LinkType lt, char const * src,
		char const * dest);
static int _ln_multiple(LinkForce lf, LinkType lt, int argc, char * argv[]);

static int _ln(LinkForce lf, LinkType lt, int argc, char * argv[])
{
	if(argc == 2 && !_ln_is_directory(argv[1]))
		return _ln_single(lf, lt, argv[0], argv[1]);
	return _ln_multiple(lf, lt, argc, argv);
}

static int _ln_error(char const * message, int ret)
{
	fputs("ln: ", stderr);
	perror(message);
	return ret;
}

static int _ln_is_directory(char * dest)
{
	struct stat buf;

	if(stat(dest, &buf) == -1 || !S_ISDIR(buf.st_mode))
		return 0;
	return 1;
}

static int _ln_single(LinkForce lf, LinkType lt, char const * src,
		char const * dest)
{
	if(lf == LF_YES)
		unlink(dest);
	if((lt == LT_HARD ? link(src, dest) : symlink(src, dest)) == -1)
		return _ln_error(dest, 1);
	return 0;
}

static int _ln_multiple(LinkForce lf, LinkType lt, int argc, char * argv[])
{
	int i;
	ssize_t len;
	char * dest = NULL;
	char * p;

	for(i = 0; i < argc - 1; i++)
	{
		len = strlen(argv[argc - 1]) + strlen(argv[i]) + 2;
		if((p = realloc(dest, len)) == NULL)
		{
			_ln_error(argv[i], 0);
			continue;
		}
		dest = p;
		sprintf(dest, "%s/%s", argv[argc - 1], argv[i]);
		_ln_single(lf, lt, argv[i], dest);
	}
	free(dest);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: ln [-fs] source_file target_file\n\
       ln [-fs] source_file ... target_dir\n\
  -f	Force existing destination pathnames to be removed\n\
  -s	Create symbolic links instead of hard links\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	LinkForce flgf = LF_NO;
	LinkType flgs = LT_HARD;
	int o;

	while((o = getopt(argc, argv, "fs")) != -1)
		switch(o)
		{
			case 'f':
				flgf = LF_YES;
				break;
			case 's':
				flgs = LT_SOFT;
				break;
			default:
				return _usage();
		}
	if(argc - optind <= 1)
		return _usage();
	return _ln(flgf, flgs, argc - optind, &argv[optind]);
}
