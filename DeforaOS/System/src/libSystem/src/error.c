/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
char const * error_get(void)
{
	return _error_do(NULL, NULL, NULL);
}


/* error_get_code */
char const * error_get_code(int * code)
{
	return _error_do(code, NULL, NULL);
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
int error_print(char const * program)
{
	int code = 0;

	if(program != NULL)
	{
		fputs(program, stderr);
		fputs(": ", stderr);
	}
	fputs(_error_do(&code, NULL, NULL), stderr);
	fputc('\n', stderr);
	return code;
}
