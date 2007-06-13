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



#ifndef AS_CODE_H
# define AS_CODE_H

# include <stdio.h>
# include "token.h"
# include "arch/arch.h"
# include "format/format.h"


/* types */
typedef struct _Code
{
	Arch * arch;
	Format * format;
	char const * filename;
	FILE * fp;
} Code;

typedef enum _CodeError
{
	CE_SUCCESS = 0, CE_INVALID_ARGUMENTS,
	CE_UNKNOWN_INSTRUCTION, CE_WRITE_ERROR
} CodeError;
# define CE_LAST (CE_WRITE_ERROR)

typedef struct _CodeOperand
{
	TokenCode type;
	void * value;
} CodeOperand;


/* variables */
extern char const * code_error[CE_LAST+1];


/* functions */
Code * code_new(char const * arch, char const * format, char const * filename);
void code_delete(Code * code, int error);

/* useful */
CodeError code_instruction(Code * code, char * instruction,
		CodeOperand operands[], int operands_cnt);

#endif /* !AS_CODE_H */
