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
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x1
#define PREFS_i 0x2


/* mv */
static int _mv_error(char const * message, int ret);
static int _mv_single(Prefs * prefs, char const * src, char const * dst);
static int _mv_multiple(Prefs * prefs, int filec, char * const filev[]);

static int _mv(Prefs * prefs, int filec, char * filev[])
{
	struct stat st;

	if(stat(filev[filec - 1], &st) != 0)
	{
		if(errno != ENOENT)
			return _mv_error(filev[filec - 1], 1);
		if(filec > 2)
		{
			errno = ENOTDIR;
			return _mv_error(filev[filec - 1], 1);
		}
	}
	else if(S_ISDIR(st.st_mode))
		return _mv_multiple(prefs, filec, filev);
	else if(filec > 2)
	{
		errno = ENOTDIR;
		return _mv_error(filev[filec - 1], 1);
	}
	return _mv_single(prefs, filev[0], filev[1]);
}

static int _mv_error(char const * message, int ret)
{
	fputs("mv: ", stderr);
	perror(message);
	return ret;
}

static int _mv_confirm(char const * dst)
{
	int c;
	int tmp;

	fprintf(stderr, "%s%s%s", "mv: ", dst, ": Overwrite? ");
	if((c = fgetc(stdin)) == EOF)
		return 0;
	while(c != '\n' && (tmp = fgetc(stdin)) != EOF && tmp != '\n');
	return c == 'y';
}

/* mv_single */
static int _single_dir(char const * src, char const * dst);
static int _single_fifo(char const * src, char const * dst);
static int _single_nod(char const * src, char const * dst, mode_t mode,
		dev_t rdev);
static int _single_symlink(char const * src, char const * dst);
static int _single_regular(char const * src, char const * dst);
static int _single_p(char const * dst, struct stat const * st);

static int _mv_single(Prefs * prefs, char const * src, char const * dst)
{
	int ret;
	struct stat st;

	if(lstat(src, &st) != 0 && errno == ENOENT) /* XXX TOCTOU */
		return _mv_error(src, 1);
	if(*prefs & PREFS_i
			&& (lstat(dst, &st) == 0 || errno != ENOENT)
			&& _mv_confirm(dst) != 1)
		return 0;
	if(rename(src, dst) == 0)
		return 0;
	if(errno != EXDEV)
		return _mv_error(src, 1);
	if(unlink(dst) != 0
			&& errno != ENOENT)
		return _mv_error(dst, 1);
	if(lstat(src, &st) != 0)
		return _mv_error(dst, 1);
	if(S_ISDIR(st.st_mode))
		ret = _single_dir(src, dst);
	else if(S_ISFIFO(st.st_mode))
		ret = _single_fifo(src, dst);
	else if(S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode))
		ret = _single_nod(src, dst, st.st_mode, st.st_rdev);
	else if(S_ISLNK(st.st_mode))
		ret = _single_symlink(src, dst);
	else if(!S_ISREG(st.st_mode)) /* FIXME not implemented */
	{
		errno = ENOSYS;
		return _mv_error(src, 1);
	}
	else
		ret = _single_regular(src, dst);
	if(ret != 0)
		return ret;
	_single_p(dst, &st);
	return 0;
}

static int _single_dir(char const * src, char const * dst)
{
	if(mkdir(dst, 0777) != 0)
		return _mv_error(dst, 1);
	if(rmdir(src) != 0) /* FIXME probably gonna fail, recurse before */
		_mv_error(src, 0);
	return 0;
}

static int _single_fifo(char const * src, char const * dst)
{
	if(mkfifo(dst, 0666) != 0)
		return _mv_error(dst, 1);
	if(unlink(src) != 0)
		_mv_error(src, 0);
	return 0;
}

static int _single_nod(char const * src, char const * dst, mode_t mode,
		dev_t rdev)
{
	if(mknod(dst, mode, rdev) != 0)
		return _mv_error(dst, 1);
	if(unlink(src) != 0)
		_mv_error(src, 0);
	return 0;
}

static int _single_symlink(char const * src, char const * dst)
{
	char buf[PATH_MAX];
	ssize_t i;

	if((i = readlink(src, buf, sizeof(buf) - 1)) == -1)
		return _mv_error(src, 1);
	buf[i] = '\0';
	if(symlink(buf, dst) != 0)
		return _mv_error(dst, 1);
	if(unlink(src) != 0)
		_mv_error(src, 0);
	return 0;
}

static int _single_regular(char const * src, char const * dst)
{
	int ret = 0;
	FILE * fsrc;
	FILE * fdst;
	char buf[BUFSIZ];
	size_t size;

	if((fsrc = fopen(src, "r")) == NULL)
		return _mv_error(dst, 1);
	if((fdst = fopen(dst, "w")) == NULL)
	{
		ret |= _mv_error(dst, 1);
		fclose(fsrc);
		return ret;
	}
	while((size = fread(buf, sizeof(char), sizeof(buf), fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != size)
			break;
	if(!feof(fsrc))
		ret |= _mv_error(size == 0 ? src : dst, 1);
	if(fclose(fsrc) != 0)
		ret |= _mv_error(src, 1);
	if(fclose(fdst) != 0)
		ret |= _mv_error(dst, 1);
	if(unlink(src) != 0)
		_mv_error(src, 0);
	return ret;
}

static int _single_p(char const * dst, struct stat const * st)
{
	struct timeval tv[2];

	if(lchown(dst, st->st_uid, st->st_gid) != 0) /* XXX TOCTOU */
	{
		_mv_error(dst, 0);
		if(chmod(dst, st->st_mode & ~(S_ISUID | S_ISGID)) != 0)
			_mv_error(dst, 0);
	}
	else if(chmod(dst, st->st_mode) != 0)
		_mv_error(dst, 0);
	tv[0].tv_sec = st->st_atime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = st->st_mtime;
	tv[1].tv_usec = 0;
	if(utimes(dst, tv) != 0)
		_mv_error(dst, 0);
	return 0;
}


/* mv_multiple */
static int _mv_multiple(Prefs * prefs, int filec, char * const filev[])
{
	int ret = 0;
	int i;
	char * dst;
	size_t len;
	char * sdst = NULL;
	char * p;

	for(i = 0; i < filec - 1; i++)
	{
		dst = basename(filev[i]);
		len = strlen(filev[i]) + strlen(dst) + 2;
		if((p = realloc(sdst, len * sizeof(char))) == NULL)
		{
			ret |= _mv_error(filev[filec - 1], 1);
			continue;
		}
		sdst = p;
		sprintf(sdst, "%s/%s", filev[filec - 1], dst);
		ret |= _mv_single(prefs, filev[i], sdst);
	}
	free(sdst);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: mv [-fi] source_file target_file\n\
       mv [-fi] source_file... target_file\n\
  -f	Do not prompt for confirmation if the destination path exists\n\
  -i	Prompt for confirmation if the destination path exists\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Prefs prefs;

	memset(&prefs, 0, sizeof(Prefs));
	prefs |= PREFS_f;
	while((o = getopt(argc, argv, "fi")) != -1)
		switch(o)
		{
			case 'f':
				prefs -= prefs & PREFS_i;
				prefs |= PREFS_f;
				break;
			case 'i':
				prefs -= prefs & PREFS_f;
				prefs |= PREFS_i;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	return _mv(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
