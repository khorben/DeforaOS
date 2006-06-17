/* i586.c */



#include <stddef.h>
#include "arch.h"


/* types */
#define REG(name, size, id) AO_ ## name = ((id << 2) | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__),
enum
{
#include "common.reg"
#include "80586.reg"
};


/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
ArchRegister arch_i586_regs[] =
{
#include "80586.reg"
	{ NULL,		0, 0 }
};

ArchInstruction arch_i586_set[] =
{
#include "80586.ins"
	{ NULL,		0x0000, AO_NONE, 0, 0, 0 }
};

ArchPlugin arch_plugin =
{
	arch_i586_regs,
	arch_i586_set
};
