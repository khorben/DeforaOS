/* code.h */



#ifndef __CODE_H
# define __CODE_H

# include <stdio.h>
# include "token.h"
# include "arch/arch.h"
# include "format/format.h"


/* types */
typedef struct _Code
{
	Arch * arch;
	Format * format;
	char * filename;
	FILE * fp;
} Code;

typedef enum _CodeError { CE_SUCCESS = 0, CE_UNKNOWN_INSTRUCTION,
	CE_INVALID_ARGUMENTS } CodeError;
# define CE_LAST (CE_INVALID_ARGUMENTS+1)

typedef struct _CodeOperand
{
	TokenCode type;
	void * value;
} CodeOperand;


/* variables */
char const * code_error[CE_LAST];


/* functions */
Code * code_new(char * arch, char * format, char * filename);
void code_delete(Code * code, int error);

/* useful */
CodeError code_instruction(Code * code, char * instruction,
		CodeOperand operands[], unsigned int operands_cnt);

#endif /* !__CODE_H */
