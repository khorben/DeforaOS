/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "System.h"


/* private */
/* error_do */
static char const * _error_do(int * codeptr, char const * format, va_list args)
{
	static char buf[256] = "";
	static int code = 0;

	if(format != NULL) /* setting the error */
	{
		vsnprintf(buf, sizeof(buf), format, args);
		if(codeptr != NULL)
			code = *codeptr;
		return buf;
	}
	if(codeptr != NULL)
		*codeptr = code;
	return buf;
}


/* public */
/* accessors */
/* error_get */
static char const * _get_do(char const * null, ...);

char const * error_get(void)
{
	/* XXX workaround for portability */
	return _get_do(NULL);
}

static char const * _get_do(char const * null, ...)
{
	char const * ret;
	va_list args;

	va_start(args, null);
	ret = _error_do(NULL, NULL, args);
	va_end(args);
	return ret;
}


/* error_get_code */
static char const * _get_code_do(int * code, ...);

char const * error_get_code(int * code)
{
	/* XXX workaround for portability */
	return _get_code_do(code);
}

static char const * _get_code_do(int * code, ...)
{
	char const * ret;
	va_list args;

	va_start(args, code);
	ret = _error_do(code, NULL, args);
	va_end(args);
	return ret;
}


/* error_set */
void error_set(char const * format, ...)
{
	va_list args;

	va_start(args, format);
	_error_do(NULL, format, args);
	va_end(args);
}


/* error_set_code */
int error_set_code(int code, char const * format, ...)
{
	va_list args;

	va_start(args, format);
	_error_do(&code, format, args);
	va_end(args);
	return code;
}


/* error_set_print */
int error_set_print(char const * program, int code, char const * format, ...)
{
	va_list args;

	va_start(args, format);
	_error_do(&code, format, args);
	va_end(args);
	return error_print(program);
}


/* useful */
/* error_print */
static int _print_do(char const * program, ...);

int error_print(char const * program)
{
	/* XXX workaround for portability */
	return _print_do(program);
}

static int _print_do(char const * program, ...)
{
	int ret = 0;
	va_list args;

	va_start(args, program);
	if(program != NULL)
	{
		fputs(program, stderr);
		fputs(": ", stderr);
	}
	fputs(_error_do(&ret, NULL, args), stderr);
	fputc('\n', stderr);
	va_end(args);
	return ret;
}
