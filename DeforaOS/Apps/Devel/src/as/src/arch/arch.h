/* arch.h */



#ifndef __ARCH_H
# define __ARCH_H


/* types */
typedef struct _ArchInstruction {
	char * string;
	unsigned int opcode;
	unsigned int operand_count;
} ArchInstruction;

typedef ArchInstruction * ArchInstructionSet;

typedef struct _Arch {
	ArchInstructionSet instructions;
} Arch;

#endif /* !__ARCH_H */
