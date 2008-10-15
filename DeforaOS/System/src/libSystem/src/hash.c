/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
#include "System.h"


/* HashEntry */
/* private */
/* types */
typedef struct _HashEntry
{
	unsigned int hash;
	void const * key;
	void * value;
} HashEntry;
ARRAY(HashEntry, _hashentry);
#define HashEntryArray _hashentryArray


/* prototypes */
static void _hashentry_init(HashEntry * he, HashFunc func, void const * key,
		void * value);

/* accessors */
static void _hashentry_set_value(HashEntry * he, void * value);


/* functions */
/* hashentry_init */
static void _hashentry_init(HashEntry * he, HashFunc func, void const * key,
		void * value)
{
	he->hash = (func != NULL) ? func(key) : 0;
	he->key = key;
	he->value = value;
}


/* accessors */
/* hashentry_set_value */
static void _hashentry_set_value(HashEntry * he, void * value)
{
	he->value = value;
}


/* Hash */
/* protected */
/* types */
struct _Hash
{
	HashFunc func;
	HashCompare compare;
	HashEntryArray * entries;
};


/* functions */
/* hash_func_string */
unsigned int hash_func_string(void const * key)
{
	String const * str = key;
	size_t i;
	unsigned int hash = 0;

	for(i = 0; i < sizeof(hash) && str[i] != '\0'; i++)
		hash |= str[i] << (i << 3);
	return hash;
}


/* hash_compare_string */
int hash_compare_string(void const * value1, void const * value2)
{
	String const * str1 = value1;
	String const * str2 = value2;

	return string_compare(str1, str2);
}


/* public */
/* functions */
/* hash_new */
Hash * hash_new(HashFunc func, HashCompare compare)
{
	Hash * hash;

	if(compare == NULL)
	{
		error_set_code(1, "%s", "Invalid comparison function");
		return NULL;
	}
	if((hash = object_new(sizeof(*hash))) == NULL)
		return NULL;
	if((hash->entries = _hashentryarray_new()) == NULL)
	{
		object_delete(hash);
		return NULL;
	}
	hash->func = func;
	hash->compare = compare;
	return hash;
}


/* hash_delete */
void hash_delete(Hash * hash)
{
	array_delete(hash->entries);
	object_delete(hash);
}


/* accessors */
/* hash_get */
void * hash_get(Hash * hash, void const * key)
{
	unsigned int h;
	size_t i;
	HashEntry * he;

	h = (hash->func != NULL) ? hash->func(key) : 0;
	for(i = array_count(hash->entries); i > 0; i--)
	{
		if((he = array_get(hash->entries, i - 1)) == NULL)
			return NULL;
		if(he->hash != h)
			continue;
		if(hash->compare(he->key, key) == 0)
			return he->value;
	}
	error_set_code(1, "%s", "Key not found");
	return NULL;
}


/* hash_set */
int hash_set(Hash * hash, void const * key, void * value)
{
	unsigned int h;
	size_t i;
	HashEntry he;
	HashEntry * p;

	h = (hash->func != NULL) ? hash->func(key) : 0;
	for(i = array_count(hash->entries); i > 0; i--)
	{
		if((p = array_get(hash->entries, i - 1)) == NULL)
			return 1;
		if(p->hash != h)
			continue;
		if(hash->compare(p->key, key) == 0)
		{
			_hashentry_set_value(p, value);
			return 0;
		}
	}
	_hashentry_init(&he, hash->func, key, value);
	if(array_append(hash->entries, &he) == 0)
		return 0;
	return 1;
}


/* useful */
static void _hash_foreach(void * value, void * data);

struct funcdata
{
	HashForeach func;
	void * data;
};

void hash_foreach(Hash * hash, HashForeach func, void * data)
{
	struct funcdata fd = { func, data };

	array_foreach(hash->entries, _hash_foreach, &fd);
}

static void _hash_foreach(void * value, void * data)
{
	HashEntry * he = value;
	struct funcdata * fd = data;

	fd->func(he->key, he->value, fd->data);
}
