/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* FIXME:
 * - check calls to code_context_set() */



#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "tokenset.h"
#include "scanner.h"
#include "c99.h"
#include "../config.h"

#ifdef DEBUG
# define DEBUG_GRAMMAR() fprintf(stderr, "DEBUG: %s(%p) \"%s\"\n", __func__, \
		c99, _parse_get_string(c99))
#else
# define DEBUG_GRAMMAR()
#endif


/* private */
/* prototypes */
static int _parse_check(C99 * c99, C99Code code);
static int _parse_check_set(C99 * c99, TokenSet set, char const * name,
		int (*callback)(C99 * c99));
static int _parse_error(C99 * c99, char const * format, ...);
static int _parse_get_code(C99 * c99);
static char const * _parse_get_string(C99 * c99);
static int _parse_in_set(C99 * c99, TokenSet set);
static int _parse_is_code(C99 * c99, C99Code code);
static int _parse_skip(C99 * c99, TokenSet set);

/* grammar */
static int _translation_unit(C99 * c99);
static int _external_declaration(C99 * c99);
static int _function_definition(C99 * c99);
static int _declaration_specifiers(C99 * c99);
static int _storage_class_specifier(C99 * c99);
static int _type_specifier(C99 * c99);
static int _struct_or_union_specifier(C99 * c99);
static int _struct_or_union(C99 * c99);
static int _struct_declaration_list(C99 * c99);
static int _struct_declaration(C99 * c99);
static int _struct_declarator_list(C99 * c99);
static int _struct_declarator(C99 * c99);
static int _enum_specifier(C99 * c99);
static int _enumerator(C99 * c99);
static int _enumeration_constant(C99 * c99);
static int _typedef_name(C99 * c99);
static int _type_qualifier(C99 * c99);
static int _function_specifier(C99 * c99);
static int _declarator(C99 * c99);
static int _pointer(C99 * c99);
static int _type_qualifier_list(C99 * c99);
static int _direct_declarator(C99 * c99);
static int _identifier(C99 * c99);
static int _identifier_list(C99 * c99);
static int _parameter_type_list(C99 * c99);
static int _parameter_declaration(C99 * c99);
static int _abstract_or_declarator(C99 * c99);
static int _abstract_declarator(C99 * c99);
static int _direct_abstract_declarator(C99 * c99);
static int _assignment_expr(C99 * c99);
static int _unary_expr(C99 * c99);
static int _postfix_expr(C99 * c99);
static int _postfix_expr_do(C99 * c99);
static int _argument_expr_list(C99 * c99);
static int _primary_expr(C99 * c99);
static int _type_name(C99 * c99);
static int _specifier_qualifier_list(C99 * c99);
static int _unary_operator(C99 * c99);
static int _assignment_operator(C99 * c99);
static int _conditional_expr(C99 * c99);
static int _logical_or_expr(C99 * c99);
static int _logical_and_expr(C99 * c99);
static int _inclusive_or_expr(C99 * c99);
static int _exclusive_or_expr(C99 * c99);
static int _and_expr(C99 * c99);
static int _equality_expr(C99 * c99);
static int _relational_expr(C99 * c99);
static int _shift_expr(C99 * c99);
static int _additive_expr(C99 * c99);
static int _multiplicative_expr(C99 * c99);
static int _cast_expr(C99 * c99);
static int _declaration_list(C99 * c99);
static int _declaration(C99 * c99);
static int _declaration_do(C99 * c99);
static int _compound_statement(C99 * c99);
static int _block_item_list(C99 * c99);
static int _block_item(C99 * c99);
static int _statement(C99 * c99);
static int _labeled_statement(C99 * c99);
static int _constant_expr(C99 * c99);
static int _expression_statement(C99 * c99);
static int _expression(C99 * c99);
static int _selection_statement(C99 * c99);
static int _iteration_statement(C99 * c99);
static int _jump_statement(C99 * c99);
static int _init_declarator_list(C99 * c99);
static int _init_declarator(C99 * c99);
static int _initializer(C99 * c99);
static int _initializer_list(C99 * c99);
static int _designation(C99 * c99);
static int _designator_list(C99 * c99);
static int _designator(C99 * c99);


/* functions */
/* parse_check */
static int _parse_check(C99 * c99, C99Code code)
{
	int ret = 0;

	if(!_parse_is_code(c99, code))
	{
		ret = _parse_error(c99, "Expected \"%s\"",
				tokencode_get_string(code));
		/* FIXME is it really good? limit to 3 scans? search ; or } ? */
		while(scan(c99) == 0 && c99->token != NULL /* actual token */
				&& !_parse_is_code(c99, code));
	}
	ret |= scan(c99);
	return ret;
}


/* parse_check_set */
static int _parse_check_set(C99 * c99, TokenSet set, char const * name,
		int (*callback)(C99 * c99))
{
	if(!_parse_in_set(c99, set))
		return _parse_error(c99, "Expected %s", name);
	return callback(c99);
}


/* parse_error */
static int _parse_error(C99 * c99, char const * format, ...)
{
	Token * token = c99->token;
	va_list ap;

	c99->error_cnt++;
	if(token == NULL) /* XXX not very elegant */
		fputs(PACKAGE ": near end of file: error: ", stderr);
	else
		fprintf(stderr, "%s%s:%u, near \"%s\": error: ", PACKAGE ": ",
				token_get_filename(token),
				token_get_line(token), token_get_string(token));
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	return 1;
}


/* parse_get_code */
static int _parse_get_code(C99 * c99)
{
	if(c99->token == NULL)
		return TC_NULL;
	return token_get_code(c99->token);
}


/* parse_get_string */
static char const * _parse_get_string(C99 * c99)
{
	if(c99->token == NULL)
		return "EOF";
	return token_get_string(c99->token);
}


/* parse_in_set */
static int _parse_in_set(C99 * c99, TokenSet set)
{
	if(c99->token == NULL)
		return 0;
	return token_in_set(c99->token, set);
}


/* parse_is_code */
static int _parse_is_code(C99 * c99, C99Code code)
{
	if(c99->token == NULL)
		return 0;
	return token_get_code(c99->token) == code;
}


/* parse_skip */
static int _parse_skip(C99 * c99, TokenSet set)
{
	int ret = 0;

	while(!_parse_in_set(c99, set) && (ret = scan(c99)) == 0
			&& c99->token != NULL);
	return ret;
}


