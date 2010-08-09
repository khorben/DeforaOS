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



#ifndef AS_CODE_H
# define AS_CODE_H

# include <stdio.h>
# include "token.h"
# include "arch/arch.h"
# include "format/format.h"
# include "as.h"


/* types */
typedef struct _Code Code;


/* functions */
Code * code_new(char const * arch, char const * format, char const * filename);
int code_delete(Code * code);

/* useful */
int code_function(Code * code, char const * function);
int code_instruction(Code * code, char const * instruction,
		AsOperand * operands[], size_t operands_cnt);
int code_section(Code * code, char const * section);

#endif /* !AS_CODE_H */
