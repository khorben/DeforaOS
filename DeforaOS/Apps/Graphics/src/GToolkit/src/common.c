/* $Id$ */



#include <stdlib.h>
#include <stdio.h>
#include "common.h"


/* useful */
/* g_alloc */
void * g_alloc(size_t size)
{
	size_t i;
	void ** p;

	for(i = 0; i < gt.alloced_cnt; i++)
		if(gt.alloced[i] == NULL)
			break;
	if(i == gt.alloced_cnt)
	{
		if((p = realloc(gt.alloced, sizeof(void*) * (i+1))) == NULL)
			return NULL;
		gt.alloced = p;
		gt.alloced_cnt++;
	}
	gt.alloced[i] = malloc(size);
	return gt.alloced[i];
}


/* g_alloced */
void * g_alloced(void * ptr)
{
	size_t i;

	for(i = 0; i < gt.alloced_cnt; i++)
		if(gt.alloced[i] == ptr)
			return ptr;
	return NULL;
}


/* g_free */
void g_free(void * ptr)
{
	size_t i;
	void ** p;

	for(i = 0; i < gt.alloced_cnt; i++)
		if(gt.alloced[i] == ptr)
			break;
	if(i == gt.alloced_cnt)
	{
		fprintf(stderr, "%s%p\n", "Unknown address to free: ", ptr);
		return;
	}
	free(ptr);
	gt.alloced[i] = NULL;
	if(i != gt.alloced_cnt-1)
		return;
	for(; i > 0 && gt.alloced[i-1] == NULL; i--);
	if((p = realloc(gt.alloced, sizeof(void*) * i)) == NULL)
		return;
	gt.alloced = p;
	gt.alloced_cnt = i;
}
