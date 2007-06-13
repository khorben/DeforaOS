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



#include <stdlib.h>
#include <string.h>
#include "System.h"


/* Array */
Array * array_new(unsigned int size)
{
	Array * array;

	if((array = malloc(sizeof(Array))) == NULL)
		return NULL;
	array->data = NULL;
	array->count = 0;
	array->size = size;
	return array;
}


void array_delete(Array * array)
{
	free(array);
}


/* useful */
int array_append(Array * array, void * data)
{
	void * p;

	if((p = realloc(array->data, array->size * (array->count + 1))) == NULL)
		return 1;
	array->data = p;
	memcpy(&p[array->size * array->count], data, array->size);
	array->count++;
	return 0;
}


void array_apply(Array * array, ArrayApplyFunc func, void * userdata)
{
	unsigned int i;

	for(i = 0; i < array->count; i++)
		func(&array->data + (i * array->size), userdata);
}


unsigned int array_count(Array * array)
{
	return array->count;
}


void * array_get(Array * array, unsigned int pos)
{
	if(pos >= array->count)
		return NULL;
	return &array->data[pos * array->size];
}


int array_get_copy(Array * array, unsigned int pos, void * data)
{
	if(pos >= array->count)
		return 1;
	memcpy(data, &array->data[pos * array->size], array->size);
	return 0;
}


int array_remove_pos(Array * array, unsigned int pos)
{
	if(pos >= array->count)
		return 1;
	array->count--; /* FIXME resize array? */
	memmove(&array->data[pos * array->size],
			&array->data[(pos + 1) * array->size],
			(array->count - pos) * array->size);
	return 0;
}
