/* i386.c */



#include <stddef.h>
#include "arch.h"


/* variables */
ArchInstruction arch_i386_set[] =
{
#include "80386.ins"
	{ NULL,		0x0000, AO_NONE }
};

ArchRegister arch_i386_regs[] =
{
#include "80386.reg"
	{ NULL,		0 }
};

ArchPlugin arch_plugin =
{
	arch_i386_set,
	arch_i386_regs
};
