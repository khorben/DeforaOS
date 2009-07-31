/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "code.h"
#include "parser.h"
#include "c99.h"


/* private */
/* constants */
#define DEFAULT_OBJECT_FILENAME "a.out"


/* public */
/* functions */
/* c99_new */
static Cpp * _new_cpp(C99Prefs const * prefs, char const * pathname);
static char * _new_outfile(int flags, char const * outfile,
		char const * pathname);

C99 * c99_new(C99Prefs const * prefs, char const * pathname)
{
	C99 * c99;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((c99 = object_new(sizeof(*c99))) == NULL)
		return NULL;
	memset(c99, 0, sizeof(*c99));
	if((c99->cpp = _new_cpp(prefs, pathname)) == NULL)
	{
		object_delete(c99);
		return NULL;
	}
	if((c99->outfile = _new_outfile(prefs->flags, prefs->outfile, pathname))
			== NULL)
	{
		c99_delete(c99);
		return NULL;
	}
	if(!(prefs->flags & C99PREFS_E)) /* we're not pre-processing */
	{
		if((c99->code = code_new(prefs, c99->outfile)) == NULL)
		{
			c99_delete(c99);
			return NULL;
		}
		return c99;
	}
	if(c99->outfile[0] == '\0')
	{
		c99->outfp = stdout;
		return c99;
	}
	if((c99->outfp = fopen(c99->outfile, "w")) != NULL)
		return c99;
	error_set_code(1, "%s: %s", c99->outfile, strerror(errno));
	c99_delete(c99);
	return NULL;
}

static Cpp * _new_cpp(C99Prefs const * prefs, char const * pathname)
{
	CppPrefs cppprefs;
	Cpp * cpp;
	size_t i;
	size_t j;
	size_t k;

	memset(&cppprefs, 0, sizeof(cppprefs));
	cppprefs.filename = pathname;
	cppprefs.filters = CPP_FILTER_TRIGRAPH | CPP_FILTER_WHITESPACE
		| CPP_FILTER_COMMENT;
	if((cpp = cpp_new(&cppprefs)) == NULL)
		return NULL;
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
		return NULL;
	}
	return cpp;
}

static char * _new_outfile(int flags, char const * outfile,
		char const * pathname)
{
	char * ret;
	size_t len;

	if(flags & C99PREFS_c && outfile == NULL && pathname != NULL)
	{
		if((len = strlen(pathname)) < 3 || pathname[len - 2] != '.'
				|| pathname[len - 1] != 'c')
		{
			error_set_code(1, "%s", strerror(EINVAL));
			return NULL;
		}
		if((ret = strdup(pathname)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			return NULL;
		}
		ret[len - 1] = 'o';
		return ret;
	}
	if(flags & C99PREFS_E && outfile == NULL)
		outfile = ""; /* XXX a bit ugly */
	else if(outfile == NULL)
		outfile = DEFAULT_OBJECT_FILENAME;
	if((ret = strdup(outfile)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	return ret;
}


/* c99_delete */
int c99_delete(C99 * c99)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	cpp_delete(c99->cpp);
	if(c99->outfp != NULL && fclose(c99->outfp) != 0)
		ret = error_set_code(1, "%s: %s", c99->outfile,
				strerror(errno));
	free(c99->outfile);
	if(c99->token != NULL)
		token_delete(c99->token);
	if(c99->code != NULL)
		ret |= code_delete(c99->code);
	object_delete(c99);
	return ret;
}


/* useful */
/* c99_parse */
int c99_parse(C99 * c99)
{
	return parse(c99);
}
