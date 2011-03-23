/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "arch.h"
#include "format.h"
#include "common.h"
#include "code.h"


/* Code */
/* private */
/* types */
struct _Code
{
	Arch * arch;
	Format * format;
	char * filename;
	FILE * fp;
};


/* functions */
Code * code_new(char const * arch, char const * format)
{
	Code * code;

	if((code = object_new(sizeof(*code))) == NULL)
		return NULL;
	memset(code, 0, sizeof(*code));
	if((code->arch = arch_new(arch)) != NULL && format == NULL)
		format = arch_get_format(code->arch);
	if(format != NULL)
		code->format = format_new(format, arch);
	if(code->arch == NULL || code->format == NULL)
	{
		code_delete(code);
		return NULL;
	}
	return code;
}


/* code_delete */
int code_delete(Code * code)
{
	int ret = 0;

	if(code->format != NULL)
		format_delete(code->format);
	if(code->arch != NULL)
		arch_delete(code->arch);
	if(code->fp != NULL && fclose(code->fp) != 0)
		ret |= error_set_code(2, "%s: %s", code->filename, strerror(
					errno));
	string_delete(code->filename);
	object_delete(code);
	return ret;
}


/* accessors */
/* code_get_arch */
char const * code_get_arch(Code * code)
{
	return arch_get_name(code->arch);
}


/* code_get_format */
char const * code_get_format(Code * code)
{
	return format_get_name(code->format);
}


/* useful */
/* code_close */
int code_close(Code * code)
{
	int ret;

	ret = format_exit(code->format);
	if(fclose(code->fp) != 0)
		ret |= -error_set_code(1, "%s: %s", code->filename,
				strerror(errno));
	return ret;
}


/* code_function */
int code_function(Code * code, char const * function)
{
	return format_function(code->format, function);
}


/* code_instruction */
static int _instruction_instruction(Code * code, ArchInstruction ** ai,
		char const * instruction, AsOperand * operands[],
		size_t operands_cnt);
static int _instruction_operands(Code * code, ArchInstruction * ai,
		AsOperand * operands[], size_t operands_cnt);
static ArchRegister * _operands_register(Arch * arch, char const * name);

int code_instruction(Code * code, char const * instruction,
		AsOperand * operands[], size_t operands_cnt)
	/* FIXME being rewritten */
{
	int ret;
	ArchInstruction * ai;
	size_t i;
	void * buf;
	size_t size;
	unsigned long u;
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
			if(ai->size == 3) /* XXX make this generic */
				u32 = htonl(ai->opcode << 8);
			else
				u32 = htonl(ai->opcode);
			buf = &u32;
			break;
	}
	if(ai->size != 0 && fwrite(buf, ai->size, 1, code->fp) != 1)
		return -error_set_code(1, "%s: %s", code->filename, strerror(
					errno));
	for(i = 0; i < operands_cnt; i++)
	{
		if(i >= 3)
			break; /* FIXME this is ugly */
		size = (i == 0) ? ai->op1size : ((i == 1) ? ai->op2size
				: ai->op3size);
		if(size == 0)
			continue;
		u = *(unsigned long*)operands[i]->value;
		switch(operands[i]->type)
		{
			case AOT_IMMEDIATE:
				/* FIXME there still is an endian problem */
				switch(size)
				{
					case 1:
						u8 = u;
						buf = &u8;
						break;
					case 2:
						u16 = u;
						buf = &u16;
						break;
					default: /* FIXME not always so */
					case 4:
						buf = &u;
						break;
				}
				break;
			case AOT_REGISTER:
			default:
				/* FIXME really implement */
				buf = NULL;
				break;
		}
		if(buf != NULL && fwrite(buf, size, 1, code->fp) != 1)
			return -error_set_code(1, "%s: %s", code->filename,
					strerror(errno));
	}
	return 0;
}

static int _instruction_instruction(Code * code, ArchInstruction ** ai,
		char const * instruction, AsOperand * operands[],
		size_t operands_cnt)
{
	size_t i;
	int cmp;
	int found = 0;

	/* FIXME check */
	for(i = 0; ((*ai) = arch_instruction_get(code->arch, i)) != NULL; i++)
	{
		/* FIXME alphabetical order assumption disabled for 80x86 */
		if((cmp = strcmp(instruction, (*ai)->name)) != 0)
			continue;
		found = 1;
		if(_instruction_operands(code, *ai, operands, operands_cnt)
				!= 0)
			continue;
		return 0;
	}
	return error_set_code(1, "%s \"%s\"", found ? "Invalid arguments to"
		: "Unknown instruction", instruction);
}

static int _instruction_operands(Code * code, ArchInstruction * ai,
		AsOperand * operands[], size_t operands_cnt)
{
	unsigned long op = 0;
	unsigned long o;
	char const * reg;
	size_t i;
	ArchRegister * ar;

	for(i = 0; i < operands_cnt; i++)
	{
		switch(operands[i]->type)
		{
			case AOT_IMMEDIATE:
				/* FIXME also check the operand size */
				o = _AO_IMM;
#ifdef DEBUG
				fprintf(stderr, "DEBUG: op %zu: imm; ", i);
#endif
				break;
			case AOT_REGISTER:
#if 0 /* XXX this looked maybe better */
				reg = operands[i].value + 1; /* "%rg" => "rg" */
#else
				reg = operands[i]->value;
#endif
				if((ar = _operands_register(code->arch, reg))
						== NULL)
					return 1;
				if(operands[i]->dereference)
					o = (_AO_DREG | (ar->id << 2));
				else
					o = (_AO_REG | (ar->id << 2));
#ifdef DEBUG
				fprintf(stderr, "DEBUG: op %zu: reg %s; ", i,
						reg);
#endif
				break;
			default:
				o = 0;
				break;
		}
		op |= o << (i * 8);
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: 0x%lx & 0x%x => 0x%lx\n", op, ai->operands,
			op & ai->operands);
#endif
	return (op == ai->operands) ? 0 : 1;
}

static ArchRegister * _operands_register(Arch * arch, char const * name)
{
	ArchRegister * ret;
	size_t i;

	for(i = 0; (ret = arch_register_get(arch, i)) != NULL; i++)
		if(strcmp(ret->name, name) == 0)
			break;
	return ret;
}


/* code_open */
int code_open(Code * code, char const * filename)
{
	if(code->filename != NULL || code->fp != NULL)
		return -error_set_code(1, "A file is already opened");
	if((code->filename = string_new(filename)) == NULL)
		return -1;
	if((code->fp = fopen(filename, "w+")) == NULL)
		return -error_set_code(1, "%s: %s", filename, strerror(errno));
	if(format_init(code->format, code->filename, code->fp) != 0)
	{
		fclose(code->fp);
		code->fp = NULL;
		string_delete(code->filename);
		code->filename = NULL;
		return -1;
	}
	return 0;
}


/* code_section */
int code_section(Code * code, char const * section)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, section);
#endif
	return format_section(code->format, section);
}
