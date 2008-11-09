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
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "System.h"


/* String */
/* public */
/* string_new */
String * string_new(String const * string)
{
	String * ret;
	size_t length = string_length(string);

	if((ret = object_new(length + 1)) == NULL)
		return NULL;
	strcpy(ret, string);
	return ret;
}


String * string_new_length(String const * string, size_t length)
{
	String * ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %zu)\n", __func__, string, length);
#endif
	if((ret = object_new(++length)) == NULL)
		return NULL;
	snprintf(ret, length, "%s", string);
	return ret;
}


/* string_delete */
void string_delete(String * string)
{
	object_delete(string);
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


/* string_explode */
/* FIXME return a StringArray instead? */
String ** string_explode(String const * string, String const * separator)
{
	String ** ret = NULL;
	size_t ret_cnt = 0;
	String ** p;			/* temporary pointer */
	size_t i;			/* current position */
	String const * s;		/* &string[i] */
	ssize_t j;			/* position of the next separator */
	ssize_t l;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\")\n", __func__, string,
			separator);
#endif
	if(separator == NULL || (l = string_length(separator)) == 0)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	for(i = 0;; i += j + l)
	{
		s = &string[i];
		j = string_index(s, separator);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(): i=%zu, j=%zd\n", __func__, i, j);
#endif
		if((p = realloc(ret, sizeof(*ret) * (ret_cnt + 2))) == NULL)
			break;
		ret = p;
		if(j < 0)
		{
			if((ret[ret_cnt++] = string_new(s)) == NULL)
				break;
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s(): \"%s\"\n", __func__,
					ret[ret_cnt - 1]);
#endif
			ret[ret_cnt++] = NULL;
			return ret;
		}
		if((ret[ret_cnt++] = string_new_length(s, j)) == NULL)
			break;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(): \"%s\"\n", __func__,
				ret[ret_cnt - 1]);
#endif
	}
	/* free everything */
	for(p = ret; *p != NULL; p++)
		string_delete(*p);
	free(ret);
	return NULL;
}


/* string_find */
String * string_find(String const * string, String const * key)
{
	ssize_t i;

	if((i = string_index(string, key)) < 0)
		return NULL;
	return (String*)&string[i]; /* XXX */
}


/* string_index */
ssize_t string_index(String const * string, String const * key)
{
	size_t len;
	ssize_t i;

	len = string_length(key);
	for(i = 0; string[i] != '\0' && string_compare_length(&string[i], key,
				len) != 0; i++);
	if(string[i] == '\0')
		return -1;
	return i;
}


/* string_length */
size_t string_length(String const * string)
{
	size_t length;

	for(length = 0; *string != '\0'; string++)
		length++;
	return length;
}
