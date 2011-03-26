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



#ifndef AS_ARCH_H
# define AS_ARCH_H

# include <sys/types.h>
# include <stdint.h>
# include "As/arch.h"


/* Arch */
/* public */
/* types */
typedef struct _Arch Arch;


/* functions */
Arch * arch_new(char const * arch);
void arch_delete(Arch * arch);


/* accessors */
char const * arch_get_format(Arch * arch);
char const * arch_get_name(Arch * arch);

ArchInstruction * arch_instruction_get(Arch * arch, size_t index);
ArchInstruction * arch_instruction_get_by_name(Arch * arch, char const * name);
ArchInstruction * arch_instruction_get_by_opcode(Arch * arch, uint8_t size,
		unsigned long opcode);
ArchRegister * arch_register_get(Arch * arch, size_t index);
ArchRegister * arch_register_get_by_id(Arch * arch, unsigned int id);


/* ArchOperands */
int archoperands_count(int operands);

#endif /* !AS_ARCH_H */
