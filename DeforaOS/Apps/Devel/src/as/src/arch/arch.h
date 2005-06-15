/* arch/arch.h */



#ifndef __ARCH_ARCH_H
# define __ARCH_ARCH_H


/* types */
typedef enum _ArchOperands {
	AO_NONE,
	AO_IMM,
	AO_REG,
	AO_IMM_IMM,
	AO_IMM_REG,
	AO_REG_IMM,
	AO_REG_REG
} ArchOperands;

typedef struct _ArchInstruction
{
	char * name;
	unsigned int opcode;
	ArchOperands operands;
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


/* Arch */
Arch * arch_new(char * arch);
void arch_delete(Arch * arch);


/* ArchOperands */
int archoperands_count(ArchOperands operands);

#endif /* !__ARCH_ARCH_H */
