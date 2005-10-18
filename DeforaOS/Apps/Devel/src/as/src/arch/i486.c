/* i486.c */



#include <stddef.h>
#include "arch.h"


/* types */
typedef enum _ArchOperands
{
#include "common.ops"
#include "80386.ops"
#include "80486.ops"
} ArchOperands;


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
	arch_i486_regs,
	arch_i486_set
};
