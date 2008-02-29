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
	const char ** paths;
	size_t paths_cnt;
	char const * output;
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

	if(prefs->output == NULL)
		fp = stdout;
	else if((fp = fopen(prefs->output, "w")) == NULL)
		return error_set_print("cpp", 1, "%s: %s", prefs->output,
				strerror(errno));
	for(i = 0; i < filec; i++)
		ret |= _cpp_do(prefs, fp, filev[i]);
	if(fclose(fp) != 0)
		return error_set_print("cpp", 1, "%s: %s", prefs->output,
				strerror(errno));
	return ret;
}

static int _cpp_do(Prefs * prefs, FILE * fp, char const * filename)
{
	int ret;
	Cpp * cpp;
	size_t i;
	Token * token;
	int code;

	if((cpp = cpp_new(filename, prefs->flags & PREFS_t
					? CPP_FILTER_TRIGRAPH : 0)) == NULL)
		return _cpp_error();
	for(i = 0; i < prefs->paths_cnt; i++)
		if(cpp_add_path(cpp, prefs->paths[i]) != 0)
			break;
	if(i != prefs->paths_cnt)
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
		if((code = token_get_code(token)) == CPP_CODE_META_ERROR)
		{
			fprintf(stderr, "%s%s\n", "Error: ",
					token_get_string(token));
		}
		else if(code == CPP_CODE_META_WARNING)
			fprintf(stderr, "%s%s\n", "Warning: ",
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
	fputs("Usage: " PACKAGE " [-I directory][-o outfile][-t] input...\n"
"  -I	Add a directory to the search path\n"
"  -o	Write output to a file\n"
"  -t	Convert trigraphs\n", stderr);
	return 1;
}


/* main */
static int _main_add_path(Prefs * prefs, char const * path);

int main(int argc, char * argv[])
{
	int ret;
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "I:o:t")) != -1)
		switch(o)
		{
			case 'I':
				if(_main_add_path(&prefs, optarg) != 0)
					return 2;
				break;
			case 'o':
				prefs.output = optarg;
				break;
			case 't':
				prefs.flags |= PREFS_t;
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
