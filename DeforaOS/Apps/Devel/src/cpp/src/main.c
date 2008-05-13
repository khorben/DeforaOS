/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* FIXME:
 * - remove -t? (parse trigraphs by default) */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "cpp.h"
#include "../config.h"


/* cpp */
/* private */
/* types */
typedef struct _Prefs
{
	int flags;
	char const * outfile;
	const char ** paths;
	size_t paths_cnt;
	const char ** defines;
	size_t defines_cnt;
	const char ** undefines;
	size_t undefines_cnt;
} Prefs;
#define PREFS_t		0x1


/* prototypes */
static int _cpp(Prefs * prefs, int filec, char * filev[]);
static int _cpp_error(void);


/* functions */
/* cpp */
static int _cpp_do(Prefs * prefs, FILE * fp, char const * filename);

static int _cpp(Prefs * prefs, int filec, char * filev[])
{
	int ret = 0;
	FILE * fp;
	int i;

	if(prefs->outfile == NULL)
		fp = stdout;
	else if((fp = fopen(prefs->outfile, "w")) == NULL)
		return error_set_print("cpp", 1, "%s: %s", prefs->outfile,
				strerror(errno));
	for(i = 0; i < filec; i++)
		ret |= _cpp_do(prefs, fp, filev[i]);
	if(fclose(fp) != 0)
		return error_set_print("cpp", 1, "%s: %s", prefs->outfile,
				strerror(errno));
	return ret;
}

static int _cpp_do(Prefs * prefs, FILE * fp, char const * filename)
{
	int ret;
	int filter = CPP_FILTER_WHITESPACE | CPP_FILTER_COMMENT;
	Cpp * cpp;
	size_t i;
	size_t j;
	size_t k;
	Token * token;
	int code;

	if(prefs->flags & PREFS_t)
		filter |= CPP_FILTER_TRIGRAPH;
	if((cpp = cpp_new(filename, filter)) == NULL)
		return _cpp_error();
	for(i = 0; i < prefs->paths_cnt; i++)
		if(cpp_path_add(cpp, prefs->paths[i]) != 0)
			break;
	for(j = 0; j < prefs->defines_cnt; j++)
		if(cpp_define_add(cpp, prefs->defines[j], NULL) != 0)
			break;
	for(k = 0; k < prefs->undefines_cnt; k++)
		if(cpp_define_remove(cpp, prefs->undefines[k]) != 0)
			break;
	if(i != prefs->paths_cnt || j != prefs->defines_cnt
			|| k != prefs->undefines_cnt)
	{
		cpp_delete(cpp);
		return 1;
	}
	while((ret = cpp_scan(cpp, &token)) == 0)
	{
		if(token == NULL) /* end of file */
			break;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s (%d)\n", token_get_string(token),
				token_get_code(token));
#else
		if((code = token_get_code(token)) == CPP_CODE_META_ERROR
				|| code == CPP_CODE_META_WARNING)
			fprintf(stderr, "%s%s%s%s%u%s%s\n",
					code == CPP_CODE_META_ERROR
					? "Error" : "Warning", " in ",
					token_get_filename(token), ":",
					token_get_line(token), ": ",
					token_get_string(token));
		else if(code >= CPP_CODE_META_FIRST
			&& code <= CPP_CODE_META_LAST)
			fprintf(fp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), fp);
#endif
		token_delete(token);
	}
	if(ret != 0)
		_cpp_error();
	cpp_delete(cpp);
	return ret;
}


/* cpp_error */
static int _cpp_error(void)
{
	return error_print(PACKAGE);
}


/* usage */
/* FIXME -E prints metadata? */
static int _usage(void)
{
	fputs("Usage: " PACKAGE " [-D name[=value]]...[-I directory][-o outfile][-t][-U name]... input...\n"
"  -D	Add a substitution\n"
"  -I	Add a directory to the search path\n"
"  -o	Write output to a file\n"
"  -t	Convert trigraphs\n"
"  -U	Remove a substitution\n", stderr);
	return 1;
}


/* main */
static int _main_add_define(Prefs * name, char * define);
static int _main_add_path(Prefs * prefs, char const * path);
static int _main_add_undefine(Prefs * prefs, char const * undefine);

int main(int argc, char * argv[])
{
	int ret;
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "D:I:o:tU:")) != -1)
		switch(o)
		{
			case 'D':
				if(_main_add_define(&prefs, optarg) != 0)
					return 2;
				break;
			case 'I':
				if(_main_add_path(&prefs, optarg) != 0)
					return 2;
				break;
			case 'o':
				prefs.outfile = optarg;
				break;
			case 't':
				prefs.flags |= PREFS_t;
				break;
			case 'U':
				if(_main_add_undefine(&prefs, optarg) != 0)
					return 2;
				break;
			default:
				return _usage();
		}
	if(argc - optind < 1)
		return _usage();
	ret = _cpp(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
	free(prefs.paths);
	return ret;
}

static int _main_add_define(Prefs * prefs, char * define)
{
	const char ** p;
	char * value;

	if(strlen(define) == 0)
		return 1;
	value = strtok(define, "=");
	if((p = realloc(prefs->defines, sizeof(*p) * (prefs->defines_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->defines = p;
	prefs->defines[prefs->defines_cnt++] = define;
	return 0;
}

static int _main_add_path(Prefs * prefs, char const * path)
{
	const char ** p;

	if((p = realloc(prefs->paths, sizeof(*p) * (prefs->paths_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->paths = p;
	prefs->paths[prefs->paths_cnt++] = path;
	return 0;
}

static int _main_add_undefine(Prefs * prefs, char const * undefine)
{
	const char ** p;

	if(strlen(undefine) == 0)
		return 1;
	if((p = realloc(prefs->undefines, sizeof(*p)
					* (prefs->undefines_cnt + 1))) == NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->undefines = p;
	prefs->undefines[prefs->undefines_cnt++] = undefine;
	return 0;
}
