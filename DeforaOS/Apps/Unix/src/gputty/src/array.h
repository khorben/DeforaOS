/* array.h */
/* Copyright (C) 2004 Pierre Pronchery */
/* This file is part of GPuTTY. */
/* GPuTTY is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPuTTY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GPuTTY; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef _ARRAY_H
# define _ARRAY_H

# include "array.h"


/* Array */
/* types */
typedef struct _Array {
	void ** data;
	unsigned int size;
} Array;


/* functions */
Array * array_new(void);
void array_delete(Array * array);

/* useful */
unsigned int array_size(Array * array);

void * array_get(Array * array, unsigned int pos);
int array_set(Array * array, unsigned int pos, void * data);

int array_append(Array * array, void * data);

# endif /* !_ARRAY_H */
