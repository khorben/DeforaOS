/* arch.h */



#ifndef __ARCH_H
# define __ARCH_H


/* types */
typedef struct _ArchInstruction {
	char * string;
	unsigned int opcode;
	unsigned int operand_count;
	/* FIXME type of data relevant to this instruction */
} ArchInstruction;

typedef ArchInstruction * ArchInstructionSet;

typedef struct _ArchRegister {
	char * name;
	unsigned int size;
} ArchRegister;

typedef ArchRegister * ArchRegisterSet;

typedef struct _Arch {
	ArchInstructionSet instructions;
	ArchRegisterSet registers;
} Arch;

#endif /* !__ARCH_H */
