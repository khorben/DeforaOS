/* arch/arch.h */



#ifndef __ARCH_ARCH_H
# define __ARCH_ARCH_H


/* types */
typedef struct _ArchRegister
{
	char * name;
	unsigned int size;
	unsigned int id;
} ArchRegister;

typedef unsigned int ArchOperands;
# define _AO_NONE	(00)
# define _AO_OP		(01)
# define _AO_IMM	(01 | _AO_OP)
# define _AO_REG	(02 | _AO_OP)
# define _AO_OP_	(_AO_OP  << 8)
# define _AO_IMM_	(_AO_IMM << 8)
# define _AO_REG_	(_AO_REG << 8)
# define _AO_OP__	(_AO_OP_  << 8)
# define _AO_IMM__	(_AO_IMM_ << 8)
# define _AO_REG__	(_AO_REG_ << 8)

typedef struct _ArchInstruction
{
	char * name;
	unsigned int opcode;
	ArchOperands operands;
} ArchInstruction;

typedef struct _ArchPlugin
{
	ArchRegister * registers;
	ArchInstruction * instructions;
} ArchPlugin;


/* Arch */
/* types */
typedef struct _Arch
{
	ArchRegister * registers;
	ArchInstruction * instructions;
	void * plugin;
} Arch;

/* functions */
Arch * arch_new(char * arch);
void arch_delete(Arch * arch);


/* ArchOperands */
int archoperands_count(int operands);

#endif /* !__ARCH_ARCH_H */
