/* array.c */



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include "array.h"


/* Array */
struct _Array
{
	char * data;
	unsigned int count;
	unsigned int size;
};

Array * array_new(unsigned int size)
{
	Array * array;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "array_new(", size, ")\n");
#endif
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

#ifdef DEBUG
	fprintf(stderr, "%s%p%s", "array_append(", data, ")\n");
#endif
	if((p = realloc(array->data, array->size * (array->count + 1))) == NULL)
		return 1;
	array->data = p;
	memcpy(&p[array->size * array->count], &data, array->size);
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


int array_get(Array * array, unsigned int pos, void * data)
{
	if(pos >= array->count)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "array_get(array, ", pos, ", data)\n");
#endif
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
