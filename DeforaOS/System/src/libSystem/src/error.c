/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <stdio.h>
#include <string.h>
#include "System.h"


/* private */
static char const * _error_do(char const * message, int * codeptr)
{
	static char buf[256] = "";
	static int code = 0;

	if(message != NULL) /* setting the error */
	{
		strncpy(buf, message, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
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
	return _error_do(NULL, NULL);
}


/* error_get_code */
char const * error_get_code(int * code)
{
	return _error_do(NULL, code);
}


/* error_set */
void error_set(char const * message)
{
	_error_do(message, NULL);
}


/* error_set_code */
int error_set_code(char const * message, int code)
{
	_error_do(message, &code);
	return code;
}


/* error_set_print */
int error_set_print(char const * prefix, char const * message, int code)
{
	_error_do(message, &code);
	return error_print(prefix);
}


/* useful */
int error_print(char const * prefix)
{
	int code = 0;

	if(prefix != NULL)
		fputs(prefix, stderr);
	fputs(_error_do(NULL, &code), stderr);
	return code;
}
