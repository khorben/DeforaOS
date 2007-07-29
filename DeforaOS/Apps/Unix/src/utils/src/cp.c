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



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>


/* types */
typedef int Prefs;
#define PREFS_f 0x01
#define PREFS_i 0x02
#define PREFS_p 0x04
#define PREFS_R 0x08
#define PREFS_H 0x10
#define PREFS_L 0x20
#define PREFS_P 0x40


/* cp */
static int _cp_error(char const * message, int ret);
static int _cp_single(Prefs * prefs, char const * src, char const * dst);
static int _cp_multiple(Prefs * prefs, int filec, char * const filev[]);

static int _cp(Prefs * prefs, int filec, char * filev[])
{
	struct stat st;

	if(stat(filev[filec - 1], &st) != 0)
	{
		if(errno != ENOENT)
			return _cp_error(filev[filec - 1], 1);
		if(filec > 2)
		{
			fprintf(stderr, "%s%s%s", "cp: ", filev[filec - 1],
					": Does not exist and more than two"
					" operands were given\n");
			return 1;
		}
	}
	else if(S_ISDIR(st.st_mode))
		return _cp_multiple(prefs, filec, filev);
	else if(filec > 2)
	{
		errno = ENOTDIR;
		return _cp_error(filev[filec - 1], 1);
	}
	return _cp_single(prefs, filev[0], filev[1]);
}

static int _cp_error(char const * message, int ret)
{
	fputs("cp: ", stderr);
	perror(message);
	return ret;
}

static int _cp_confirm(char const * message)
{
	int c;
	int tmp;

	fprintf(stderr, "%s%s%s", "cp: ", message, ": Overwrite? ");
	if((c = fgetc(stdin)) == EOF)
		return 0;
	while(c != '\n' && (tmp = fgetc(stdin)) != EOF && tmp != '\n');
	return c == 'y';
}

/* _cp_single
 * XXX TOCTOU all over the place (*stat) but seem impossible to avoid */
static int _single_dir(Prefs * prefs, char const * src, char const * dst);
static int _single_fifo(char const * dst);
static int _single_symlink(char const * src, char const * dst);
static int _single_regular(char const * src, char const * dst);
static int _single_p(char const * dst, struct stat * st);

static int _cp_single(Prefs * prefs, char const * src, char const * dst)
{
	int ret;
	struct stat st;
	struct stat st2;

	if(*prefs & PREFS_P) /* don't follow symlinks */
	{
		if(lstat(src, &st) != 0 && errno == ENOENT)
			return _cp_error(src, 1);
	}
	else if(stat(src, &st) != 0 && errno == ENOENT) /* follow symlinks */
		return _cp_error(src, 1);
	if(lstat(dst, &st2) == 0)
	{
		if(*prefs & PREFS_i && _cp_confirm(dst) != 1)
			return 0;
		if(unlink(dst) != 0)
			return _cp_error(dst, 1);
	}
	if(S_ISDIR(st.st_mode))
		ret = _single_dir(prefs, src, dst);
	else if(S_ISFIFO(st.st_mode))
		ret = _single_fifo(dst);
	else if(S_ISLNK(st.st_mode))
		ret = _single_symlink(src, dst);
	else
		ret = _single_regular(src, dst);
	if(ret != 0)
		return ret;
	if(*prefs & PREFS_p) /* XXX TOCTOU */
		_single_p(dst, &st);
	return 0;
}

/* single_dir */
static int _single_recurse(Prefs * prefs, char const * src, char const * dst);

static int _single_dir(Prefs * prefs, char const * src, char const * dst)
{
	if(*prefs & PREFS_R)
		return _single_recurse(prefs, src, dst);
	fprintf(stderr, "%s%s%s", "cp: ", src, ": Omitting directory\n");
	return 0;
}

