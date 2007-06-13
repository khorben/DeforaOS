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
