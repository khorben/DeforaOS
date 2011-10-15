/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
/* TODO
 * - use one malloc() for a buffer */



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "System.h"


/* Buffer */
/* private */
/* types */
struct _Buffer
{
	size_t size;
	char * data;
};


/* public */
/* functions */
/* buffer_new */
Buffer * buffer_new(size_t size, char const * data)
{
	Buffer * buffer;

	if((buffer = object_new(sizeof(*buffer))) == NULL)
		return NULL;
	if((buffer->data = object_new(size)) == NULL && size != 0)
	{
		object_delete(buffer);
		return NULL;
	}
	if(data == NULL)
		memset(buffer->data, 0, size);
	else
		memcpy(buffer->data, data, size);
	buffer->size = size;
	return buffer;
}


/* buffer_delete */
void buffer_delete(Buffer * buffer)
{
	object_delete(buffer->data);
	object_delete(buffer);
}


/* accessors */
/* buffer_get_data */
char * buffer_get_data(Buffer const * buffer)
{
	return buffer->data;
}


/* buffer_get_size */
size_t buffer_get_size(Buffer const * buffer)
{
	return buffer->size;
}


/* buffer_set_data */
int buffer_set_data(Buffer * buffer, size_t offset, char * data, size_t size)
{
	if(offset + size > buffer->size) /* FIXME integer overflow */
		if(buffer_set_size(buffer, offset + size) != 0)
			return 1;
	memcpy(&buffer->data[offset], data, size);
	return 0;
}


/* buffer_set_size */
int buffer_set_size(Buffer * buffer, size_t size)
{
	char * p;

	if((p = realloc(buffer->data, size)) == NULL && size != 0)
		return error_set_code(1, "%s", strerror(errno));
	buffer->data = p;
	if(size > buffer->size)
		memset(&buffer->data[buffer->size], 0, size - buffer->size);
	buffer->size = size;
	return 0;
}
