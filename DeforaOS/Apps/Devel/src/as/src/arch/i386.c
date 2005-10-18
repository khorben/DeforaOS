/* i386.c */



#include <stddef.h>
#include "arch.h"


/* types */
typedef enum _ArchOperands
{
#include "common.ops"
#include "80386.ops"
} ArchOperands;


/* variables */
ArchRegister arch_i386_regs[] =
{
#include "80386.reg"
	{ NULL,		0 }
};

ArchInstruction arch_i386_set[] =
{
#include "80386.ins"
	{ NULL,		0x0000, AO_NONE }
};

ArchPlugin arch_plugin =
{
	arch_i386_regs,
	arch_i386_set
};
