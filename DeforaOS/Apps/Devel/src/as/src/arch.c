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



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "As/arch.h"
#include "arch.h"
#include "../config.h"


/* Arch */
/* private */
/* types */
struct _Arch
{
	char * name;
	ArchRegister * registers;
	size_t registers_cnt;
	ArchInstruction * instructions;
	size_t instructions_cnt;
	char const * format;
	Plugin * handle;
};


/* public */
/* functions */
/* arch_new */
Arch * arch_new(char const * arch)
{
	Arch * a;
	Plugin * handle;
	ArchPlugin * plugin;

	if((handle = plugin_new(LIBDIR, PACKAGE, "arch", arch)) == NULL)
		return NULL;
	if((plugin = plugin_lookup(handle, "arch_plugin")) == NULL)
	{
		plugin_delete(handle);
		return NULL;
	}
	if((a = object_new(sizeof(*a))) == NULL
			|| (a->name = string_new(arch)) == NULL)
	{
		object_delete(a);
		plugin_delete(handle);
		return NULL;
	}
	a->instructions_cnt = 0;
	if((a->instructions = plugin->instructions) != NULL)
		for(; a->instructions[a->instructions_cnt].name != NULL;
				a->instructions_cnt++);
	a->registers_cnt = 0;
	if((a->registers = plugin->registers) != NULL)
		for(; a->registers[a->registers_cnt].name != NULL;
				a->registers_cnt++);
	a->format = plugin->format;
	a->handle = handle;
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	plugin_delete(arch->handle);
	string_delete(arch->name);
	object_delete(arch);
}


/* accessors */
/* arch_get_format */
char const * arch_get_format(Arch * arch)
{
	if(arch->format != NULL)
		return arch->format;
	return "elf";
}


/* arch_get_name */
char const * arch_get_name(Arch * arch)
{
	return arch->name;
}


/* arch_instruction_get */
ArchInstruction * arch_instruction_get(Arch * arch, size_t index)
{
	if(index >= arch->instructions_cnt)
		return NULL;
	return &arch->instructions[index];
}


/* arch_instruction_get_by_name */
ArchInstruction * arch_instruction_get_by_name(Arch * arch, char const * name)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(arch, \"%s\")\n", __func__, name);
#endif
	for(i = 0; i < arch->instructions_cnt; i++)
		if(strcmp(arch->instructions[i].name, name) == 0)
			return &arch->instructions[i];
	return NULL;
}


/* arch_instruction_get_by_opcode */
ArchInstruction * arch_instruction_get_by_opcode(Arch * arch, uint8_t size,
		unsigned long opcode)
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(arch, %u, 0x%lx)\n", __func__, size, opcode);
#endif
	for(i = 0; i < arch->instructions_cnt; i++)
		if(arch->instructions[i].size == size
				&& arch->instructions[i].opcode == opcode)
			return &arch->instructions[i];
#if 0 /* XXX this is experimental and may not be adequate */
	for(i = 0; i < arch->instructions_cnt; i++)
		if(arch->instructions[i].size == 0
				&& arch->instructions[i].op1size == size)
			return &arch->instructions[i];
#endif
	return NULL;
}


/* arch_register_get */
ArchRegister * arch_register_get(Arch * arch, size_t index)
{
	if(index >= arch->registers_cnt)
		return NULL;
	return &arch->registers[index];
}


/* arch_register_get_by_id */
ArchRegister * arch_register_get_by_id(Arch * arch, unsigned int id)
{
	size_t i;

	for(i = 0; i < arch->registers_cnt; i++)
		if(arch->registers[i].id == id)
			return &arch->registers[i];
	return NULL;
}


/* ArchOperands */
int archoperands_count(int operands)
{
	if(operands & _AO_OP__)
		return 3;
	if(operands & _AO_OP_)
		return 2;
	if(operands & _AO_OP)
		return 1;
	assert(operands == 0);
	return 0;
}
