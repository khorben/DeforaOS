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



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


/* pr */
/* types */
typedef struct _Prefs
{
	int flags;
	char * header;
	int lines;
	int width;
	int offset;
} Prefs;
#define PR_PREFS_d 1
#define PR_PREFS_F 2
#define PR_PREFS_n 4
#define PR_PREFS_t 8

/* functions */
static int _pr_error(char const * message, int ret);
static int _pr_do(Prefs * prefs, FILE * fp, char const * filename);

static int _pr(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	int i;
	FILE * fp;

	if(filec == 0)
		return _pr_do(prefs, stdin, "");
	for(i = 0; i < filec; i++)
	{
		if(strcmp(filev[i], "-") == 0)
		{
			ret |= _pr_do(prefs, stdin, "");
			continue;
		}
		if((fp = fopen(filev[i], "r")) == NULL)
		{
			ret |= _pr_error(filev[i], 1);
			continue;
		}
		ret |= _pr_do(prefs, fp, filev[i]);
		if(fclose(fp) != 0)
			ret |= _pr_error(filev[i], 1);
	}
	return ret;
}

static int _pr_error(char const * message, int ret)
{
	fputs("pr: ", stderr);
	perror(message);
	return ret;
}

/* _pr_do */
static char const * _do_mtime(FILE * fp, char const * filename);
static void _do_offset(int offset);
static void _do_header(Prefs * prefs, char const * mtime, char const * filename,
		unsigned long page);
static void _do_footer(Prefs * prefs);

static int _pr_do(Prefs * prefs, FILE * fp, char const * filename)
{
	char const * mtime;
	char * buf;
	size_t len;
	int nb = 0;
	unsigned long page = 1;
	unsigned long line = 1;

	mtime = _do_mtime(fp, filename);
	if((buf = malloc(prefs->width + 1)) == NULL)
		return _pr_error("malloc", 1);
	while(fgets(buf, prefs->width, fp) != NULL)
	{
		if(nb == 0 && !(prefs->flags & PR_PREFS_t) && prefs->lines > 10)
		{
			_do_header(prefs, mtime, filename, page++);
			nb = 10;
		}
		_do_offset(prefs->offset); /* FIXME not if truncated line */
		if(prefs->flags & PR_PREFS_n)
			printf("%5lu ", line);
		if((len = strlen(buf)) > 0)
		{
			if(buf[len - 1] != '\n')
				buf[len++] = '\n';
			else
			{
				line++;
				if(prefs->flags & PR_PREFS_d)
					buf[len++] = '\n';
			}
		}
		if(fwrite(buf, sizeof(char), len, stdout) != len)
		{
			free(buf);
			return _pr_error("stdout", 1);
		}
		if(nb++ == prefs->lines && prefs->lines > 10
				&& !(prefs->flags & PR_PREFS_t))
		{
			_do_footer(prefs);
			nb = 0;
		}
	}
	if(prefs->lines > 10 && !(prefs->flags & PR_PREFS_t))
		for(; nb != prefs->lines; nb++)
			fputc('\n', stdout);
	free(buf);
	return 0;
}

static char const * _do_mtime(FILE * fp, char const * filename)
{
	static char buf[18];
	struct stat st;
	struct tm tm;

	if(fp == stdin)
		st.st_mtime = time(NULL);
	else if(fstat(fileno(fp), &st) != 0)
	{
		st.st_mtime = 0;
		_pr_error(filename, 0);
	}
	localtime_r(&st.st_mtime, &tm);
	buf[strftime(buf, sizeof(buf), "%b %e %H:%M %Y", &tm)] = '\0';
	return buf;
}

static void _do_offset(int offset)
{
	while(offset-- > 0)
		fputc(' ', stdout);
}

static void _do_header(Prefs * prefs, char const * mtime, char const * filename,
		unsigned long page)
{
	int nb;

	for(nb = 0; nb < 5; nb++)
	{
		_do_offset(prefs->offset);
		if(nb == 2)
			printf("%s %s%s%lu", mtime, prefs->header != NULL
					? prefs->header : filename,
					" Page ", page);
		fputc('\n', stdout);
	}
}

static void _do_footer(Prefs * prefs)
{
	int i;

	if(prefs->flags & PR_PREFS_F)
	{
		_do_offset(prefs->offset);
		fputc('\f', stdout);
		return;
	}
	for(i = 0; i < 5; i++)
	{
		_do_offset(prefs->offset);
		fputc('\n', stdout);
	}
}


/* usage */
static int _usage(void)
{
	fputs("Usage: pr [+page][-column][-adFmrt][-e [char][ gap]]"
			"[-h header][-i[char][gap]]\n"
			"[-l lines][-n [char][width]][-o offset][-s[char]]"
			"[-w width] file...\n",
			stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;
	char * p;

	memset(&prefs, 0, sizeof(prefs));
	prefs.header = NULL;
	prefs.lines = 66;
	prefs.offset = 0;
	prefs.width = 72;
	while((o = getopt(argc, argv, "dFh:l:no:tw:")) != -1)
		switch(o)
		{
			case 'd':
				prefs.flags |= PR_PREFS_d;
				break;
			case 'F':
				prefs.flags |= PR_PREFS_F;
				break;
			case 'h':
				prefs.header = optarg;
				break;
			case 'l':
				prefs.lines = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines <= 0)
					return _usage();
				break;
			case 'n':
				prefs.flags |= PR_PREFS_n;
				break;
			case 'o':
				prefs.offset = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.lines < 0)
					return _usage();
				break;
			case 't':
				prefs.flags |= PR_PREFS_t;
				break;
			case 'w':
				prefs.width = strtol(optarg, &p, 10);
				if(optarg[0] == '\0' || *p != '\0'
						|| prefs.width <= 0)
					return _usage();
				break;
			default:
				return _usage();
		}
	return _pr(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
