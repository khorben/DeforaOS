/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
/* FIXME:
 * - integer overflows when resizing array */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "System.h"


/* Array */
/* public */
/* array_new */
Array * array_new(size_t size)
{
	Array * array;

	if((array = object_new(sizeof(*array))) == NULL)
		return NULL;
	array->data = NULL;
	array->count = 0;
	array->size = size;
	return array;
}


/* array_delete */
void array_delete(Array * array)
{
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
	return &array->data[pos * array->size];
}


/* array_get_copy */
int array_get_copy(Array * array, size_t pos, void * data)
{
	if(pos >= array->count)
		return 1;
	memcpy(data, &array->data[pos * array->size], array->size);
	return 0;
}


/* array_set */
int array_set(Array * array, size_t pos, void * data)
	/* FIXME not tested */
{
	void * p;
	size_t cursize;
	size_t newpos;

	newpos = array->count * (pos);
	if(array->count <= pos)
	{
		if((p = realloc(array->data, array->size * (pos + 1))) == NULL)
			return error_set_code(1, "%s", strerror(errno));
		array->data = p;
		cursize = array->count * array->size;
		memset(&array->data[cursize], 0, newpos - cursize);
		array->count = pos + 1;
	}
	memcpy(&array->data[newpos], data, array->size);
	return 0;
}


/* useful */
/* array_append */
int array_append(Array * array, void * data)
{
	char * p;

	if((p = realloc(array->data, array->size * (array->count + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	array->data = p;
	memcpy(&p[array->size * array->count], data, array->size);
	array->count++;
	return 0;
}


/* array_remove_pos */
int array_remove_pos(Array * array, size_t pos)
{
	if(pos >= array->count)
		return 1;
	array->count--; /* FIXME resize array? */
	memmove(&array->data[pos * array->size],
			&array->data[(pos + 1) * array->size],
			(array->count - pos) * array->size);
	return 0;
}


/* array_apply */
void array_apply(Array * array, ArrayApplyFunc func, void * userdata)
{
	size_t i;

	for(i = 0; i < array->count; i++)
		func(&array->data + (i * array->size), userdata);
}
