/* sparc64.c */



#include <stdlib.h>
#include "arch.h"


/* variables */
ArchRegister arch_sparc64_regs[] =
{
	{ NULL,	0,	0 }
};

ArchInstruction arch_sparc64_set[] =
{
	{ NULL,	0x0,	0, 0, 0, 0 }
};

ArchPlugin arch_plugin =
{
	arch_sparc64_regs,
	arch_sparc64_set
};
