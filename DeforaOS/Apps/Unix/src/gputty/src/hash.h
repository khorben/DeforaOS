/* hash.h */
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



#ifndef _HASH_H
# define _HASH_H

# include "array.h"


/* Hash */
/* types */
typedef Array Hash;


/* functions */
Hash * hash_new(void);
void hash_delete(Hash * hash);

/* useful */
void * hash_get(Hash * hash, char * name);
int hash_set(Hash * hash, char * name, void * data);

# endif /* !_HASH_H */
