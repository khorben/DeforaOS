/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include "System.h"


/* Object */
/* public */
/* functions */
/* object_new */
void * object_new(size_t size)
{
	void * object;

	if((object = malloc(size)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => %p\n", __func__, object);
#endif
	return object;
}


/* object_delete */
void object_delete(void * object)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, object);
#endif
	free(object);
}


/* useful */
Object * object_resize(Object ** object, size_t size)
{
	void * p;

	if((p = realloc(*object, size)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	*object = p;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(&%p, %zu) => %p\n", __func__, *object, size,
			p);
#endif
	return p;
}
