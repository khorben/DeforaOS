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



#ifndef AS_CODE_H
# define AS_CODE_H

# include <stdio.h>
# include "token.h"
# include "arch/arch.h"
# include "format/format.h"


/* types */
typedef struct _Code Code;

typedef struct _CodeOperand
{
	TokenCode type;
	int dereference;
	void * value;
} CodeOperand;


/* functions */
Code * code_new(char const * arch, char const * format, char const * filename);
int code_delete(Code * code);

/* useful */
int code_function(Code * code, char const * function);
int code_instruction(Code * code, char const * instruction,
		CodeOperand operands[], size_t operands_cnt);
int code_section(Code * code, char const * section);

#endif /* !AS_CODE_H */
