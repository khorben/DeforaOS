/* gedi.c */



#include <stdlib.h>
#include "gedi.h"


/* GEDI */
GEDI * gedi_new(void)
{
	GEDI * gedi;

	if((gedi = malloc(sizeof(GEDI))) == NULL)
		return NULL;
	return gedi;
}

void gedi_delete(GEDI * gedi)
{
	free(gedi);
}
