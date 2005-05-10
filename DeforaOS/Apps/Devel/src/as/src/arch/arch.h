/* arch/arch.h */



#ifndef __ARCH_ARCH_H
# define __ARCH_ARCH_H


/* types */
typedef struct _ArchInstruction
{
	char * name;
	unsigned int opcode;
	unsigned int operands_cnt;
	/* FIXME type of data relevant to this instruction */
} ArchInstruction;
typedef ArchInstruction * ArchInstructionSet;

typedef struct _ArchRegister
{
	char * name;
	unsigned int size;
} ArchRegister;
typedef ArchRegister * ArchRegisterSet;

typedef struct _ArchPlugin
{
	ArchInstructionSet instructions;
	ArchRegisterSet registers;
} ArchPlugin;

typedef struct _Arch
{
	ArchInstructionSet instructions;
	ArchRegisterSet registers;
	void * plugin;
} Arch;


/* functions */
Arch * arch_new(char * arch);
void arch_delete(Arch * arch);

#endif /* !__ARCH_ARCH_H */
