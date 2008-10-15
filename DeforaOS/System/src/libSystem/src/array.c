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
/* FIXME:
 * - integer overflows when resizing array */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "System.h"


/* Array */
/* protected */
/* types */
struct _Array
{
	size_t count;
	size_t size;
	char * value;
};


/* public */
/* array_new */
Array * array_new(size_t size)
{
	Array * array;

	if((array = object_new(sizeof(*array))) == NULL)
		return NULL;
	array->count = 0;
	array->size = size;
	array->value = NULL;
	return array;
}


/* array_delete */
void array_delete(Array * array)
{
	free(array->value);
	object_delete(array);
}


/* accessors */
/* array_count */
size_t array_count(Array * array)
{
	return array->count;
}


/* array_get */
void * array_get(Array * array, size_t pos)
{
	if(pos >= array->count)
		return NULL;
	return &array->value[pos * array->size];
}


/* array_get_copy */
int array_get_copy(Array * array, size_t pos, void * value)
{
	if(pos >= array->count)
		return 1;
	memcpy(value, &array->value[pos * array->size], array->size);
	return 0;
}


/* array_set */
int array_set(Array * array, size_t pos, void * value)
	/* FIXME not tested */
{
	void * p;
	size_t cursize;
	size_t newpos;

	newpos = array->count * (pos);
	if(array->count <= pos)
	{
		if((p = realloc(array->value, array->size * (pos + 1))) == NULL)
			return error_set_code(1, "%s", strerror(errno));
		array->value = p;
		cursize = array->count * array->size;
		memset(&array->value[cursize], 0, newpos - cursize);
		array->count = pos + 1;
	}
	memcpy(&array->value[newpos], value, array->size);
	return 0;
}


/* useful */
/* array_append */
int array_append(Array * array, void * value)
{
	char * p;

	if((p = realloc(array->value, array->size * (array->count + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	array->value = p;
	memcpy(&p[array->size * array->count], value, array->size);
	array->count++;
	return 0;
}


/* array_remove_pos */
int array_remove_pos(Array * array, size_t pos)
{
	if(pos >= array->count)
		return 1;
	array->count--; /* FIXME resize array? */
	memmove(&array->value[pos * array->size],
			&array->value[(pos + 1) * array->size],
			(array->count - pos) * array->size);
	return 0;
}


/* array_foreach */
void array_foreach(Array * array, ArrayForeach func, void * data)
{
	size_t i;

	for(i = 0; i < array->count; i++)
		func(array->value + (i * array->size), data);
}
