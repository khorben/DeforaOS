/* i386.c */



#include <stddef.h>
#include "arch.h"


/* types */
#define REG(name, size, id) AO_ ## name = ((id << 2) | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__),
enum {
#include "common.reg"
#include "80386.reg"
};

/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
ArchRegister arch_i386_regs[] =
{
#include "80386.reg"
	{ NULL,		0, 0 }
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