/* grammar */
/* translation-unit */
static int _translation_unit(C99 * c99)
	/* external-declaration { external-declaration } */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(scan(c99) != 0)
		return 1;
	while(c99->token != NULL) /* end of file */
		if(!_parse_in_set(c99, c99set_external_declaration))
		{
			ret |= _parse_error(c99, "Expected declaration");
			ret |= _parse_skip(c99, c99set_external_declaration);
		}
		else
			ret |= _external_declaration(c99);
	if(c99->token != NULL)
		ret |= 1;
	return ret;
}


/* external_declaration
 * PRE	the first token starts a declaration-specifiers */
static int _external_declaration(C99 * c99)
	/* function-definition | declaration */
{
	int ret;

	DEBUG_GRAMMAR();
	code_context_set(c99->code, CODE_CONTEXT_DECLARATION_OR_FUNCTION);
	ret = _declaration_specifiers(c99);
	if(_parse_is_code(c99, C99_CODE_OPERATOR_SEMICOLON))
	{
		code_context_set(c99->code, CODE_CONTEXT_DECLARATION);
		ret |= _declaration_do(c99);
		return ret;
	}
	ret |= _declarator(c99);
	if(_parse_is_code(c99, C99_CODE_OPERATOR_EQUALS))
	{
		code_context_set(c99->code, CODE_CONTEXT_DECLARATION);
		ret |= scan(c99);
		ret |= _initializer(c99);
		while(_parse_is_code(c99, C99_CODE_COMMA))
		{
			ret |= scan(c99);
			ret |= _init_declarator(c99);
		}
		ret |= _declaration_do(c99);
		return ret;
	}
	if(_parse_in_set(c99, c99set_declaration_list))
	{
		code_context_set(c99->code, CODE_CONTEXT_FUNCTION);
		ret |= _declaration_list(c99);
		ret |= _parse_check_set(c99, c99set_compound_statement,
				"function definition", _function_definition);
		return ret;
	}
	if(_parse_in_set(c99, c99set_compound_statement))
	{
		code_context_set(c99->code, CODE_CONTEXT_FUNCTION);
		ret |= _function_definition(c99);
		return ret;
	}
	code_context_set(c99->code, CODE_CONTEXT_DECLARATION);
	if(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		ret |= _init_declarator_list(c99);
	}
	ret |= _declaration_do(c99);
	return ret;
}


/* function_definition
 * PRE	the first token starts a compound-statement */
static int _function_definition(C99 * c99)
	/* compound-statement */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	code_context_set(c99->code, CODE_CONTEXT_FUNCTION_BEGIN);
	ret |= _compound_statement(c99);
	code_context_set(c99->code, CODE_CONTEXT_FUNCTION_END);
	return ret;
}


/* declaration-list */
static int _declaration_list(C99 * c99)
	/* declaration { declaration } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _declaration(c99);
	while(_parse_in_set(c99, c99set_declaration))
		ret |= _declaration(c99);
	return ret;
}


/* declaration */
static int _declaration(C99 * c99)
	/* declaration-specifiers [ init-declarator-list ] ";" */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	ret |= _declaration_specifiers(c99);
	if(_parse_in_set(c99, c99set_init_declarator_list))
		ret |= _init_declarator_list(c99);
	ret |= _declaration_do(c99);
	return ret;
}

/* PRE	a full declaration was just parsed until ";" */
static int _declaration_do(C99 * c99)
{
	int ret;

	ret = _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	ret |= code_context_set(c99->code, CODE_CONTEXT_DECLARATION_END);
	return ret;
}


/* declaration-specifiers */
static int _declaration_specifiers(C99 * c99)
	/* storage-class-specifier [ declaration-specifiers ]
	 * type-specifier [ declaration-specifiers ]
	 * type-qualifier [ declaration-specifiers ]
	 * function-specifier [ declaration-specifiers ] */
{
	int ret = 0;
	int looped;

	DEBUG_GRAMMAR();
	for(looped = 0;; looped = 1)
		if(_parse_in_set(c99, c99set_storage_class_specifier))
			ret |= _storage_class_specifier(c99);
		else if(_parse_in_set(c99, c99set_type_specifier))
			ret |= _type_specifier(c99);
		else if(_parse_in_set(c99, c99set_type_qualifier))
			ret |= _type_qualifier(c99);
		else if(_parse_in_set(c99, c99set_function_specifier))
			ret |= _function_specifier(c99);
		else
		{
			if(looped == 0)
				ret |= _parse_error(c99, "Expected declaration"
					" specifier");
			break;
		}
	return ret;
}


/* storage-class-specifier
 * PRE	the first token starts a storage-class-specifier */
static int _storage_class_specifier(C99 * c99)
	/* "typedef" | "extern" | "static" | "auto" | "register" */
{
	int ret;
	CodeStorage storage = CODE_STORAGE_NULL;

	DEBUG_GRAMMAR();
	switch(_parse_get_code(c99))
	{
		case C99_CODE_KEYWORD_TYPEDEF:
			storage = CODE_STORAGE_TYPEDEF;
			break;
		case C99_CODE_KEYWORD_EXTERN:
			storage = CODE_STORAGE_EXTERN;
			break;
		case C99_CODE_KEYWORD_STATIC:
			storage = CODE_STORAGE_STATIC;
			break;
		case C99_CODE_KEYWORD_AUTO:
			storage = CODE_STORAGE_AUTO;
			break;
		case C99_CODE_KEYWORD_REGISTER:
			storage = CODE_STORAGE_REGISTER;
			break;
		default:
			break;
	}
	if((ret = code_context_set_storage(c99->code, storage)) != 0)
		_parse_error(c99, error_get());
	ret |= scan(c99);
	return ret;
}


/* type-specifier
 * PRE	the first token starts a storage-class-specifier */
