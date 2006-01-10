/* buffer.c */



#include <stdlib.h>
#include <string.h>
#include "buffer.h"


/* Buffer */
Buffer * buffer_new(unsigned int size, char * data)
{
	Buffer * buffer;

	if((buffer = malloc(sizeof(Buffer))) == NULL)
		return NULL;
	if((buffer->data = malloc(size * sizeof(char))) == NULL)
	{
		free(buffer);
		return NULL;
	}
	if(data != NULL)
		memcpy(buffer->data, data, size);
	buffer->size = size;
	return buffer;
}


void buffer_delete(Buffer * buffer)
{
	free(buffer->data);
	free(buffer);
}


/* returns */
char * buffer_data(Buffer const * buffer)
{
	return buffer->data;
}


unsigned int buffer_length(Buffer const * buffer)
{
	return buffer->size;
}
