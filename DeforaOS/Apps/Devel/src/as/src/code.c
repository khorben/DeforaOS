/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "as.h"
#include "common.h"
#include "code.h"


/* Code */
/* private */
/* types */
struct _Code
{
	Arch * arch;
	Format * format;
	char const * filename;
	FILE * fp;
};


/* functions */
Code * code_new(char const * arch, char const * format, char const * filename)
{
	Code * code;

	if((code = object_new(sizeof(*code))) == NULL)
		return NULL;
	memset(code, 0, sizeof(*code));
	if((code->fp = fopen(filename, "w+")) == NULL
			|| (code->arch = arch_new(arch)) == NULL
			|| (code->format = format_new(format, arch, filename,
					code->fp)) == NULL)
	{
		if(code->fp == NULL)
			error_set_code(1, "%s: %s", filename, strerror(errno));
		else if(unlink(filename) != 0)
			perror(filename);
		code_delete(code);
		return NULL;
	}
	code->filename = filename;
	return code;
}


/* code_delete */
int code_delete(Code * code)
{
	int ret = 0;

	if(code->format != NULL)
		ret = format_delete(code->format);
	if(code->arch != NULL)
		arch_delete(code->arch);
	if(code->fp != NULL && fclose(code->fp) != 0)
		ret |= error_set_code(2, "%s: %s", code->filename, strerror(
					errno));
	object_delete(code);
	return ret;
}


/* useful */
/* code_function */
int code_function(Code * code, char const * function)
{
	return format_function(code->format, function);
}


/* code_instruction */
static int _instruction_instruction(Code * code, ArchInstruction ** ai,
		char const * instruction, CodeOperand operands[],
		size_t operands_cnt);
int code_instruction(Code * code, char const * instruction,
		CodeOperand operands[], size_t operands_cnt)
	/* FIXME being rewritten */
{
	int ret;
	ArchInstruction * ai;
	size_t i;
	void * buf;
	size_t size;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;

	if((ret = _instruction_instruction(code, &ai, instruction, operands,
					operands_cnt)) != 0)
		return ret;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: instruction %s, opcode 0x%lx, operands: 0x%x\n",
			instruction, ai->opcode, ai->operands);
#endif
	switch(ai->size)
	{
		case sizeof(u8):
			u8 = ai->opcode;
			buf = &u8;
			break;
		case sizeof(u16):
			u16 = htons(ai->opcode);
			buf = &u16;
			break;
		case sizeof(u32):
		default:
			u32 = htonl(ai->opcode);
			buf = &u32;
			break;
	}
#if 0
	if(ai->size == 0) /* FIXME bad definition? */
		return CE_SUCCESS;
#endif
	if(fwrite(buf, ai->size, 1, code->fp) != 1)
		return error_set_code(1, "%s: %s", code->filename, strerror(
					errno));
	for(i = 0; i < operands_cnt; i++)
	{
		if(i >= 2)
			break; /* FIXME looks ugly */
		size = (i == 0) ? ai->op1size : ai->op2size;
		if(size == 0)
			continue;
		switch(operands[i].type)
		{
			case AS_CODE_IMMEDIATE:
				/* FIXME only valid if size == 4 */
				u32 = strtoll(operands[i].value + 1, NULL, 0);
				buf = &u32;
				break;
			case AS_CODE_REGISTER:
				continue;
			default:
				/* FIXME */
				continue;
		}
		if(fwrite(buf, size, 1, code->fp) == 1)
			continue;
		return error_set_code(1, "%s: %s", code->filename, strerror(
					errno));
	}
	return 0;
}

static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt);
static int _instruction_instruction(Code * code, ArchInstruction ** ai,
		char const * instruction, CodeOperand operands[],
		size_t operands_cnt)
{
	size_t i;
	int cmp;
	int found = 0;

	/* FIXME check */
	for(i = 0; ((*ai) = arch_instruction_get(code->arch, i)) != NULL; i++)
	{
		if((cmp = strcmp(instruction, (*ai)->name)) > 0)
			continue;
		if(cmp < 0)
			break;
		found = 1;
		if(_instruction_operands(code, *ai, operands, operands_cnt)
				!= 0)
			continue;
		return 0;
	}
	return error_set_code(1, "%s \"%s\"", found ? "Invalid arguments to"
		: "Unknown instruction", instruction);
}

static ArchRegister * _operands_register(Arch * arch, char * name);
static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt)
{
	unsigned long op = 0;
	char * reg;
	int i;
	ArchRegister * ar;

	for(i = 0; i < operands_cnt; i++)
	{
		op = op << 8;
		switch(operands[i].type)
		{
			case AS_CODE_IMMEDIATE:
			case AS_CODE_NUMBER:
				op |= _AO_IMM;
#ifdef DEBUG
				fprintf(stderr, "DEBUG: op %d: imm; ", i);
#endif
				break;
			case AS_CODE_REGISTER:
#if 0
				reg = operands[i].value + 1; /* "%rg" => "rg" */
#endif
				reg = operands[i].value;
				if((ar = _operands_register(code->arch, reg))
						== NULL)
					return 1;
				op |= (_AO_REG | (ar->id << 2));
#ifdef DEBUG
				fprintf(stderr, "DEBUG: op %d: reg %s; ", i,
						reg);
#endif
				break;
			default:
				break;
		}
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: 0x%lx & 0x%x => 0x%lx\n", op, ai->operands,
			op & ai->operands);
#endif
	return (op == ai->operands) ? 0 : 1;
}

static ArchRegister * _operands_register(Arch * arch, char * name)
{
	ArchRegister * ret;
	size_t i;

	for(i = 0; (ret = arch_register_get(arch, i)) != NULL; i++)
		if(strcmp(ret->name, name) == 0)
			break;
	return ret;
}


/* code_section */
int code_section(Code * code, char const * section)
{
	return format_section(code->format, section);
}