static int _type_specifier(C99 * c99)
	/* void | char | short | int | long | float | double | signed | unsigned
	 * | _Bool | _Complex | _Imaginary | struct-or-union-specifier
	 * | enum-specifier | typedef-name */
{
	int ret = 0;
	CodeClass cclass = CODE_CLASS_NULL;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_struct_or_union_specifier))
		return _struct_or_union_specifier(c99);
	else if(_parse_in_set(c99, c99set_enum_specifier))
		return _enum_specifier(c99);
	else if(_parse_in_set(c99, c99set_typedef_name))
		return _typedef_name(c99);
	switch(_parse_get_code(c99))
	{
		case C99_CODE_KEYWORD_VOID:
			cclass = CODE_CLASS_VOID;
			break;
		case C99_CODE_KEYWORD_CHAR:
			cclass = CODE_CLASS_CHAR;
			break;
		case C99_CODE_KEYWORD_SHORT:
			cclass = CODE_CLASS_SHORT;
			break;
		case C99_CODE_KEYWORD_INT:
			cclass = CODE_CLASS_INT;
			break;
		case C99_CODE_KEYWORD_LONG:
			cclass = CODE_CLASS_LONG;
			break;
		case C99_CODE_KEYWORD_FLOAT:
			cclass = CODE_CLASS_FLOAT;
			break;
		case C99_CODE_KEYWORD_DOUBLE:
			cclass = CODE_CLASS_DOUBLE;
			break;
		case C99_CODE_KEYWORD_SIGNED:
			cclass = CODE_CLASS_SIGNED;
			break;
		case C99_CODE_KEYWORD_UNSIGNED:
			cclass = CODE_CLASS_UNSIGNED;
			break;
		case C99_CODE_KEYWORD__BOOL:
			cclass = CODE_CLASS__BOOL;
			break;
		case C99_CODE_KEYWORD__COMPLEX:
			cclass = CODE_CLASS__COMPLEX;
			break;
		case C99_CODE_KEYWORD__IMAGINARY:
			cclass = CODE_CLASS__IMAGINARY;
			break;
		default:
			assert(0);
			break;
	}
	if((ret = code_context_set_class(c99->code, cclass)) != 0)
		_parse_error(c99, error_get());
	ret |= scan(c99);
	return ret;
}


/* struct-or-union-specifier
 * PRE	the first token starts a struct-or-union-specifier */
static int _struct_or_union_specifier(C99 * c99)
	/* struct-or-union [ identifier ] "{" struct-declaration-list "}"
	 * | struct-or-union identifier */
{
	int ret;
	CodeContext context;
	C99Code code;

	DEBUG_GRAMMAR();
	context = code_context_get(c99->code);
	ret = _struct_or_union(c99);
	if((code = _parse_get_code(c99)) == C99_CODE_IDENTIFIER)
	{
		ret |= _identifier(c99);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_LBRACE))
		{
			ret |= code_context_set(c99->code, context);
			return ret;
		}
	}
	else if(code != C99_CODE_OPERATOR_LBRACE)
	{
		ret |= code_context_set(c99->code, context);
		ret |= _parse_error(c99, "%s", "Expected identifier or {");
		return ret;
	}
	ret |= scan(c99);
	/* XXX the grammar says there's got to be one */
	if(_parse_in_set(c99, c99set_struct_declaration_list))
	{
		ret |= _struct_declaration_list(c99);
		/* XXX the grammar doesn't mention this? */
		if(_parse_is_code(c99, C99_CODE_COMMA))
			ret |= scan(c99);
	}
	ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
	ret |= code_context_set(c99->code, context);
	return ret;
}


/* struct-or-union
 * PRE	the first token starts a struct-or-union */
static int _struct_or_union(C99 * c99)
	/* "struct" | "union" */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = code_context_set(c99->code, _parse_get_code(c99)
			== C99_CODE_KEYWORD_STRUCT ? CODE_CONTEXT_STRUCT
			: CODE_CONTEXT_UNION);
	ret |= scan(c99);
	return ret;
}


/* struct-declaration-list
 * PRE	the first token starts a struct-declaration-list */
static int _struct_declaration_list(C99 * c99)
	/* struct-declaration { struct-declaration } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _struct_declaration(c99);
	while(_parse_in_set(c99, c99set_struct_declaration))
		ret |= _struct_declaration(c99);
	return ret;
}


/* struct-declaration
 * PRE	the first token starts a struct-declaration */
static int _struct_declaration(C99 * c99)
	/* specifier-qualifier-list struct-declarator-list ";" */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _specifier_qualifier_list(c99);
	ret |= _struct_declarator_list(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	return ret;
}


/* struct-declarator-list */
static int _struct_declarator_list(C99 * c99)
	/* struct-declarator { "," struct-declarator } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _struct_declarator(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		ret |= _struct_declarator(c99);
	}
	return ret;
}


/* struct-declarator */
static int _struct_declarator(C99 * c99)
	/* declarator
	 * [ declarator ] ":" constant-expr */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(!_parse_is_code(c99, C99_CODE_OPERATOR_COLON))
		ret |= _declarator(c99);
	if(_parse_is_code(c99, C99_CODE_OPERATOR_COLON))
	{
		ret |= scan(c99);
		ret |= _constant_expr(c99);
	}
	return ret;
}


/* enum-specifier
 * PRE	the first token starts an enum-specifier */
static int _enum_specifier(C99 * c99)
	/* "enum" [ identifier ] "{" enumerator { "," enumerator } [ ","] "}"
	 * "enum" identifier */
{
	int ret;
	CodeContext context;

	DEBUG_GRAMMAR();
	context = code_context_get(c99->code);
	ret = scan(c99);
	if(_parse_is_code(c99, C99_CODE_IDENTIFIER))
	{
		code_context_set(c99->code, CODE_CONTEXT_ENUMERATION_NAME);
		ret |= _identifier(c99);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_LBRACE))
		{
			code_context_set(c99->code, context);
			return ret;
		}
	}
	ret |= _parse_check(c99, C99_CODE_OPERATOR_LBRACE);
	ret |= _parse_check_set(c99, c99set_enumerator, "enumerator",
			_enumerator);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_RBRACE))
			ret |= _parse_check_set(c99, c99set_enumerator,
					"enumerator", _enumerator);
		else
			break;
	}
	ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
	code_context_set(c99->code, context);
	return ret;
}


/* enumerator
 * PRE	the first token starts an enumerator */
static int _enumerator(C99 * c99)
	/* enumeration-constant [ "=" constant-expression ] */
{
	int ret;

	DEBUG_GRAMMAR();
	code_context_set(c99->code, CODE_CONTEXT_ENUMERATION_CONSTANT);
	ret = _enumeration_constant(c99);
	if(_parse_is_code(c99, C99_CODE_OPERATOR_EQUALS))
	{
		ret |= scan(c99);
		code_context_set(c99->code, CODE_CONTEXT_ENUMERATION_VALUE);
		ret |= _constant_expr(c99);
	}
	return ret;
}


/* enumeration-constant
 * PRE	the first token starts an enumeration-constant */
static int _enumeration_constant(C99 * c99)
	/* identifier */
{
	DEBUG_GRAMMAR();
	return _identifier(c99);
}


/* typedef-name
 * PRE	the first token starts a typedef-name */
