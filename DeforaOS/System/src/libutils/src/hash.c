/* hash.c */



#include <stdlib.h>
#include <string.h>
#include "hash.h"


/* types */
typedef struct _HashEntry {
	char * name;
	void * data;
} HashEntry;


/* HashEntry */
static HashEntry * hashentry_new(char * name, void * data)
{
	HashEntry * he;

	if((he = malloc(sizeof(HashEntry))) == NULL)
		return NULL;
	he->name = strdup(name);
	he->data = data;
	return he;
}

static void hashentry_delete(HashEntry * he)
{
	free(he->name);
	free(he);
}


/* useful */
/*static int hashentry_compare(HashEntry * h1, HashEntry * h2)
{
	return strcmp(h1->name, h2->name);
}*/


/* Hash */
Hash * hash_new()
{
	return darray_new();
}

void hash_delete(Hash * h)
{
	HashEntry * he;
	int i;

	for(i = 0; i < darray_count(h); i++)
	{
		if((he = darray_get(h, i)) == NULL)
			continue;
		hashentry_delete(he);
	}
	free(h);
}


/* useful */
void * hash_get(Hash * h, char * name)
{
	HashEntry * he;
	int i;

	for(i = 0; i < darray_count(h); i++)
	{
		if((he = darray_get(h, i)) == NULL)
			continue;
		if(strcmp(name, he->name) == 0)
			return he->data;
	}
	return NULL;
}

int hash_set(Hash * h, char * name, void * data)
{
	HashEntry * he;
	int avail = -1;
	int i;

	for(i = 0; i < darray_count(h); i++)
	{
		if((he = darray_get(h, i)) == NULL)
		{
			avail = i;
			continue;
		}
		if(strcmp(name, he->name) == 0)
		{
			he->data = data;
			return 0;
		}
	}
	if((he = hashentry_new(name, data)) == NULL)
		return -1;
	if(avail != -1)
		i = avail;
	if(darray_set(h, i, he) == -1) 
	{
		hashentry_delete(he);
		return -1;
	}
	return 0;
}
