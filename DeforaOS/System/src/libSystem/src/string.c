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



#include <stdlib.h>
#include <string.h>
#include "System.h"


/* String */
/* public */
/* string_new */
String * string_new(String const * string)
{
	String * str;
	size_t length = string_length(string);

	if((str = malloc(length + 1)) == NULL)
		return NULL;
	strcpy(str, string);
	return str;
}


/* string_delete */
void string_delete(String * string)
{
	free(string);
}


/* useful */
/* string_append */
int string_append(String ** string, String const * append)
{
	char * p;
	size_t length = string_length(*string);

	if((p = realloc(*string, length + string_length(append) + 1)) == NULL)
		return 1;
	*string = p;
	strcpy(*string + length, append);
	return 0;
}


/* string_compare */
int string_compare(String const * string, String const * string2)
{
	unsigned char const * u1;
	unsigned char const * u2;

	u1 = (unsigned char const *)string;
	u2 = (unsigned char const *)string2;
	while(*u1 && *u2 && *u1 == *u2)
	{
		u1++;
		u2++;
	}
	return *u1 - *u2;
}


/* string_compare_length */
int string_compare_length(String const * string, String const * string2,
		size_t length)
{
	unsigned char const * u1;
	unsigned char const * u2;

	u1 = (unsigned char const *)string;
	u2 = (unsigned char const *)string2;
	while(--length && *u1 && *u2 && *u1 == *u2)
	{
		u1++;
		u2++;
	}
	return *u1 - *u2;
}


/* string_find */
String * string_find(String const * string, String const * key)
{
	size_t len;
	char const * p;

	len = string_length(key);
	for(p = string; *p != '\0' && string_compare_length(key, p, len) != 0;
			p++);
	if(*p == '\0')
		return NULL;
	return (String *)p;
}


/* string_length */
size_t string_length(String const * string)
{
	size_t length;

	for(length = 0; *string != '\0'; string++)
		length++;
	return length;
}
