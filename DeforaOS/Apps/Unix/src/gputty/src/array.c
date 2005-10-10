/* array.c */
/* Copyright (c) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include "array.h"


/* Array */
Array * array_new(void)
{
	Array * array;

	if((array = malloc(sizeof(Array))) == NULL)
		return NULL;
	array->data = NULL;
	array->size = 0;
	return array;
}

void array_delete(Array * array)
{
	if(array->size > 0)
		free(array->data);
	free(array);
}


unsigned int array_get_size(Array * array)
{
	return array->size;
}


int array_set_size(Array * array, unsigned int size)
{
	void * p;

	if((p = realloc(array->data, sizeof(void*) * size)) == NULL)
		return 1;
	array->data = p;
	array->size = size;
	return 0;
}


void * array_get(Array * array, unsigned int pos)
{
	if(pos >= array->size)
		return NULL;
	return array->data[pos];
}


int array_set(Array * array, unsigned int pos, void * data)
{
	if(array->size <= pos && array_set_size(array, pos + 1) != 0)
		return 1;
	array->data[pos] = data;
	return 0;
}


int array_append(Array * array, void * data)
{
	if(array_set_size(array, array->size + 1) != 0)
		return 1;
	array->data[array->size - 1] = data;
	return 0;
}