static int _single_recurse(Prefs * prefs, char const * src, char const * dst)
{
	int ret = 0;
	Prefs prefs2 = *prefs;
	size_t srclen;
	size_t dstlen;
	DIR * dir;
	struct dirent * de;
	char * ssrc = NULL;
	char * sdst = NULL;
	char * p;

	if(mkdir(dst, 0777) != 0 && errno != EEXIST)
		return _cp_error(dst, 1);
	srclen = strlen(src);
	dstlen = strlen(dst);
	if((dir = opendir(src)) == NULL)
		return _cp_error(src, 1);
	prefs2 |= (prefs2 & PREFS_H) ? PREFS_P : 0;
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.' && (de->d_name[1] == '\0'
					|| (de->d_name[1] == '.'
						&& de->d_name[2] == '\0')))
			continue;
		if((p = realloc(ssrc, srclen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _cp_error(src, 1);
			continue;
		}
		ssrc = p;
		sprintf(ssrc, "%s/%s", src, de->d_name);
		if((p = realloc(sdst, dstlen + strlen(de->d_name) + 2)) == NULL)
		{
			ret |= _cp_error(ssrc, 1);
			continue;
		}
		sdst = p;
		sprintf(sdst, "%s/%s", dst, de->d_name);
		ret |= _cp_single(&prefs2, ssrc, sdst);
	}
	closedir(dir);
	free(ssrc);
	free(sdst);
	return ret;
}

static int _single_fifo(char const * dst)
{
	if(mkfifo(dst, 0666) != 0)
		return _cp_error(dst, 1);
	return 0;
}

static int _single_symlink(char const * src, char const * dst)
{
	char buf[PATH_MAX];
	ssize_t len;

	if((len = readlink(src, buf, sizeof(buf) - 1)) == -1)
		return _cp_error(src, 1);
	buf[len] = '\0';
	if(symlink(buf, dst) != 0)
		return _cp_error(dst, 1);
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
		return _cp_error(src, 1);
	if((fdst = fopen(dst, "w")) == NULL)
	{
		ret = _cp_error(dst, 1);
		fclose(fsrc);
		return ret;
	}
	while((size = fread(buf, sizeof(char), sizeof(buf), fsrc)) > 0)
		if(fwrite(buf, sizeof(char), size, fdst) != size)
		{
			ret = _cp_error(dst, 1);
			fclose(fsrc);
			fclose(fdst);
			return 1;
		}
	if(!feof(fsrc))
		ret = _cp_error(src, 1);
	if(fclose(fsrc) != 0)
		ret = _cp_error(src, 1);
	if(fclose(fdst) != 0)
		return _cp_error(dst, 1);
	return ret;
}

static int _single_p(char const * dst, struct stat * st)
{
	struct timeval tv[2];

	if(chown(dst, st->st_uid, st->st_gid) != 0)
	{
		_cp_error(dst, 0);
		if(chmod(dst, st->st_mode & ~(S_ISUID | S_ISGID)) != 0)
			_cp_error(dst, 0);
	}
	else if(chmod(dst, st->st_mode) != 0)
		_cp_error(dst, 0);
	tv[0].tv_sec = st->st_atime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = st->st_mtime;
	tv[1].tv_usec = 0;
	if(utimes(dst, tv) != 0)
		_cp_error(dst, 0);
	return 0;
}

static int _cp_multiple(Prefs * prefs, int filec, char * const filev[])
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
			ret |= _cp_error(filev[filec - 1], 1);
			continue;
		}
		sdst = p;
		sprintf(sdst, "%s/%s", filev[filec - 1], dst);
		ret |= _cp_single(prefs, filev[i], sdst);
	}
	free(sdst);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: cp [-fip] source_file target_file\n\
       cp [-fip] source_file ... target\n\
       cp -R [-H | -L | -P][-fip] source_file ... target\n\
       cp -r [-H | -L | -P][-fip] source_file ... target\n\
  -f    attempt to remove destination file before a copy if necessary\n\
  -i    prompt before a copy to an existing file\n\
  -p    duplicate characteristics of the source files\n\
  -R    copy file hierarchies\n\
  -r    copy file hierarchies\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(Prefs));
	prefs |= PREFS_i | PREFS_H;
	while((o = getopt(argc, argv, "fipRrHLP")) != -1)
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
			case 'p':
				prefs |= PREFS_p;
				break;
			case 'R':
			case 'r':
				prefs |= PREFS_R;
				break;
			case 'H':
				prefs -= prefs & (PREFS_L | PREFS_P);
				prefs |= PREFS_H;
				break;
			case 'L':
				prefs -= prefs & (PREFS_H | PREFS_P);
				prefs |= PREFS_L;
				break;
			case 'P':
				prefs -= prefs & (PREFS_H | PREFS_L);
				prefs |= PREFS_P;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 2)
		return _usage();
	return _cp(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
