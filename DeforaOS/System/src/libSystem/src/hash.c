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


/* HashEntry */
/* types */
typedef struct _HashEntry {
	char * name;
	void * data;
} HashEntry;
ARRAY(HashEntry, hashentry);


/* functions */
static int hashentry_init(HashEntry * he, char const * name, void * data)
{
	if((he->name = string_new(name)) == NULL)
		return 1;
	he->data = data;
	return 0;
}

static void hashentry_destroy(HashEntry * he)
{
	free(he->name);
}


/* useful */
static void hashentry_set_data(HashEntry * he, void * data)
{
	he->data = data;
}


/* Hash */
Hash * hash_new(void)
{
	Hash * hash;

	if((hash = hashentryarray_new()) == NULL)
		return NULL;
	return hash;
}

void hash_delete(Hash * hash)
{
	size_t i;
	HashEntry * he;

	for(i = array_count(hash); i > 0; i--)
		if((he = array_get(hash, i - 1)) != NULL)
			hashentry_destroy(he);
	array_delete(hash);
}


/* useful */
void * hash_get(Hash * hash, char const * name)
{
	size_t i;
	HashEntry * he;

	for(i = array_count(hash); i > 0; i--)
	{
		if((he = array_get(hash, i - 1)) == NULL)
			return NULL;
		if(string_compare(he->name, name) == 0)
			return he->data;
	}
	error_set_code(1, "%s%s", name, ": Not found");
	return NULL;
}


int hash_set(Hash * hash, char const * name, void * data)
{
	size_t i;
	HashEntry he;
	HashEntry * p;

	for(i = array_count(hash); i > 0; i--)
	{
		if((p = array_get(hash, i - 1)) == NULL)
			return 1;
		if(string_compare(p->name, name) == 0)
		{
			hashentry_set_data(p, data);
			return 0;
		}
	}
	if(hashentry_init(&he, name, data) != 0)
		return 1;
	if(array_append(hash, &he) == 0)
		return 0;
	hashentry_destroy(&he);
	return 1;
}
