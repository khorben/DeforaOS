/* slist.c */



#include <stdlib.h>
#include "slist.h"


/* SList */
SList * slist_new(void)
{
	SList * slist;

	if((slist = malloc(sizeof(SList))) == NULL)
		return NULL;
	slist->type = ST_HEAD;
	slist->data = NULL;
	slist->next = NULL;
	return slist;
}

void slist_delete(SList * slist)
{
	SList * p;

	while(slist != NULL)
	{
		p = slist->next;
		free(slist);
		slist = p;
	}
}
