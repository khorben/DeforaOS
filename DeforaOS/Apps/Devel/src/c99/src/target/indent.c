/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "c99/target.h"
#include "../common.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


/* indent */
/* private */
/* variables */
static FILE * _fp;
static char * _filename;
static int _scope = 0;
static int _newline = 0;


/* protected */
/* prototypes */
static int _indent_init(char const * outfile, int optlevel);
static int _indent_exit(void);
static int _indent_token(Token * token);
static int _indent_function_begin(char const * name);
static int _indent_function_call(char const * name);
static int _indent_function_end(void);


/* public */
/* variables */
TargetPlugin target_plugin =
{
	NULL,				/* helper */
	NULL,				/* options */
	_indent_init,
	_indent_exit,
	_indent_token,			/* parsing */
	NULL,				/* section */
	_indent_function_begin,
	_indent_function_call,
	_indent_function_end,
	NULL				/* label_set */
};


/* protected */
/* functions */
/* indent_init */
static int _indent_init(char const * outfile, int optlevel)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %d)\n", __func__, outfile, optlevel);
#endif
	if((_filename = strdup(outfile)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	if((_fp = fopen(outfile, "w")) == NULL)
	{
		free(_filename);
		return error_set_code(1, "%s: %s", outfile, strerror(errno));
	}
	return 0;
}


/* indent_exit */
static int _indent_exit(void)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(fclose(_fp) != 0)
		ret |= error_set_code(1, "%s: %s", _filename, strerror(errno));
	free(_filename);
	return 0;
}


/* indent_token */
static int _indent_token(Token * token)
{
	char const * str;
	C99Code code;
	int i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token == NULL)
		return 0;
	str = token_get_string(token);
	if((code = token_get_code(token)) == C99_CODE_WHITESPACE)
	{
		if(strchr(str, '\n') != NULL)
			_newline = 1;
	}
	else
	{
		if(code == C99_CODE_OPERATOR_RBRACE
				|| code == C99_CODE_OPERATOR_RBRACKET)
			_scope--;
		if(code < C99_CODE_META_FIRST || code > C99_CODE_META_LAST)
		{
			if(_newline != 0)
			{
				for(i = 0; i < _scope; i++)
					fputc('\t', _fp);
				_newline = 0;
			}
		}
		if(code == C99_CODE_OPERATOR_LBRACE
				|| code == C99_CODE_OPERATOR_LBRACKET)
			_scope++;
	}
	fputs(str, _fp);
	return 0;
}


/* indent_function_begin */
static int _indent_function_begin(char const * name)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	_scope = 1;
	return ret;
}


/* indent_function_call */
static int _indent_function_call(char const * name)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	return 0;
}


/* indent_function_end */
static int _indent_function_end(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_scope = 0;
	return 0;
}
