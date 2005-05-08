/* arch/arch.c */



#include <stdlib.h>
#include "../as.h"
#include "arch.h"


/* Arch */
Arch * arch_new(char * arch)
{
	Arch * a;

	if((a = malloc(sizeof(Arch))) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	/* FIXME */
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	free(arch);
}
