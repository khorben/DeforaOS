/* hash.c */



#include <stdlib.h>
#include <string.h>
#include "hash.h"


/* HashEntry */
/* types */
typedef struct _HashEntry {
	char * name;
	void * data;
} HashEntry;
ARRAY(HashEntry *, hashentry);


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
	unsigned int i;
	HashEntry * he;

	for(i = array_count(hash); i > 0; i--)
		if((he = array_get(hash, i - 1)) != NULL)
			hashentry_destroy(he);
	array_delete(hash);
}


/* useful */
void * hash_get(Hash * hash, char const * name)
{
	unsigned int i;
	HashEntry * he;

	for(i = array_count(hash); i > 0; i--)
	{
		if((he = array_get(hash, i - 1)) == NULL)
			return NULL;
		if(string_compare(he->name, name) == 0)
			return he->data;
	}
	return NULL;
}


int hash_set(Hash * hash, char const * name, void * data)
{
	unsigned int i;
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
