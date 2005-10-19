/* code.c */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "as.h"
#include "arch/arch.h"
#include "code.h"


/* Code */
char const * code_error[CE_LAST] = {
	"Success",
	"Invalid arguments",
	"Unknown instruction",
	"Write error"
};


Code * code_new(char * arch, char * format, char * filename)
{
	Code * c;

	if((c = malloc(sizeof(Code))) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	c->format = NULL;
	c->fp = NULL;
	if((c->arch = arch_new(arch)) == NULL
			|| (c->format = format_new(format, arch)) == NULL
			|| (c->fp = fopen(filename, "w")) == NULL
			|| format_init(c->format, c->fp) != 0)
	{
		if(c->fp != NULL)
		{
			fclose(c->fp);
			if(unlink(filename) != 0)
				as_error(filename, 0);
		}
		if(c->format != NULL)
			format_delete(c->format);
		if(c->arch != NULL)
			arch_delete(c->arch);
		free(c);
		return NULL;
	}
	c->filename = filename;
	return c;
}


/* code_delete */
void code_delete(Code * code, int error)
{
	arch_delete(code->arch);
	format_delete(code->format);
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
static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt);
CodeError code_instruction(Code * code, char * instruction,
		CodeOperand operands[], int operands_cnt)
{
	int i;
	ArchInstruction * ai;
	int cmp;
	int found = 0;

	for(i = 0; (ai = &(code->arch->instructions[i])) && ai->name != NULL;
			i++)
	{
		if((cmp = strcmp(instruction, ai->name)) > 0)
			continue;
		if(cmp < 0)
			break;
		found = 1;
		if(_instruction_operands(code, ai, operands, operands_cnt) != 0)
			continue;
		if(fwrite(&ai->opcode, sizeof(char), 1, code->fp)
				!= sizeof(char))
			return CE_WRITE_ERROR;
		for(i = 0; i < operands_cnt; i++)
		{
			if(operands[i].type != TC_IMMEDIATE)
				continue;
			if(fwrite(&operands[i], sizeof(char), 1, code->fp)
					!= sizeof(char))
				return CE_WRITE_ERROR;
		}
		return 0;
	}
	return found ? CE_INVALID_ARGUMENTS : CE_UNKNOWN_INSTRUCTION;
}

static ArchRegister * _operands_register(ArchRegister * registers, char * name);
static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt)
{
	unsigned int op = 0;
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
				reg = operands[i].value + 1;
				if((ar = _operands_register(code->arch->registers, reg)) == NULL)
					return 1;
				op |= (_AO_REG | (ar->id << 2));
#ifdef DEBUG
				fprintf(stderr, "op %d: reg; ", i);
#endif
				break;
			default:
				assert(0);
				break;
		}
	}
#ifdef DEBUG
	fprintf(stderr, "%d & %d => %d\n", op, ai->operands, op & ai->operands);
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

/* static int _operands_compare(Code * code, int operand, CodeOperand * co);
static int _instruction_operands(Code * code, ArchInstruction * ai,
		CodeOperand operands[], int operands_cnt)
{
	int i;
	int op;
	int ai_op;

	if(operands_cnt != archoperands_count(ai->operands))
		return 1;
	for(i = 0; i < operands_cnt; i++)
	{
		op = (operands_cnt - i - 1);
		ai_op = (ai->operands >> (op * 8));
		if(_operands_compare(code, ai_op, &operands[i]) != 0)
			break;
	}
	return i == operands_cnt ? 0 : 1;
}

static int _operands_compare(Code * code, int operand, CodeOperand * co)
{
	int co_op = 0;
	char * reg;
	ArchRegister * ar;
	int i;

	switch(co->type)
	{
		case TC_REGISTER:
			reg = co->value;
			reg++;
			for(i = 0; (ar = &code->arch->registers[i]) != NULL;
					i++)
				if(strcmp(reg, ar->name) == 0)
				{
					co_op |= (_AO_REG | ar->id);
					break;
				}
			if(co_op == 0)
				return 1;
			printf("Found opcode for register: %s (%d)\n", reg, ar->id);
			break;
		case TC_IMMEDIATE:
			co_op |= _AO_IMM;
			break;
		default:
			return 1;
	}
	printf("Comparing %d to %d (%d)\n", operand, co_op, operand & co_op);
	return operand & co_op ? 0 : 1;
} */
