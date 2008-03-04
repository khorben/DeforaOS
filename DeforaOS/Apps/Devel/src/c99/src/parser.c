/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
#include "tokenset.h"
#include "c99.h"


/* public */
/* functions */
/* useful */
/* c99_parse */
static int _parse_E(C99 * c99);

int c99_parse(C99 * c99)
{
	if(c99->flags & C99PREFS_E)
		return _parse_E(c99);
	/* FIXME implement */
	return error_set_code(1, "%s", strerror(ENOSYS));
}

static int _parse_E(C99 * c99)
{
	int ret;
	Token * token;
	int code;

	while((ret = cpp_scan(c99->cpp, &token)) == 0
			&& token != NULL)
	{
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
			fprintf(c99->outfp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), c99->outfp);
		token_delete(token);
	}
	return ret;
}
