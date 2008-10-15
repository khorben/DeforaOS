/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBSYSTEM_ARRAY_H
# define LIBSYSTEM_ARRAY_H


/* Array */
/* macros */
# define ARRAY(type, name) \
	typedef Array name ## Array; \
	Array * name ## array_new(void) { return array_new(sizeof(type)); }


/* types */
typedef struct _Array Array;

typedef void (*ArrayForeach)(void * value, void * data);


/* functions */
Array * array_new(size_t size);
void array_delete(Array * array);

/* accessors */
size_t array_count(Array * array);

void * array_get(Array * array, size_t pos);
int array_get_copy(Array * array, size_t pos, void * value);
int array_set(Array * array, size_t pos, void * value);

/* useful */
int array_append(Array * array, void * value);
int array_remove_pos(Array * array, size_t pos);

void array_foreach(Array * array, ArrayForeach func, void * data);

#endif /* !LIBSYSTEM_ARRAY_H */