static int _typedef_name(C99 * c99)
	/* identifier */
{
	int ret;
	CodeContext context;

	DEBUG_GRAMMAR();
	context = code_context_get(c99->code);
	ret = code_context_set(c99->code, CODE_CONTEXT_TYPEDEF_NAME);
	ret |= _identifier(c99);
	ret |= code_context_set(c99->code, context);
	return ret;
}


/* type-qualifier
 * PRE	the first token starts a type-qualifier */
static int _type_qualifier(C99 * c99)
	/* "const" | "restrict" | "volatile" */
{
	DEBUG_GRAMMAR();
	return scan(c99);
}


/* function-specifier
 * PRE	the first token starts a function-specifier */
static int _function_specifier(C99 * c99)
	/* "inline" */
{
	DEBUG_GRAMMAR();
	return scan(c99);
}


/* declarator */
static int _declarator(C99 * c99)
	/* [ pointer ] direct-declarator */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_pointer))
		ret |= _pointer(c99);
	ret |= _direct_declarator(c99);
	return ret;
}


/* pointer
 * PRE	the first token starts a pointer */
static int _pointer(C99 * c99)
	 /* "*" [ type-qualifier-list ] { "*" [ type-qualifier-list ] } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = scan(c99);
	if(_parse_in_set(c99, c99set_type_qualifier_list))
		ret |= _type_qualifier_list(c99);
	while(_parse_in_set(c99, c99set_pointer))
	{
		ret |= scan(c99);
		if(_parse_in_set(c99, c99set_type_qualifier_list))
			ret |= _type_qualifier_list(c99);
	}
	return ret;
}


/* type-qualifier-list
 * PRE	the first token starts a type-qualifier-list */
static int _type_qualifier_list(C99 * c99)
	/* type-qualifier { type-qualifier } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _type_qualifier(c99);
	while(_parse_in_set(c99, c99set_type_qualifier))
		ret |= _type_qualifier(c99);
	return ret;
}


/* direct-declarator */
static int _direct_declarator(C99 * c99)
	/* identifier
	 * "(" declarator ")"
	 * direct-declarator "[" [ assignment-expr | "*" ] "]"
	 * direct-declarator "(" parameter-type-list ")"
	 * direct-declarator "(" [ identifier-list ] ")" */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	if(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
	{
		ret = scan(c99);
		ret |= _declarator(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else
		ret = _identifier(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN)
			|| _parse_is_code(c99, C99_CODE_OPERATOR_LBRACKET))
	{
		code = token_get_code(c99->token);
		ret |= scan(c99);
		if(code == C99_CODE_OPERATOR_LBRACKET)
		{
			if(_parse_is_code(c99, C99_CODE_OPERATOR_TIMES))
				ret |= scan(c99);
			else if(_parse_in_set(c99, c99set_assignment_expr))
				ret |= _assignment_expr(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
		}
		else /* C99_CODE_OPERATOR_LPAREN */
		{
			if(_parse_in_set(c99, c99set_parameter_type_list))
			{
				ret |= code_context_set(c99->code,
						CODE_CONTEXT_PARAMETERS_TYPE);
				ret |= _parameter_type_list(c99);
			}
			else if(_parse_in_set(c99, c99set_identifier_list))
			{
				ret |= code_context_set(c99->code,
						CODE_CONTEXT_PARAMETERS);
				ret |= _identifier_list(c99);
			}
			else
				ret |= code_context_set(c99->code,
						CODE_CONTEXT_PARAMETERS);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
	}
	return ret;
}


/* identifier */
static int _identifier(C99 * c99)
	/* identifier-nondigit { (identifier-nondigit | identifier-digit) } */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(code_context_set_identifier(c99->code, _parse_get_string(c99)) != 0)
		ret = _parse_error(c99, "%s", error_get());
	ret |= scan(c99);
	return ret;
}


/* identifier-list
 * PRE	the first token starts an identifier-list */
static int _identifier_list(C99 * c99)
	/* identifier { "," identifier } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _identifier(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		ret |= _parse_check_set(c99, c99set_identifier, "identifier",
				_identifier);
	}
	return ret;
}


/* parameter-type-list
 * PRE	the first token starts a parameter-type-list */
static int _parameter_type_list(C99 * c99)
	/* parameter-declaration { "," parameter-declaration } [ "," "..." ] */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _parameter_declaration(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		if(_parse_is_code(c99, C99_CODE_OPERATOR_DOTDOTDOT))
		{
			ret |= scan(c99);
			break;
		}
		ret |= _parameter_declaration(c99);
	}
	return ret;
}


/* parameter-declaration */
static int _parameter_declaration(C99 * c99)
	/* declaration-specifiers [ (declarator | abstract-declarator) ] */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _declaration_specifiers(c99);
	if(_parse_in_set(c99, c99set_abstract_or_declarator))
		ret |= _abstract_or_declarator(c99);
	return ret;
}


/* abstract-or-declarator */
static int _abstract_or_declarator(C99 * c99)
	/* pointer
	 * [ pointer ] (direct-declarator | direct-abstract-declarator) */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_pointer))
		ret |= _pointer(c99);
	if(_parse_is_code(c99, C99_CODE_IDENTIFIER))
		ret |= _direct_declarator(c99);
	else if(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
		/* after "(" it can be:
		 * - declarator (pointer, identifier, "(")
		 * - abstract-declarator (pointer, parameter-type-list, "(",
		 *                        "[", nothing) */
		while(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
		{
			ret |= scan(c99);
			if(_parse_in_set(c99, c99set_pointer))
				ret |= _pointer(c99);
			if(_parse_is_code(c99, C99_CODE_IDENTIFIER))
				ret |= _direct_declarator(c99);
			else if(_parse_is_code(c99, C99_CODE_OPERATOR_LBRACKET))
				ret |= _direct_abstract_declarator(c99);
			else if(_parse_in_set(c99, c99set_parameter_type_list))
				ret |= _parameter_type_list(c99);
			else
				/* FIXME get rid of recursivity */
				ret |= _abstract_or_declarator(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
	else
		ret |= _direct_abstract_declarator(c99);
	return ret;
}


/* abstract-declarator */
static int _abstract_declarator(C99 * c99)
	/* pointer
	 * [ pointer ] direct-abstract-declarator */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_pointer))
		ret = _pointer(c99);
	if(_parse_in_set(c99, c99set_direct_abstract_declarator))
		ret |= _direct_abstract_declarator(c99);
	return ret;
}


