/* arch/arch.h */



#ifndef __ARCH_ARCH_H
# define __ARCH_ARCH_H


/* types */
typedef struct _ArchRegister
{
	char * name;
	unsigned int size;
} ArchRegister;

typedef struct _ArchInstruction
{
	char * name;
	unsigned int opcode;
	int operands;
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
