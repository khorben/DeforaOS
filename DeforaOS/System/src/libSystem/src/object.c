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



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "System.h"


/* Object */
/* public */
/* functions */
/* object_new */
void * object_new(size_t size)
{
	void * ptr;

	if((ptr = malloc(size)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	return ptr;
}


/* object_delete */
void object_delete(void * object)
{
	free(object);
}