/* direct-abstract-declarator */
static int _direct_abstract_declarator(C99 * c99)
	/* "(" abstract-declarator ")"
	 * [ direct-abstract-declarator ] "[" [ assignment-expr ] "]"
	 * direct-abstract-declarator "[" "*" "]"
	 * [ direct-abstract-declarator ] "(" [ parameter-type-list ] ")" */

{
	int ret = 0;
	C99Code code;

	/* FIXME verify if correct */
	DEBUG_GRAMMAR();
	code = _parse_get_code(c99);
	if(code == C99_CODE_OPERATOR_LPAREN)
	{
		ret = scan(c99);
		if(_parse_in_set(c99, c99set_parameter_type_list))
			ret |= _parameter_type_list(c99);
		else if(_parse_get_code(c99) != C99_CODE_OPERATOR_RPAREN)
			_parse_check_set(c99, c99set_abstract_declarator,
					"parameter type list"
					"or abstract declarator",
					_abstract_declarator);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else if(code == C99_CODE_OPERATOR_LBRACKET)
	{
		ret = scan(c99);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_RBRACKET))
			ret |= _assignment_expr(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
	}
	while((code = _parse_get_code(c99)) != TC_NULL)
		if(code == C99_CODE_OPERATOR_LPAREN)
		{
			ret |= scan(c99);
			if(_parse_in_set(c99, c99set_parameter_type_list))
				ret |= _parameter_type_list(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else if(code == C99_CODE_OPERATOR_LBRACKET)
		{
			if(_parse_is_code(c99, C99_CODE_OPERATOR_TIMES))
				ret |= scan(c99);
			else if(_parse_in_set(c99, c99set_assignment_expr))
				ret |= _assignment_expr(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
		}
		else
			break;
	return ret;
}


/* assignment-expr */
static int _assignment_expr(C99 * c99)
	/* { unary-expr assignment-operator } conditional-expr */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	/* FIXME hack around the conflict between unary and conditional */
	for(;;)
	{
		ret |= _conditional_expr(c99);
		if(!_parse_in_set(c99, c99set_assignment_operator))
			return ret;
		ret |= code_context_set(c99->code, CODE_CONTEXT_ASSIGNMENT);
		ret |= _assignment_operator(c99);
	}
	return ret;
}


/* unary-expr */
static int _unary_expr(C99 * c99)
	/* FIXME still recursive
	 * postfix-expr
	 * "++" unary-expr
	 * "--" unary-expr
	 * unary-operator cast-expr
	 * "sizeof" unary-expr
	 * "sizeof" "(" type-name ")" */
{
	int ret = 0;
	C99Code code;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_postfix_expr))
		return _postfix_expr(c99);
	/* FIXME use _parse_get_code() */
	else if(c99->token == NULL)
		return _parse_error(c99, "Unexpected end of file");
	else if((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_DPLUS
			|| code == C99_CODE_OPERATOR_DMINUS)
	{
		ret = scan(c99);
		ret |= _unary_expr(c99);
	}
	else if(_parse_in_set(c99, c99set_unary_operator))
	{
		ret = _unary_operator(c99);
		ret |= _cast_expr(c99);
	}
	else if(code == C99_CODE_KEYWORD_SIZEOF)
	{
		ret = scan(c99);
		if(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
		{
			ret |= scan(c99);
			/* FIXME it may still be an unary-expr ("{") */
			if(_parse_in_set(c99, c99set_type_name))
				ret |= _type_name(c99);
			else
				ret |= _parse_check_set(c99, c99set_expression,
						"type cast or expression",
						_expression);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else
			ret |= _parse_check_set(c99, c99set_unary_expr,
					"type cast or expression",
					_unary_expr);
	}
	return ret;
}


/* postfix-expr
 * PRE	the first token starts a postfix-expr */
static int _postfix_expr(C99 * c99)
	/* primary-expr
	 * postfix-expr "[" expression "]"
	 * postfix-expr "(" [ argument-expr-list ] ")"
	 * postfix-expr "." identifier
	 * postfix-expr "->" identifier
	 * postfix-expr "++"
	 * postfix-expr "--"
	 * "(" type-name ")" "{" initializer-list [ "," ] "}"
	 * "(" expression ")" */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_primary_expr))
		ret |= _primary_expr(c99);
	else if(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
	{
		ret |= scan(c99);
		if(_parse_in_set(c99, c99set_type_name))
		{
			ret |= _type_name(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_LBRACE);
			ret |= _initializer_list(c99);
			if(_parse_is_code(c99, C99_CODE_COMMA))
				ret |= scan(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
		}
		else if(_parse_in_set(c99, c99set_expression))
		{
			ret |= _expression(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else
			ret |= _parse_error(c99,
					"Expected type name or expression");
	}
	ret |= _postfix_expr_do(c99);
	return ret;
}


/* postfix-expr-do */
static int _postfix_expr_do(C99 * c99)
{
	int ret = 0;
	C99Code code;

	while((code = _parse_get_code(c99)) != TC_NULL)
		if(code == C99_CODE_OPERATOR_LBRACKET)
		{
			ret |= scan(c99);
			ret |= _parse_check_set(c99, c99set_expression,
					"expression", _expression);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
		}
		else if(code == C99_CODE_OPERATOR_LPAREN)
		{
			if(code_context_set(c99->code,
					CODE_CONTEXT_FUNCTION_PARAMETERS) != 0)
				ret |= _parse_error(c99, error_get());
			ret |= scan(c99);
			if(!_parse_is_code(c99, C99_CODE_OPERATOR_RPAREN))
				ret |= _parse_check_set(c99,
						c99set_argument_expr_list,
						"argument list",
						_argument_expr_list);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else if(code == C99_CODE_OPERATOR_DOT
				|| code == C99_CODE_OPERATOR_MGREATER)
		{
			ret |= scan(c99);
			ret |= _parse_check_set(c99, c99set_identifier,
					"identifier", _identifier);
		}
		else if(code == C99_CODE_OPERATOR_DPLUS
				|| code == C99_CODE_OPERATOR_DMINUS)
			ret |= scan(c99);
		else
			break;
	return ret;
}


/* argument-expr-list */
static int _argument_expr_list(C99 * c99)
	/* assignment-expr { "," assignment-expr } */
{
	int ret;
#ifdef DEBUG
	unsigned int cnt = 1;

	fprintf(stderr, "DEBUG: %s() arg 1 \"%s\"\n", __func__,
			_parse_get_string(c99));
#endif
	ret = _assignment_expr(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() arg %u \"%s\"\n", __func__, ++cnt,
				_parse_get_string(c99));
#endif
		ret |= _assignment_expr(c99);
	}
	return ret;
}


/* primary-expr
 * PRE	the first token starts a primary-expr */
static int _primary_expr(C99 * c99)
	/* identifier
	 * constant
	 * string-literal */
{
	int ret = 0;
	C99Code code;

	DEBUG_GRAMMAR();
	if(code_context_set(c99->code, CODE_CONTEXT_PRIMARY_EXPR) != 0)
		ret |= _parse_error(c99, error_get());
	if((code = token_get_code(c99->token)) == C99_CODE_IDENTIFIER)
	{
		ret |= _identifier(c99);
		if(c99->can_label && _parse_is_code(c99,
					C99_CODE_OPERATOR_COLON))
			/* handle this as a labeled_statement */
			c99->is_label = 1;
	}
	else if(code == C99_CODE_DQUOTE) /* string-litteral */
	{
		ret |= scan(c99);
		/* FIXME ugly work-around string-literal concatenation */
		while(token_get_code(c99->token) == C99_CODE_DQUOTE)
			ret |= scan(c99);
	}
	else /* constant */
		ret |= scan(c99);
	c99->can_label = 0;
	return ret;
}


/* type-name
 * PRE	the first token starts a type-name */
static int _type_name(C99 * c99)
	/* specifier-qualifier-list [ abstract-declarator ] */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _specifier_qualifier_list(c99);
	if(_parse_in_set(c99, c99set_abstract_declarator))
		ret |= _abstract_declarator(c99);
	return ret;
}


/* specifier-qualifier-list
 * PRE	the first token starts a specifier-qualifier-list */
static int _specifier_qualifier_list(C99 * c99)
	/* (type-specifier | type-qualifier) { (type-specifier | type-qualifier) } */
{
	int ret = 0;
	int looped = 0;

	DEBUG_GRAMMAR();
	for(;; looped = 1)
		if(_parse_in_set(c99, c99set_type_specifier))
			ret |= _type_specifier(c99);
		else if(_parse_in_set(c99, c99set_type_qualifier))
			ret |= _type_qualifier(c99);
		else if(looped == 0)
			ret |= _parse_error(c99, "Expected type specifier"
					" or type qualifier");
		else
			break;
	return ret;
}


/* unary-operator */
static int _unary_operator(C99 * c99)
	/* "&" | "*" | "+" | "-" | "~" | "!" */
{
	DEBUG_GRAMMAR();
	return scan(c99);
}


/* assignment-operator */
static int _assignment_operator(C99 * c99)
	/* "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^="
	 * | "|=" */
{
	DEBUG_GRAMMAR();
	return scan(c99);
}


/* conditional-expr */
static int _conditional_expr(C99 * c99)
	/* logical-OR-expr { "?" expression ":" logical-OR-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _logical_or_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_QUESTION))
	{
		ret |= scan(c99);
		ret |= _parse_check_set(c99, c99set_expression, "expression",
				_expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_COLON);
		ret |= _logical_or_expr(c99);
	}
	return ret;
}


/* logical-OR-expr */
static int _logical_or_expr(C99 * c99)
	/* logical-AND-expr { "||" logical-AND-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _logical_and_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_DBAR))
	{
		ret |= scan(c99);
		ret |= _logical_and_expr(c99);
	}
	return ret;
}


/* logical-AND-expr */
static int _logical_and_expr(C99 * c99)
	/* inclusive-OR-expr { "&&" inclusive-OR-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _inclusive_or_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_DAMPERSAND))
	{
		ret |= scan(c99);
		ret |= _inclusive_or_expr(c99);
	}
	return ret;
}


/* inclusive-OR-expr */
static int _inclusive_or_expr(C99 * c99)
	/* exclusive-OR-expr { "|" exclusive-OR-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _exclusive_or_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_BAR))
	{
		ret |= scan(c99);
		ret |= _exclusive_or_expr(c99);
	}
	return ret;
}


/* exclusive-OR-expr */
static int _exclusive_or_expr(C99 * c99)
	/* AND-expr { "^" AND-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _and_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_XOR))
	{
		ret |= scan(c99);
		ret |= _and_expr(c99);
	}
	return ret;
}


/* AND-expr */
static int _and_expr(C99 * c99)
	/* equality-expr { "&" equality-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _equality_expr(c99);
	while(_parse_is_code(c99, C99_CODE_OPERATOR_AMPERSAND))
	{
		ret |= scan(c99);
		ret |= _equality_expr(c99);
	}
	return ret;
}


/* equality-expr */
static int _equality_expr(C99 * c99)
	/* relational-expr { ("==" | "!=") relational-expr } */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	ret = _relational_expr(c99);
	while((code = _parse_get_code(c99)) != TC_NULL
			&& (code == C99_CODE_OPERATOR_DEQUALS
				|| code == C99_CODE_OPERATOR_NEQUALS))
	{
		ret |= scan(c99);
		ret |= _relational_expr(c99);
	}
	return ret;
}


/* relational-expr */
static int _relational_expr(C99 * c99)
	/* shift-expr { ("<" | ">" | "<=" | ">=") shift-expr } */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	ret = _shift_expr(c99);
	while((code = _parse_get_code(c99)) != TC_NULL
			&& (code == C99_CODE_OPERATOR_LESS
				|| code == C99_CODE_OPERATOR_GREATER
				|| code == C99_CODE_OPERATOR_LEQUALS
				|| code == C99_CODE_OPERATOR_GEQUALS))
	{
		ret |= scan(c99);
		ret |= _shift_expr(c99);
	}
	return ret;
}


/* shift-expr */
static int _shift_expr(C99 * c99)
	/* additive-expr { ("<<" | ">>") additive-expr } */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	ret = _additive_expr(c99);
	while((code = _parse_get_code(c99)) != TC_NULL
			&& (code == C99_CODE_OPERATOR_DLESS
				|| code == C99_CODE_OPERATOR_DGREATER))
	{
		ret |= scan(c99);
		ret |= _additive_expr(c99);
	}
	return ret;
}


