/* i486.c */



#include <stddef.h>
#include "arch.h"


/* variables */
ArchInstruction arch_i486_set[] =
{
#include "80486.ins"
	{ NULL,		0x0000, AO_NONE }
};

ArchRegister arch_i486_regs[] =
{
#include "80486.reg"
	{ NULL,		0 }
};

ArchPlugin arch_plugin =
{
	arch_i486_set,
	arch_i486_regs
};
