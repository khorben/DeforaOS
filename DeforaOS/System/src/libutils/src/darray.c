/* darray.c */



#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "darray.h"


/* darray */
DArray * darray_new(void)
{
	DArray * d;

	if((d = malloc(sizeof(DArray))) == NULL)
		return NULL;
	d->size = 0;
	d->count = 0;
	d->array = NULL;
	return d;
}

void darray_delete(DArray * d)
{
	if(d == NULL)
		return;
	free(d->array);
	free(d);
}


/* returns */
int darray_count(DArray * d)
{
	return d->count;
}

void * darray_get(DArray * d, int n)
{
	if(n >= d->count)
		return NULL;
	return d->array[n];
}


/* sets */
int darray_set(DArray * d, int n, void * data)
{
	void ** p;

	if(d->size <= n)
	{
		if((p = realloc(d->array, sizeof(void*) * (n+5))) == NULL)
			return -1;
		d->array = p;
		d->size = n + 5;
		memset(p + (sizeof(void*) * d->count), 0,
				(sizeof(void*) * (d->size - d->count)));
		d->count = n + 1;
	}
	else
		d->count = max(d->count, n + 1);
	d->array[n] = data;
	return 0;
}

void darray_merge(DArray * d)
{
	while(d->count && d->array[d->count] == NULL)
		d->count--;
	/* FIXME realloc if possible */
}
