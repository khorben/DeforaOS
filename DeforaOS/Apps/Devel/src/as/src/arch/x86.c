/* x86.c */



#include <stddef.h>
#include "arch.h"


/* variables */
ArchInstruction arch_x86_set[] =
{
	{ "adc",  0x0010, 2 },
	{ "adc",  0x0011, 2 },
	{ "adc",  0x0012, 2 },
	{ "adc",  0x0013, 2 },
	{ "adc",  0x0014, 2 },
	{ "adc",  0x0015, 2 },
	{ "add",  0x0000, 2 },
	{ "add",  0x0001, 2 },
	{ "add",  0x0002, 2 },
	{ "add",  0x0003, 2 },
	{ "add",  0x0004, 2 },
	{ "add",  0x0005, 2 },
	{ "and",  0x0020, 2 },
	{ "and",  0x0021, 2 },
	{ "and",  0x0022, 2 },
	{ "and",  0x0023, 2 },
	{ "and",  0x0024, 2 },
	{ "and",  0x0025, 2 },
	{ "clc",  0x00f8, 0 },
	{ "cld",  0x00fc, 0 },
	{ "cli",  0x00fa, 0 },
	{ "hlt",  0x00f4, 0 },
	{ "lea",  0x008d, 0 },
	{ "nop",  0x0090, 0 },
	{ "pop",  0x0007, 1 },
	{ "pop",  0x0017, 1 },
	{ "push", 0x0006, 1 },
	{ "push", 0x0016, 1 },
	{ "stc",  0x00f9, 0 },
	{ "std",  0x00fd, 0 },
	{ "sti",  0x00fb, 0 },
	{ NULL,   0x0000, 0 }
};

Arch arch_x86 = {
	arch_x86_set
};
