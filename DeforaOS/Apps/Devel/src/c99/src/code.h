/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
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



#ifndef _C99_CODE_H
# define _C99_CODE_H

# include "c99.h"


/* Code */
/* protected */
typedef struct _Code Code;


/* public */
/* types */
typedef enum _CodeContext
{
	CODE_CONTEXT_NULL = 0,
	CODE_CONTEXT_ASSIGNMENT,
	CODE_CONTEXT_DECLARATION,
	CODE_CONTEXT_DECLARATION_BEGIN,
	CODE_CONTEXT_DECLARATION_END,
	CODE_CONTEXT_DECLARATION_OR_FUNCTION,
	CODE_CONTEXT_DECLARATION_PARAMETERS,
	CODE_CONTEXT_ENUMERATION_CONSTANT,
	CODE_CONTEXT_ENUMERATION_NAME,
	CODE_CONTEXT_ENUMERATION_VALUE,
	CODE_CONTEXT_FUNCTION,
	CODE_CONTEXT_FUNCTION_BEGIN,
	CODE_CONTEXT_FUNCTION_CALL,
	CODE_CONTEXT_FUNCTION_END,
	CODE_CONTEXT_FUNCTION_PARAMETERS,
	CODE_CONTEXT_LABEL,
	CODE_CONTEXT_PARAMETERS,
	CODE_CONTEXT_PARAMETERS_TYPE,
	CODE_CONTEXT_PRIMARY_EXPR,
	CODE_CONTEXT_STATEMENT,
	CODE_CONTEXT_STRUCT,
	CODE_CONTEXT_TYPEDEF_NAME,
	CODE_CONTEXT_UNION
} CodeContext;
# define CODE_CONTEXT_LAST	CODE_CONTEXT_UNION
# define CODE_CONTEXT_COUNT	(CODE_CONTEXT_LAST + 1)

typedef enum _CodeStorage
{
	CODE_STORAGE_NULL	= 0x0,
	CODE_STORAGE_TYPEDEF	= 0x1,
	CODE_STORAGE_EXTERN	= 0x2,
	CODE_STORAGE_STATIC	= 0x4,
	CODE_STORAGE_AUTO	= 0x8,
	CODE_STORAGE_REGISTER	= 0x10
} CodeStorage;
# define CODE_STORAGE_LAST	CODE_STORAGE_REGISTER
# define CODE_STORAGE_COUNT	(CODE_STORAGE_LAST + 1)

typedef enum _CodeClass
{
	CODE_CLASS_NULL		= 0x0000,
	CODE_CLASS_VOID		= 0x0001,
	CODE_CLASS_CHAR		= 0x0002,
	CODE_CLASS_SHORT	= 0x0004,
	CODE_CLASS_INT		= 0x0008,
	CODE_CLASS_LONG		= 0x0010,
	CODE_CLASS_LONG_LONG	= 0x0020,
	CODE_CLASS_FLOAT	= 0x0040,
	CODE_CLASS_DOUBLE	= 0x0080,
	CODE_CLASS_SIGNED	= 0x0100,
	CODE_CLASS_UNSIGNED	= 0x0200,
	CODE_CLASS__BOOL	= 0x0400,
	CODE_CLASS__COMPLEX	= 0x0800,
	CODE_CLASS__IMAGINARY	= 0x1000
} CodeClass;
# define CODE_CLASS_LAST	CODE_CLASS__IMAGINARY
# define CODE_CLASS_COUNT	(CODE_CLASS_LAST + 1)


/* functions */
Code * code_new(C99Prefs const * prefs, C99Helper * helper,
		char const * outfile);
int code_delete(Code * code);

/* useful */
/* parsing */
int code_token(Code * code, Token * token);

/* context */
CodeContext code_context_get(Code * code);
int code_context_set(Code * code, CodeContext context);
int code_context_set_class(Code * code, CodeClass cclass);
int code_context_set_identifier(Code * code, char const * identifier);
int code_context_set_storage(Code * code, CodeStorage storage);

/* functions */
int code_function_begin(Code * code, char const * name);
int code_function_call(Code * code, char const * name);
int code_function_end(Code * code);

/* labels */
int code_label_set(Code * code, char const * name);

/* scope */
int code_scope_push(Code * code);
int code_scope_pop(Code * code);

/* types */
int code_type_add(Code * code, char const * name);
int code_type_get(Code * code, char const * name);

/* variables */
int code_variable_add(Code * code, char const * name);

#endif /* !_C99_CODE_H */
