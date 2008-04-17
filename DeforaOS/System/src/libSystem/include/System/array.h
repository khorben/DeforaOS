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



#ifndef LIBSYSTEM_ARRAY_H
# define LIBSYSTEM_ARRAY_H


/* Array */
/* macros */
# define ARRAY(type, name) \
	typedef Array name ## Array; \
	Array * name ## array_new(void) { return array_new(sizeof(type)); }


/* types */
typedef struct _Array
{
	char * data;
	size_t count;
	size_t size;
} Array;

typedef void (*ArrayApplyFunc)(void * data, void * userdata);


/* functions */
Array * array_new(size_t size);
void array_delete(Array * array);

/* accessors */
size_t array_count(Array * array);

void * array_get(Array * array, size_t pos);
int array_get_copy(Array * array, size_t pos, void * data);
int array_set(Array * array, size_t pos, void * data);

/* useful */
int array_append(Array * array, void * data);
int array_remove_pos(Array * array, size_t pos);

void array_apply(Array * array, ArrayApplyFunc func, void * userdata);

#endif /* !LIBSYSTEM_ARRAY_H */
