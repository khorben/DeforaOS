/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef AS_ARCH_ARCH_H
# define AS_ARCH_ARCH_H

# include <stdint.h>


/* types */
typedef struct _ArchRegister
{
	char * name;
	unsigned int size;
	unsigned int id;
} ArchRegister;

typedef unsigned int ArchOperands;
# define _AO_NONE	(00)
# define _AO_IMM	(01)
# define _AO_REG	(02)
# define _AO_DREG	(03)
# define _AO_OP		(03)
# define _AO_IMM_	(_AO_IMM << 8)
# define _AO_REG_	(_AO_REG << 8)
# define _AO_DREG_	(_AO_DREG << 8)
# define _AO_OP_	(_AO_OP  << 8)
# define _AO_IMM__	(_AO_IMM_ << 8)
# define _AO_REG__	(_AO_REG_ << 8)
# define _AO_DREG__	(_AO_DREG_ << 8)
# define _AO_OP__	(_AO_OP_  << 8)

typedef struct _ArchInstruction
{
	char * name;
	unsigned long opcode;
	ArchOperands operands;
	uint8_t size;
	uint8_t op1size;
	uint8_t op2size;
	uint8_t op3size;
} ArchInstruction;

typedef struct _ArchPlugin
{
	ArchRegister * registers;
	ArchInstruction * instructions;
} ArchPlugin;


/* Arch */
/* types */
typedef struct _Arch Arch;

/* functions */
Arch * arch_new(char const * arch);
void arch_delete(Arch * arch);


/* accessors */
ArchInstruction * arch_instruction_get(Arch * arch, size_t index);
ArchInstruction * arch_instruction_get_by_opcode(Arch * arch,
		unsigned long opcode);
ArchRegister * arch_register_get(Arch * arch, size_t index);


/* ArchOperands */
int archoperands_count(int operands);

#endif /* !AS_ARCH_ARCH_H */
