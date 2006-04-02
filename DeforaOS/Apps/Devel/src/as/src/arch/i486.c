/* i486.c */



#include <stddef.h>
#include "arch.h"


/* types */
#define REG(name, size, id) AO_ ## name = (id | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__),
enum
{
#include "common.reg"
#include "80486.reg"
};


/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
ArchRegister arch_i486_regs[] =
{
#include "80486.reg"
	{ NULL,		0, 0 }
};

ArchInstruction arch_i486_set[] =
{
#include "80486.ins"
	{ NULL,		0x0000, AO_NONE, 0, 0, 0 }
};

ArchPlugin arch_plugin =
{
	arch_i486_regs,
	arch_i486_set
};
