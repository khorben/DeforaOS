/* array.c */



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
