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



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "as.h"
#include "arch.h"
#include "../../config.h"


/* Arch */
/* private */
/* types */
struct _Arch
{
	ArchRegister * registers;
	size_t registers_cnt;
	ArchInstruction * instructions;
	size_t instructions_cnt;
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
	if((a = object_new(sizeof(*a))) == NULL)
	{
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
	a->handle = handle;
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	plugin_delete(arch->handle);
	object_delete(arch);
}


/* accessors */
/* arch_instruction_get */
ArchInstruction * arch_instruction_get(Arch * arch, size_t index)
{
	if(index >= arch->instructions_cnt)
		return NULL;
	return &arch->instructions[index];
}


/* arch_instruction_get_by_opcode */
ArchInstruction * arch_instruction_get_by_opcode(Arch * arch,
		unsigned long opcode)
{
	size_t i;

	for(i = 0; i < arch->instructions_cnt; i++)
		if(arch->instructions[i].opcode == opcode)
			return &arch->instructions[i];
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