/* additive-expr */
static int _additive_expr(C99 * c99)
	/* multiplicative-expr { ("+" | "-") multiplicative-expr } */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	ret = _multiplicative_expr(c99);
	while((code = _parse_get_code(c99)) != TC_NULL
			&& (code == C99_CODE_OPERATOR_PLUS
				|| code == C99_CODE_OPERATOR_MINUS))
	{
		ret |= scan(c99);
		ret |= _multiplicative_expr(c99);
	}
	return ret;
}


/* multiplicative-expr */
static int _multiplicative_expr(C99 * c99)
	/* cast-expr { ("*" | "/" | "%") cast-expr } */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	ret = _cast_expr(c99);
	while((code = _parse_get_code(c99)) != TC_NULL
			&& (code == C99_CODE_OPERATOR_TIMES
				|| code == C99_CODE_OPERATOR_DIVIDE
				|| code == C99_CODE_OPERATOR_MODULO))
	{
		ret |= scan(c99);
		ret |= _cast_expr(c99);
	}
	return ret;
}


/* cast-expr */
static int _cast_expr(C99 * c99)
	/* { "(" type-name ")" } unary-expr */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	while(_parse_is_code(c99, C99_CODE_OPERATOR_LPAREN))
	{
		ret |= scan(c99);
		/* in both cases it may already be an unary-expr */
		if(_parse_in_set(c99, c99set_type_name))
		{
			ret |= _type_name(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
			/* FIXME "{" => postfix-expr with initializer-list */
			continue;
		}
		/* primary-expr */
		ret |= _parse_check_set(c99, c99set_expression,
				"type cast or expression", _expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _postfix_expr_do(c99);
		return ret;
	}
	ret |= _parse_check_set(c99, c99set_unary_expr, "unary expression",
			_unary_expr);
	return ret;
}


/* compound-statement
 * PRE	the first token starts a compound-statement */
static int _compound_statement(C99 * c99)
	/* "{" [ block-item-list ] "}" */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = scan(c99);
	if(code_scope_push(c99->code) < 0)
		ret |= _parse_error(c99, error_get());
	if(_parse_in_set(c99, c99set_block_item_list))
		ret |= _block_item_list(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
	if(code_scope_pop(c99->code) < 0)
		ret |= _parse_error(c99, error_get());
	return ret;
}


/* block-item-list
 * PRE	the first token starts a block-item-list */
static int _block_item_list(C99 * c99)
	/* block-item { block-item } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _block_item(c99);
	while(_parse_in_set(c99, c99set_block_item))
		ret |= _block_item(c99);
	return ret;
}


/* block-item
 * PRE	the first token starts a block-item */
static int _block_item(C99 * c99)
	/* declaration | statement */
{
	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_declaration))
		return _declaration(c99);
	else if(_parse_in_set(c99, c99set_statement))
		return _statement(c99);
	/* XXX should be bloat now */
	return _parse_error(c99, "Expected declaration or statement");
}


