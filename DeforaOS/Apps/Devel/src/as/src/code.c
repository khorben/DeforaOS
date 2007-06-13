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



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "as.h"
#include "code.h"


/* Code */
/* variables */
char const * code_error[CE_LAST+1] =
{
	"Success",
	"Invalid arguments",
	"Unknown instruction",
	"Write error"
};


/* functions */
Code * code_new(char const * arch, char const * format, char const * filename)
{
	Code * code;

	if((code = malloc(sizeof(Code))) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	code->format = NULL;
	code->fp = NULL;
	if((code->arch = arch_new(arch)) == NULL
			|| (code->format = format_new(format, arch, filename))
			== NULL
			|| (code->fp = fopen(filename, "w+")) == NULL
			|| format_init(code->format, code->fp) != 0)
	{
		if(code->fp != NULL)
		{
			fclose(code->fp);
			if(unlink(filename) != 0)
				as_error(filename, 0);
		}
		if(code->format != NULL)
			format_delete(code->format, code->fp);
		if(code->arch != NULL)
			arch_delete(code->arch);
		free(code);
		return NULL;
	}
	code->filename = filename;
	return code;
}


/* code_delete */
void code_delete(Code * code, int error)
{
	arch_delete(code->arch);
	error |= format_delete(code->format, code->fp);
	if(code->fp != NULL)
	{
		fclose(code->fp);
		if(error != 0 && unlink(code->filename) != 0)
			as_error(code->filename, 0);
	}
	free(code);
}


/* useful */
/* code_instruction */
static CodeError _instruction_instruction(Code * code, ArchInstruction ** ai,
		char * instruction, CodeOperand operands[], int operands_cnt);
CodeError code_instruction(Code * code, char * instruction,
		CodeOperand operands[], int operands_cnt)
/* FIXME being rewritten */
{
	ArchInstruction * ai;
	int ret;
	int i;
	long long buf;
	unsigned long opcode;
	size_t size;

	if((ret = _instruction_instruction(code, &ai, instruction, operands,
					operands_cnt)) != CE_SUCCESS)
		return ret;
#ifdef DEBUG
	fprintf(stderr, "instruction %s, opcode 0x%lx, operands: 0x%x\n",
			instruction, ai->opcode, ai->operands);
#endif
	switch(ai->size)
	{
		case sizeof(short):
			opcode = htons(ai->opcode); /* FIXME provide this */
			break;
		case 3:
			/* FIXME not even portable */
			opcode = htonl(ai->opcode << 8);
			break;
		case sizeof(long):
			opcode = htonl(ai->opcode); /* FIXME provide this */
			break;
		default:
			opcode = ai->opcode;
			break;
	}
	if(fwrite(&opcode, ai->size, 1, code->fp) != 1)
		return CE_WRITE_ERROR;
	if(ai->size == 0) /* FIXME bad definition? */
		return CE_SUCCESS;
	for(i = 0; i < operands_cnt; i++)
	{
		if(i >= 2)
			break;
		size = i == 0 ? ai->op1size : ai->op2size;
		if(size == 0)
			continue;
		memset(&buf, 0, sizeof(buf));
		switch(operands[i].type)
		{
			case TC_IMMEDIATE:
				/* FIXME only valid if size == 4 */
				buf = strtoll(operands[i].value+1, NULL, 0);
				break;
			case TC_REGISTER:
				continue;
			default:
				/* FIXME */
				continue;
		}
		if(fwrite(&buf, size, 1, code->fp) == 1)
			continue;
		return CE_WRITE_ERROR;
	}
	return CE_SUCCESS;
}

static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt);
static CodeError _instruction_instruction(Code * code, ArchInstruction ** ai,
		char * instruction, CodeOperand operands[], int operands_cnt)
{
	int i;
	int cmp;
	int found = 0;

	/* FIXME check */
	for(i = 0; ((*ai) = &(code->arch->instructions[i]))
			&& (*ai)->name != NULL; i++)
	{
		if((cmp = strcmp(instruction, (*ai)->name)) > 0)
			continue;
		if(cmp < 0)
			break;
		found = 1;
		if(_instruction_operands(code, *ai, operands, operands_cnt)
				!= 0)
			continue;
		return CE_SUCCESS;
	}
	return found ? CE_INVALID_ARGUMENTS : CE_UNKNOWN_INSTRUCTION;
}

static ArchRegister * _operands_register(ArchRegister * registers, char * name);
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
			case TC_IMMEDIATE:
				op |= _AO_IMM;
#ifdef DEBUG
				fprintf(stderr, "op %d: imm; ", i);
#endif
				break;
			case TC_REGISTER:
				reg = operands[i].value + 1; /* "%rg" => "rg" */
				ar = code->arch->registers;
				if((ar = _operands_register(ar, reg)) == NULL)
					return 1;
				op |= (_AO_REG | (ar->id << 2));
#ifdef DEBUG
				fprintf(stderr, "op %d: reg %s; ", i, reg);
#endif
				break;
			default:
				break;
		}
	}
#ifdef DEBUG
	fprintf(stderr, "0x%lx & 0x%x => 0x%lx\n", op, ai->operands,
			op & ai->operands);
#endif
	return op == ai->operands ? 0 : 1;
}

static ArchRegister * _operands_register(ArchRegister * registers, char * name)
{
	int i;

	for(i = 0; registers[i].name != NULL; i++)
		if(strcmp(registers[i].name, name) == 0)
			return &registers[i];
	return NULL;
}
