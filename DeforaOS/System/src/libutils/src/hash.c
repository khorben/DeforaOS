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


/* functions */
static HashEntry * hashentry_new(char const * name, void * data)
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
	HashEntry * he;

	if((i = array_get_size(hash)) != 0)
		for(; i > 0; i--)
		{
			if((he = array_get(hash, i - 1)) != NULL)
				hashentry_delete(he);
		}
	array_delete(hash);
}


/* useful */
void * hash_get(Hash * hash, char const * name)
{
	unsigned int i;
	HashEntry * he;

	if((i = array_get_size(hash)) == 0)
		return NULL;
	for(; i > 0; i--)
	{
		if((he = array_get(hash, i - 1)) == NULL)
			return NULL;
		if(strcmp(he->name, name) == 0)
			return he->data;
	}
	return NULL;
}


int hash_set(Hash * hash, char const * name, void * data)
{
	unsigned int i;
	HashEntry * he;

	if((i = array_get_size(hash)) != 0)
		for(; i > 0; i--)
		{
			if((he = array_get(hash, i - 1)) == NULL)
				return 1;
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
