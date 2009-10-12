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
#include <unistd.h>
#include <stdio.h>


/* prefs */
typedef int Prefs;
#define PREFS_h	1
#define PREFS_m	2
#define PREFS_M	4
#define PREFS_d	6
#define PREFS_i	8


/* file */
static int _file_do(Prefs * p, char * filename);
static int _file(Prefs * p, int argc, char * argv[])
{
	int ret = 0;
	int i;

	for(i = 0; i < argc; i++)
		ret += _file_do(p, argv[i]);
	return ret;
}

static int _file_do(Prefs * p, char * filename)
{
	struct stat st;
	int (*statfunc)(const char *, struct stat*) = *p & PREFS_h
		? lstat : stat;

	printf("%s: ", filename);
	if(statfunc(filename, &st) != 0)
	{
		printf("%s\n", "cannot open file");
		return 0;
	}
	if(S_ISLNK(st.st_mode))
		printf("%s", "symbolic link to ");
	if(S_ISBLK(st.st_mode))
		printf("%s", "block special ");
	else if(S_ISCHR(st.st_mode))
		printf("%s", "character special ");
	else if(S_ISDIR(st.st_mode))
		printf("%s", "directory ");
	else if(S_ISFIFO(st.st_mode))
		printf("%s", "fifo ");
	else if(S_ISSOCK(st.st_mode))
		printf("%s", "socket ");
	else
	{
		if(*p & PREFS_i)
			printf("%s", "regular file ");
		else if(st.st_size == 0)
			printf("%s", "empty ");
		if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			printf("%s", "executable ");
	}
	printf("%s", "data");
	fputc('\n', stdout);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: file [-dh][-M file][-m file] file ...\n\
       file -i [-h] file ...\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs p = 0;
	int o;

	while((o = getopt(argc, argv, "dhM:m:i")) != -1)
		switch(o)
		{
			case 'd':
				if(p & PREFS_i)
					return _usage();
				p |= PREFS_d;
				break;
			case 'h':
				p |= PREFS_h;
				break;
			case 'i':
				if(p != 0)
					return _usage();
				p = PREFS_i;
				break;
			case 'M':
				if(p & PREFS_i)
					return _usage();
				p |= PREFS_M;
				break;
			case 'm':
				if(p & PREFS_i)
					return _usage();
				p |= PREFS_m;
				break;
			default:
				return _usage();
		}
	return _file(&p, argc-optind, &argv[optind]);
}
