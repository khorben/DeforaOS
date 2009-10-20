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
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* Prefs */
typedef int Prefs;
#define DU_PREFS_a 1
#define DU_PREFS_s 2 
#define DU_PREFS_x 4
#define DU_PREFS_k 8 
#define DU_PREFS_H 16
#define DU_PREFS_L 48


/* du */
static int _du_do(Prefs * prefs, char const * filename);

static int _du(Prefs * prefs, int argc, char * argv[])
{
	int ret = 0;
	int i;

	if(argc == 0)
		return _du_do(prefs, ".") == 0 ? 0 : 2;
	for(i = 0; i < argc; i++)
		ret |= _du_do(prefs, argv[i]);
	return ret;
}

static int _du_error(char const * error)
{
	fputs("du: ", stderr);
	perror(error);
	return 1;
}

/* _du_do */
static void _du_print(Prefs * prefs, off_t size, char const * filename);
static int _du_do_recursive(Prefs * prefs, char const * filename);

static int _du_do(Prefs * prefs, char const * filename)
{
	int (*_stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;

	if(*prefs & DU_PREFS_H)
		_stat = stat;
	if(_stat(filename, &st) != 0)
		return _du_error(filename);
	if(!S_ISDIR(st.st_mode))
	{
		_du_print(prefs, st.st_blocks, filename);
		return 0;
	}
	return _du_do_recursive(prefs, filename);
}

/* _du_do_recursive */
static int _recursive_do(Prefs * prefs, off_t * size, char ** filename);

static int _du_do_recursive(Prefs * prefs, char const * filename)
{
	int ret;
	off_t size = 0;
	char * p;
	int len;

	len = strlen(filename);
	if((p = malloc(len + 1)) == NULL)
		return _du_error("malloc");
	strcpy(p, filename);
	ret = _recursive_do(prefs, &size, &p);
	printf("%lld %s\n", (long long)size, filename);
	free(p);
	return ret;
}

static void _recursive_do_stat(Prefs * prefs, off_t * size, char ** filename);
static int _recursive_do(Prefs * prefs, off_t * size, char ** filename)
{
	DIR * dir;
	struct dirent * de;
	size_t len;
	char * p;

	if((dir = opendir(*filename)) == NULL)
		return _du_error(*filename);
	len = strlen(*filename);
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '\0')
			continue;
		if(de->d_name[0] == '.')
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
		if((p = realloc(*filename, len + strlen(de->d_name) + 2))
				== NULL)
		{
			_du_error("malloc");
			continue;
		}
		*filename = p;
		for(; len > 0 && p[len-1] == '/'; len--);
		p[len] = '/';
		strcpy(&p[len+1], de->d_name);
		_recursive_do_stat(prefs, size, filename);
	}
	return closedir(dir) == 0 ? 0 : 1;
}

static off_t _du_blocks(Prefs * prefs, off_t size);
static void _recursive_do_stat(Prefs * prefs, off_t * size, char ** filename)
{
	int (*_stat)(const char * filename, struct stat * buf) = lstat;
	struct stat st;
	char * p;
	long long dirsize;

	if(*prefs & DU_PREFS_L)
		_stat = stat;
	if(_stat(*filename, &st) != 0)
	{
		_du_error(*filename);
		return;
	}
	*size += _du_blocks(prefs, st.st_blocks);
	if(S_ISDIR(st.st_mode))
	{
		dirsize = _du_blocks(prefs, st.st_blocks);
		if((p = strdup(*filename)) == NULL)
		{
			_du_error(*filename);
			return;
		}
		_recursive_do(prefs, size, filename);
		if(!(*prefs & DU_PREFS_s))
			printf("%lld %s\n", dirsize, p);
		free(p);
	}
	else if(*prefs & DU_PREFS_a)
		_du_print(prefs, st.st_blocks, *filename);
}

static off_t _du_blocks(Prefs * prefs, off_t size)
{
	if(*prefs & DU_PREFS_k)
		return size / 2;
	return size;
}

static void _du_print(Prefs * prefs, off_t size, char const * filename)
{
	long long s;
	
	s = _du_blocks(prefs, size);
	printf("%lld %s\n", s, filename);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: du [-a | -s][-kx][-H | -L][file...]\n\
  -a	Report the size of every file encountered\n\
  -s	Report only the total sum for each of the specified files\n\
  -k	Write the file sizes in units of 1024 bytes rather than 512\n\
  -x	Evaluate file sizes only on the same device as the file specified\n\
  -H	Dereference specified files\n\
  -L	Dereference every file evaluated\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs = 0;
	int o;

	while((o = getopt(argc, argv, "askxHL")) != -1)
		switch(o)
		{
			case 'a':
				prefs -= prefs & DU_PREFS_s;
				prefs |= DU_PREFS_a;
				break;
			case 's':
				prefs -= prefs & DU_PREFS_a;
				prefs |= DU_PREFS_s;
				break;
			case 'k':
				prefs |= DU_PREFS_k;
				break;
			case 'x':
				prefs |= DU_PREFS_x;
				break;
			case 'H':
				prefs |= DU_PREFS_H;
				break;
			case 'L':
				prefs |= DU_PREFS_L;
				break;
			default:
				return _usage();
		}
	return _du(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
