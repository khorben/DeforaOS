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
	char * output;
	int flags;
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
	Token * token;
	int code;

	if((cpp = cpp_new(filename, prefs->flags & PREFS_t
					? CPP_FILTER_TRIGRAPH : 0)) == NULL)
		return _cpp_error();
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
		{
			if(fp == stdout)
				fprintf(stderr, "%s\n", token_get_string(
							token));
		}
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
	fputs("Usage: " PACKAGE " [-o output][-t] input...\n"
"  -o	Write output to a file\n"
"  -t	Convert trigraphs\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "o:t")) != -1)
		switch(o)
		{
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
	return _cpp(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}