/* statement
 * PRE	the first token starts a statement */
static int _statement(C99 * c99)
	/* labeled-statement
	 * compound-statement
	 * expression-statement
	 * selection-statement
	 * iteration-statement
	 * jump-statement */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = code_context_set(c99->code, CODE_CONTEXT_STATEMENT);
	if(_parse_in_set(c99, c99set_labeled_statement))
		ret |= _labeled_statement(c99);
	else if(_parse_in_set(c99, c99set_compound_statement))
		ret |= _compound_statement(c99);
	else if(_parse_in_set(c99, c99set_expression_statement))
		ret |= _expression_statement(c99);
	else if(_parse_in_set(c99, c99set_selection_statement))
		ret |= _selection_statement(c99);
	else if(_parse_in_set(c99, c99set_iteration_statement))
		ret |= _iteration_statement(c99);
	else if(_parse_in_set(c99, c99set_jump_statement))
		ret |= _jump_statement(c99);
	return ret;
}


/* labeled-statement
 * PRE	the first token starts a labeled-statement */
static int _labeled_statement(C99 * c99)
	/* identifier ":" statement
	 * "case" constant-expr ":" statement
	 * "default" ":" statement */
{
	int ret = 0;
	C99Code code;

	DEBUG_GRAMMAR();
	if((code = _parse_get_code(c99)) == C99_CODE_IDENTIFIER)
		ret |= _identifier(c99);
	else
	{
		if(!c99->in_switch)
			ret |= _parse_error(c99, "Not in a switch");
		if(code == C99_CODE_KEYWORD_CASE)
		{
			ret |= scan(c99);
			ret |= _constant_expr(c99);
		}
		else /* default */
			ret |= scan(c99);
	}
	ret |= _parse_check(c99, C99_CODE_OPERATOR_COLON);
	ret |= _parse_check_set(c99, c99set_statement, "statement", _statement);
	return ret;
}


/* constant_expr */
static int _constant_expr(C99 * c99)
	/* conditional-expr */
{
	DEBUG_GRAMMAR();
	return _conditional_expr(c99);
}


/* expression-statement
 * PRE	the first token starts an expression-statement or a label */
static int _expression_statement(C99 * c99)
	/* [ expression ] ";"
	 * identifier ":" */
{
	int ret;

	DEBUG_GRAMMAR();
	if(_parse_is_code(c99, C99_CODE_OPERATOR_SEMICOLON))
		return scan(c99);
	c99->can_label = 1;
	c99->is_label = 0;
	ret = _parse_check_set(c99, c99set_expression, "expression",
			_expression);
	if(c99->is_label)
	{
		ret |= code_context_set(c99->code, CODE_CONTEXT_LABEL);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_COLON);
		c99->is_label = 0;
	}
	else
		ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	return ret;
}


/* expression
 * PRE	the first token starts an expression */
static int _expression(C99 * c99)
	/* assignment-expr { "," assignment-expr } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _assignment_expr(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		scan(c99);
		ret |= _parse_check_set(c99, c99set_assignment_expr,
				"assignment expression", _assignment_expr);
	}
	return ret;
}


/* selection-statement
 * PRE	the first token starts a selection-statement */
static int _selection_statement(C99 * c99)
	/* "if" "(" expression ")" statement [ "else" statement ]
	 * "switch" "(" expression ")" statement */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	if((code = _parse_get_code(c99)) == C99_CODE_KEYWORD_SWITCH)
		c99->in_switch++;
	ret = scan(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
	ret |= _parse_check_set(c99, c99set_expression, "expression",
			_expression);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	ret |= _parse_check_set(c99, c99set_statement, "statement", _statement);
	if(code == C99_CODE_KEYWORD_SWITCH)
		c99->in_switch--;
	else if(code == C99_CODE_KEYWORD_IF
			&& _parse_is_code(c99, C99_CODE_KEYWORD_ELSE))
	{
		ret |= scan(c99);
		ret |= _parse_check_set(c99, c99set_statement, "statement",
				_statement);
	}
	return ret;
}


