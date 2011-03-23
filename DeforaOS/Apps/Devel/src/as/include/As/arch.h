/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#ifndef DEVEL_AS_ARCH_H
# define DEVEL_AS_ARCH_H

# include <stdint.h>


/* AsArch */
/* types */
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

typedef struct _ArchRegister
{
	char * name;
	unsigned int size;
	unsigned int id;
} ArchRegister;

typedef struct _ArchPlugin
{
	char const * format;
	ArchRegister * registers;
	ArchInstruction * instructions;
} ArchPlugin;

#endif /* !DEVEL_AS_ARCH_H */
