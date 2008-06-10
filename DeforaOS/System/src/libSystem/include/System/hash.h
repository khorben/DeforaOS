/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBSYSTEM_HASH_H
# define LIBSYSTEM_HASH_H

# include "array.h"


/* Hash */
/* types */
typedef struct _Hash Hash;

typedef unsigned int (*HashFunc)(void const * value);
typedef int (*HashCompare)(void const * value1, void const * value2);
typedef void (*HashForeach)(void const * key, void * value, void * data);


/* functions */
Hash * hash_new(HashFunc func, HashCompare compare);
void hash_delete(Hash * h);

/* helpers */
extern unsigned int hash_func_string(void const * value);
extern int hash_compare_string(void const * value1, void const * value2);

/* accessors */
void * hash_get(Hash * h, void const * key);
int hash_set(Hash * h, void const * key, void * value);
size_t hash_count(Hash * hash);

/* useful */
void hash_foreach(Hash * hash, HashForeach func, void * data);

#endif /* !LIBSYSTEM_HASH_H */
