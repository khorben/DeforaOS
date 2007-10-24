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



#ifndef LIBSYSTEM_BUFFER_H
# define LIBSYSTEM_BUFFER_H


/* Buffer */
/* types */
typedef struct _Buffer
{
	unsigned int size;
	char * data;
} Buffer;


/* functions */
Buffer * buffer_new(unsigned int size, char const * data);
void buffer_delete(Buffer * buffer);

/* returns */
char * buffer_data(Buffer const * buffer);
unsigned int buffer_length(Buffer const * buffer);

#endif /* !LIBSYSTEM_BUFFER_H */
