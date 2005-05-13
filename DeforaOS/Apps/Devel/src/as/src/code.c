/* code.c */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
	if((c->arch = arch_new(arch)) == NULL
			|| (c->format = format_new(format)) == NULL
			|| (c->fp = fopen(filename, "w")) == NULL
			|| format_init(c->format, c->fp) != 0)
	{
		if(c->arch != NULL)
			arch_delete(c->arch);
		if(c->format != NULL)
			format_delete(c->format);
		if(c->fp != NULL)
		{
			fclose(c->fp);
			if(unlink(filename) != 0)
				as_error(filename, 0);
		}
		else
			as_error(filename, 0);
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
CodeError code_instruction(Code * code, char * instruction,
		CodeOperand operands[], unsigned int operands_cnt)
{
	unsigned int i;
	ArchInstruction * ai;
	int cmp;

	for(i = 0; (ai = &(code->arch->instructions[i])) && ai->name != NULL; i++)
	{
		if((cmp = strcmp(instruction, ai->name)) > 0)
			continue;
		if(cmp < 0)
			break;
		if(operands_cnt != ai->operands_cnt)
			continue;
		/* FIXME check operands types */
#ifdef DEBUG
		fprintf(stderr, "%s%s%s", "DEBUG instruction: ", instruction,
				"\n");
#endif
		if(fwrite(&ai->opcode, sizeof(char), 1, code->fp)
				!= sizeof(char))
			return CE_WRITE_ERROR;
		return CE_SUCCESS;
	}
	return CE_UNKNOWN_INSTRUCTION;
}