/* iteration-statement
 * PRE	the first token starts an iteration-statement */
static int _iteration_statement(C99 * c99)
	/* while "(" expression ")" statement
	 * do statement while "(" expression ")" ;
	 * for "(" [ (expr | declaration) ] ; [ expr ] ; [ expr ] ")" statement */
{
	int ret = 0;
	C99Code code;

	DEBUG_GRAMMAR();
	if((code = _parse_get_code(c99)) == C99_CODE_KEYWORD_WHILE)
	{
		ret = scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		ret |= _parse_check_set(c99, c99set_expression, "expression",
				_expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _parse_check_set(c99, c99set_statement, "statement",
				_statement);
	}
	else if(code == C99_CODE_KEYWORD_DO)
	{
		ret = scan(c99);
		ret |= _parse_check_set(c99, c99set_statement, "statement",
				_statement);
		ret |= _parse_check(c99, C99_CODE_KEYWORD_WHILE);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		ret |= _parse_check_set(c99, c99set_expression, "expression",
				_expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else if(code == C99_CODE_KEYWORD_FOR)
	{
		ret = scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		/* FIXME or declaration */
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_SEMICOLON))
			ret |= _parse_check_set(c99, c99set_expression,
					"expression", _expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_SEMICOLON))
			ret |= _parse_check_set(c99, c99set_expression,
					"expression", _expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
		if(!_parse_is_code(c99, C99_CODE_OPERATOR_RPAREN))
			ret |= _parse_check_set(c99, c99set_expression,
					"expression", _expression);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _parse_check_set(c99, c99set_statement, "statement",
				_statement);
	}
	return ret;
}


/* jump-statement
 * PRE	the first token starts a jump-statement */
static int _jump_statement(C99 * c99)
	/* "goto" identifier ";"
	 * "continue" ";"
	 * "break" ";"
	 * "return" [ expression ] ";" */
{
	int ret;
	C99Code code;

	DEBUG_GRAMMAR();
	if((code = token_get_code(c99->token)) == C99_CODE_KEYWORD_GOTO)
	{
		ret = scan(c99);
		ret |= _parse_check_set(c99, c99set_identifier, "identifier",
				_identifier);
	}
	else if(code == C99_CODE_KEYWORD_RETURN)
	{
		ret = scan(c99);
		if(_parse_in_set(c99, c99set_expression))
			ret = _expression(c99);
	}
	else /* continue or break */
		ret = scan(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	return ret;
}


/* init-declarator-list */
static int _init_declarator_list(C99 * c99)
	/* init-declarator { "," init-declarator } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _init_declarator(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		ret |= _init_declarator(c99);
	}
	return ret;
}


/* init-declarator */
static int _init_declarator(C99 * c99)
	/* declarator [ "=" initializer ] */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _declarator(c99);
	if(_parse_is_code(c99, C99_CODE_OPERATOR_EQUALS))
	{
		ret |= scan(c99);
		ret |= _initializer(c99);
	}
	return ret;
}


/* initializer */
static int _initializer(C99 * c99)
	/* assignment-expr
	 * "{" initializer-list [ "," ] "}" */
{
	int ret;

	DEBUG_GRAMMAR();
	if(_parse_is_code(c99, C99_CODE_OPERATOR_LBRACE))
	{
		ret = scan(c99);
		ret |= _initializer_list(c99);
		if(_parse_is_code(c99, C99_CODE_COMMA))
			ret |= scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
	}
	else
		ret = _assignment_expr(c99);
	return ret;
}


/* initializer-list */
static int _initializer_list(C99 * c99)
	/* [ designation ] initializer { "," [ designation] initializer } */
{
	int ret = 0;

	DEBUG_GRAMMAR();
	if(_parse_in_set(c99, c99set_designation))
		ret |= _designation(c99);
	ret |= _initializer(c99);
	while(_parse_is_code(c99, C99_CODE_COMMA))
	{
		ret |= scan(c99);
		if(_parse_in_set(c99, c99set_designation))
			ret |= _designation(c99);
		ret |= _initializer(c99);
	}
	return ret;
}


/* designation
 * PRE	the first token starts a designation */
static int _designation(C99 * c99)
	/* designator-list "=" */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _designator_list(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_EQUALS);
	return ret;
}


/* designator-list
 * PRE	the first token starts a designator-list */
static int _designator_list(C99 * c99)
	/* designator { designator } */
{
	int ret;

	DEBUG_GRAMMAR();
	ret = _designator(c99);
	while(_parse_in_set(c99, c99set_designator))
		ret |= _designator(c99);
	return ret;
}


/* designator
 * PRE	the first token starts a designator */
static int _designator(C99 * c99)
	/* "[" constant-expression "]"
	 * "." identifier */
{
	int ret;

	DEBUG_GRAMMAR();
	if(_parse_is_code(c99, C99_CODE_OPERATOR_LBRACKET))
	{
		ret = scan(c99);
		ret |= _constant_expr(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
	}
	else
	{
		ret = scan(c99);
		ret |= _parse_check_set(c99, c99set_identifier, "identifier",
				_identifier);
	}
	return ret;
}


/* public */
/* functions */
/* useful */
/* parse */
static int _parse_E(C99 * c99);

int parse(C99 * c99)
{
	int ret;

	if(c99->outfp != NULL) /* acting like a pre-processor */
		return _parse_E(c99);
	if((ret = _translation_unit(c99)) != 0)
	{
		unlink(c99->outfile);
		fprintf(stderr, "%s%s%s%u%s%u%s", PACKAGE ": ",
				cpp_get_filename(c99->cpp),
				": Compilation failed with ", c99->error_cnt,
				" error(s) and ", c99->warning_cnt,
				" warning(s)\n");
	}
	return ret;
}

static int _parse_E(C99 * c99)
{
	int ret;
	Token * token;
	C99Code code;

	while((ret = cpp_scan(c99->cpp, &token)) == 0 && token != NULL)
	{
		if((code = token_get_code(token)) == CPP_CODE_META_ERROR
				|| code == CPP_CODE_META_WARNING)
			fprintf(stderr, "%s%s%s%s%u%s%s\n",
					code == CPP_CODE_META_ERROR
					? "Error" : "Warning", " in ",
					token_get_filename(token), ":",
					token_get_line(token), ": ",
					token_get_string(token));
		else if(code >= CPP_CODE_META_FIRST
				&& code <= CPP_CODE_META_LAST)
			fprintf(c99->outfp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), c99->outfp);
		token_delete(token);
	}
	return ret;
}
