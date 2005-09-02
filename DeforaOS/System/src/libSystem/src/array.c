/* array.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "array.h"


/* Array */
struct _Array
{
	void * data;
	unsigned int count;
	unsigned int size;
};

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
	{
		perror("malloc");
		return 1;
	}
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
