/* hash.c */
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



#include <stdlib.h>
#include <string.h>
#include "hash.h"


/* HashEntry */
typedef struct _HashEntry {
	char * name;
	void * data;
} HashEntry;


/* functions */
static HashEntry * hashentry_new(char * name, void * data)
{
	HashEntry * he;

	if((he = malloc(sizeof(HashEntry))) == NULL)
		return NULL;
	if((he->name = strdup(name)) == NULL)
	{
		free(he);
		return NULL;
	}
	he->data = data;
	return he;
}

static void hashentry_delete(HashEntry * he)
{
	free(he->name);
	free(he);
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

	if((hash = array_new()) == NULL)
		return NULL;
	return hash;
}

void hash_delete(Hash * hash)
{
	unsigned int i;

	for(i = array_get_size(hash) - 1; i > 0; i--)
		hashentry_delete(array_get(hash, i));
	if(array_get_size(hash) > 0)
		hashentry_delete(array_get(hash, 0));
	array_delete(hash);
}


/* useful */
int hash_set(Hash * hash, char * name, void * data)
{
	unsigned int i;
	HashEntry * he;

	for(i = array_get_size(hash) - 1; i > 0; i--)
	{
		if((he = array_get(hash, i)) == NULL)
			return 1;
		if(strcmp(he->name, name) == 0)
		{
			hashentry_set_data(he, data);
			return 0;
		}
	}
	if(array_get_size(hash) > 0)
	{
		he = array_get(hash, 0);
		if(strcmp(he->name, name) == 0)
		{
			hashentry_set_data(he, data);
			return 0;
		}
	}
	if((he = hashentry_new(name, data)) == NULL)
		return 1;
	if(array_append(hash, he) == 0)
		return 0;
	hashentry_delete(he);
	return 1;
}
