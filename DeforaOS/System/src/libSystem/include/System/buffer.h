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
typedef struct _Buffer Buffer;


/* functions */
Buffer * buffer_new(size_t size, char const * data);
void buffer_delete(Buffer * buffer);

/* accessors */
char * buffer_get_data(Buffer const * buffer);
int buffer_set_data(Buffer * buffer, size_t offset, char * data, size_t size);

size_t buffer_get_size(Buffer const * buffer);
int buffer_set_size(Buffer * buffer, size_t size);

#endif /* !LIBSYSTEM_BUFFER_H */
