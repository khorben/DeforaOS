/* format/format.c */



#include <stdlib.h>
#include "../as.h"
#include "format.h"


/* Format */
Format * format_new(char * format)
{
	Format * f;

	if((f = malloc(sizeof(Format))) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	/* FIXME */
	return f;
}


/* format_delete */
void format_delete(Format * format)
{
	free(format);
}
