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



#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "tokenset.h"
#include "c99.h"
#include "../config.h"


/* private */
/* prototypes */
static int _parse_check(C99 * c99, TokenCode code);
static int _parse_error(C99 * c99, char const * format, ...);

/* grammar */
static int _translation_unit(C99 * c99);
static int _external_declaration(C99 * c99);
static int _function_definition(C99 * c99);
static int _declaration_specifiers(C99 * c99);
static int _storage_class_specifier(C99 * c99);
static int _type_specifier(C99 * c99);
static int _struct_or_union_specifier(C99 * c99);
static int _enum_specifier(C99 * c99);
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
static int _parameter_list(C99 * c99);
static int _parameter_declaration(C99 * c99);
static int _abstract_declarator(C99 * c99);
static int _assignment_expr(C99 * c99);
static int _unary_expr(C99 * c99);
static int _postfix_expr(C99 * c99);
static int _argument_expression_list(C99 * c99);
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
static int _parse_check(C99 * c99, TokenCode code)
{
	int ret;

	if((ret = (token_get_code(c99->token) != code)))
		_parse_error(c99, "Expected \"%s\"", code_get_string(code));
	c99_scan(c99);
	return ret;
}


static int _parse_error(C99 * c99, char const * format, ...)
{
	Token * token = c99->token;
	va_list ap;

	fprintf(stderr, "%s%s:%u, near \"%s\": error: ", PACKAGE ": ",
			token_get_filename(token), token_get_line(token),
			token_get_string(token));
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	return 1;
}


/* grammar */
static int _translation_unit(C99 * c99)
	/* external-declaration { external-declaration } */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while(c99_scan(c99) == 0 && c99->token != NULL)
		ret |= _external_declaration(c99);
	return ret;
}


/* external_declaration */
static int _external_declaration(C99 * c99)
	/* function-definition | declaration */
{
	/* FIXME ambiguity between function definition and declaration */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_function_definition))
		return _function_definition(c99);
	else if(token_in_set(c99->token, c99set_declaration))
		return _declaration(c99);
	return _parse_error(c99, "Expected function definition or declaration");
}


/* function_definition */
static int _function_definition(C99 * c99)
	/* declaration-specifiers declarator [ declaration-list ]
	 * compound-statement */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret |= _declaration_specifiers(c99);
	ret |= _declarator(c99);
	if(token_in_set(c99->token, c99set_declaration_list))
		ret |= _declaration_list(c99);
	ret |= _compound_statement(c99);
	return ret;
}


/* declaration-list */
static int _declaration_list(C99 * c99)
	/* declaration { declaration } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _declaration(c99);
	while(token_in_set(c99->token, c99set_declaration))
		ret |= _declaration(c99);
	return ret;
}


/* declaration */
static int _declaration(C99 * c99)
	/* declaration-specifiers [ init-declarator-list ] */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret |= _declaration_specifiers(c99);
	if(token_in_set(c99->token, c99set_init_declarator))
		ret |= _init_declarator_list(c99);
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
	int looped = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(;;)
	{
		if(token_in_set(c99->token, c99set_storage_class_specifier))
			ret |= _storage_class_specifier(c99);
		else if(token_in_set(c99->token, c99set_type_specifier))
			ret |= _type_specifier(c99);
		else if(token_in_set(c99->token, c99set_type_qualifier))
			ret |= _type_qualifier(c99);
		else if(token_in_set(c99->token, c99set_function_specifier))
			ret |= _function_specifier(c99);
		else if(looped == 0)
			return _parse_error(c99, "Expected"
					" storage class specifier"
					", type specifier, type qualifier"
					" or function specifier");
		else
			break;
		looped = 1;
	}
	return ret;
}


/* storage-class-specifier */
static int _storage_class_specifier(C99 * c99)
	/* typedef | extern | static | auto | register */
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	return c99_scan(c99);
}


/* type-specifier */
static int _type_specifier(C99 * c99)
	/* void | char | short | int | long | float | double | signed | unsigned
	 * | _Bool | _Complex | _Imaginary | struct-or-union-specifier
	 * | enum-specifier | typedef-name */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	if(token_in_set(c99->token, c99set_struct_or_union_specifier))
		ret = _struct_or_union_specifier(c99);
	else if(token_in_set(c99->token, c99set_enum_specifier))
		ret = _enum_specifier(c99);
	else if(token_in_set(c99->token, c99set_typedef_name))
		ret = _typedef_name(c99);
	else
		ret = c99_scan(c99);
	return ret;
}


/* struct-or-union-specifier */
static int _struct_or_union_specifier(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	c99_scan(c99);
	return 0;
}


/* enum-specifier */
static int _enum_specifier(C99 * c99)
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	c99_scan(c99);
	return 0;
}


/* typedef-name */
static int _typedef_name(C99 * c99)
	/* identifier */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	ret = c99_scan(c99);
	return ret;
}


/* type-qualifier */
static int _type_qualifier(C99 * c99)
	/* const | restrict | volatile */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	return c99_scan(c99);
}


/* function-specifier */
static int _function_specifier(C99 * c99)
	/* inline */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	return c99_scan(c99);
}


/* declarator */
static int _declarator(C99 * c99)
	/* [ pointer ] direct-declarator */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_pointer))
		ret |= _pointer(c99);
	ret |= _direct_declarator(c99);
	return ret;
}


/* pointer */
static int _pointer(C99 * c99)
	 /* "*" [ type-qualifier-list ] { "*" [ type-qualifier-list ] } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"*\" got \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	ret = c99_scan(c99);
	if(token_in_set(c99->token, c99set_type_qualifier_list))
		ret |= _type_qualifier_list(c99);
	while(token_in_set(c99->token, c99set_pointer))
	{
		ret |= c99_scan(c99);
		if(token_in_set(c99->token, c99set_type_qualifier_list))
			ret |= _type_qualifier_list(c99);
	}
	return ret;
}


/* type-qualifier-list */
static int _type_qualifier_list(C99 * c99)
	/* type-qualifier { type-qualifier } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _type_qualifier(c99);
	while(token_in_set(c99->token, c99set_type_qualifier))
		ret |= _type_qualifier(c99);
	return ret;
}


/* direct-declarator */
static int _direct_declarator(C99 * c99)
	/* FIXME still recursive
	 * identifier
	 * "(" declarator ")"
	 * direct-declarator [ assignment-expression ]
	 * direct-declarator [ * ]
	 * direct-declarator "(" parameter-type-list ")"
	 * direct-declarator "(" identifier-list ")" */
{
	int ret = 0;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_get_code(c99->token) == C99_CODE_OPERATOR_LPAREN)
	{
		c99_scan(c99);
		ret = _declarator(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else
		ret = _identifier(c99);
	if((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_LPAREN)
	{
		c99_scan(c99);
		if(token_in_set(c99->token, c99set_parameter_type_list))
			ret |= _parameter_type_list(c99);
		else if(token_in_set(c99->token, c99set_identifier_list))
			ret |= _identifier_list(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else if(code == C99_CODE_OPERATOR_TIMES)
		c99_scan(c99);
	else if(token_in_set(c99->token, c99set_assignment_expr))
		ret |= _assignment_expr(c99);
	return ret;
}


/* identifier */
static int _identifier(C99 * c99)
	/* identifier-nondigit { (identifier-nondigit | identifier-digit) } */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	return c99_scan(c99);
}


/* identifier-list */
static int _identifier_list(C99 * c99)
	/* identifier { "," identifier } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _identifier(c99);
	while(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		c99_scan(c99);
		ret |= _identifier(c99);
	}
	return ret;
}


/* parameter-type-list */
static int _parameter_type_list(C99 * c99)
	/* FIXME merge with parameter_list
	 * parameter-list [ "," "..." ] */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _parameter_list(c99);
	if(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		ret |= c99_scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_DOTDOTDOT);
	}
	return ret;
}


/* parameter-list */
static int _parameter_list(C99 * c99)
	/* parameter-declaration { "," parameter-declaration } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _parameter_declaration(c99);
	while(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		ret |= c99_scan(c99);
		ret |= _parameter_declaration(c99);
	}
	return ret;
}


/* parameter-declaration */
static int _parameter_declaration(C99 * c99)
	/* declaration-specifiers declarator
	 * declaration-specifiers [ abstract-declarator ] */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _declaration_specifiers(c99);
	if(token_in_set(c99->token, c99set_abstract_declarator))
		ret |= _abstract_declarator(c99);
	else if(token_in_set(c99->token, c99set_declarator))
		ret |= _declarator(c99);
	else
		ret |= _parse_error(c99, "Expected declarator"
				" or abstract declarator");
	return ret;
}


/* abstract-declarator */
static int _abstract_declarator(C99 * c99)
	/* pointer
	 * [ pointer ] direct-abstract-declarator */
{
	int ret = 0;

	/* FIXME complete */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_pointer))
		ret = _pointer(c99);
	return ret;
}


/* assignment-expr */
static int _assignment_expr(C99 * c99)
	/* { unary-expr assignment-operator } conditional-expr */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	/* FIXME heavily suspect conflict between unary and conditional */
#if 0
	while(token_in_set(c99->token, c99set_unary_expr))
	{
		ret |= _unary_expr(c99);
		ret |= _assignment_operator(c99);
	}
#endif
	ret |= _conditional_expr(c99);
	return ret;
}


/* unary-expr */
static int _unary_expr(C99 * c99)
	/* FIXME still recursive
	 * postfix-expr
	 * ++ unary-expr
	 * -- unary-expr
	 * unary-operator cast-expr
	 * sizeof unary-expr
	 * sizeof "(" type-name ")" */
{
	int ret;
	int code;

	/* FIXME complete */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	if(token_in_set(c99->token, c99set_postfix_expr))
		return _postfix_expr(c99);
	else if((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_DPLUS
			|| code == C99_CODE_OPERATOR_DMINUS)
	{
		ret = c99_scan(c99);
		ret |= _unary_expr(c99);
	}
	else if(token_in_set(c99->token, c99set_unary_operator))
	{
		ret = _unary_operator(c99);
		ret |= _cast_expr(c99);
	}
	else if(code == C99_CODE_KEYWORD_SIZEOF)
	{
		ret = c99_scan(c99);
		if(token_get_code(c99->token) == C99_CODE_OPERATOR_LPAREN)
		{
			ret |= c99_scan(c99);
			ret |= _type_name(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else
			ret |= _unary_expr(c99);
	}
	return ret;
}


/* postfix-expr */
static int _postfix_expr(C99 * c99)
	/* primary-expr
	 * postfix-expr "[" expression "]"
	 * postfix-expr "(" argument-expression-list-opt ")"
	 * postfix-expr "." identifier
	 * postfix-expr "->" identifier
	 * postfix-expr "++"
	 * postfix-expr "--"
	 * "(" type-name ")" "{" initializer-list "}"
	 * "(" type-name ")" "{" initializer-list "," "}" */
{
	int ret = 0;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_primary_expr))
		ret |= _primary_expr(c99);
	else if(token_get_code(c99->token) == C99_CODE_OPERATOR_LPAREN)
	{
		ret |= c99_scan(c99);
		ret |= _type_name(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LBRACE);
		ret |= _initializer_list(c99);
		if(token_get_code(c99->token) == C99_CODE_COMMA)
			ret |= c99_scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACE);
	}
	for(;;)
	{
		code = token_get_code(c99->token);
		if(code == C99_CODE_OPERATOR_LBRACKET)
		{
			ret |= c99_scan(c99);
			ret |= _expression(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
		}
		else if(code == C99_CODE_OPERATOR_LPAREN)
		{
			ret |= c99_scan(c99);
			if(token_get_code(c99->token)
					!= C99_CODE_OPERATOR_RPAREN)
				ret |= _argument_expression_list(c99);
			ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		}
		else if(code == C99_CODE_OPERATOR_DOT
				|| code == C99_CODE_OPERATOR_MGREATER)
		{
			ret |= c99_scan(c99);
			ret |= _identifier(c99);
		}
		else if(code == C99_CODE_OPERATOR_DPLUS
				|| code == C99_CODE_OPERATOR_DMINUS)
			ret |= c99_scan(c99);
		else
			break;
	}
	return ret;
}


/* argument-expression-list */
static int _argument_expression_list(C99 * c99)
	/* assignment-expr { "," assignment-expr } */
{
	int ret;
#ifdef DEBUG
	unsigned int cnt = 1;

	fprintf(stderr, "DEBUG: %s() arg 1 \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	ret = _assignment_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		ret |= c99_scan(c99);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() arg %u \"%s\"\n", __func__, ++cnt,
				token_get_string(c99->token));
#endif
		ret |= _assignment_expr(c99);
	}
	return ret;
}


/* primary-expr */
static int _primary_expr(C99 * c99)
	/* identifier
	 * constant
	 * string-literal
	 * "(" expression ")" */
{
	int ret;
	int code;

	/* FIXME complete */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	if((code = token_get_code(c99->token)) == C99_CODE_IDENTIFIER)
		return c99_scan(c99);
	else if(code == C99_CODE_OPERATOR_LPAREN)
	{
		ret = c99_scan(c99);
		ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		return ret;
	}
	else
		return c99_scan(c99);
}


/* type-name */
static int _type_name(C99 * c99)
	/* specifier-qualifier-list [ abstract-declarator ] */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _specifier_qualifier_list(c99);
	if(token_in_set(c99->token, c99set_abstract_declarator))
		ret |= _abstract_declarator(c99);
	return ret;
}


/* specifier-qualifier-list */
static int _specifier_qualifier_list(C99 * c99)
	/* (type-specifier | type-qualifier) { (type-specifier | type-qualifier) } */
{
	int ret = 0;
	int looped = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	for(;;)
	{
		if(token_in_set(c99->token, c99set_type_specifier))
			ret |= _type_specifier(c99);
		else if(token_in_set(c99->token, c99set_type_qualifier))
			ret |= _type_qualifier(c99);
		else if(looped == 0)
			ret |= _parse_error(c99, "Expected type specifier"
					" or type qualifier");
		else
			break;
		looped = 1;
	}
	return ret;
}


/* unary-operator */
static int _unary_operator(C99 * c99)
	/* "&" | "*" | "+" | "-" | "~" | "!" */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__,
			token_get_string(c99->token));
#endif
	return c99_scan(c99);
}


/* assignment-operator */
static int _assignment_operator(C99 * c99)
	/* "=" | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^="
	 * | "|=" */
{
	/* FIXME implement */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return c99_scan(c99);
}


/* conditional-expr */
static int _conditional_expr(C99 * c99)
	/* logical-OR-expr [ "?" expr ":" conditional-expr ] */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	ret = _logical_or_expr(c99);
	if(token_get_code(c99->token) == C99_CODE_OPERATOR_QUESTION)
	{
		ret |= c99_scan(c99);
		ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_COLON);
		ret |= _conditional_expr(c99);
	}
	return ret;
}


/* logical-OR-expr */
static int _logical_or_expr(C99 * c99)
	/* logical-AND-expr { "||" logical-AND-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _logical_and_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_DBAR)
	{
		ret |= c99_scan(c99);
		ret |= _logical_and_expr(c99);
	}
	return ret;
}



/* logical-AND-expr */
static int _logical_and_expr(C99 * c99)
	/* inclusive-OR-expr { "&&" inclusive-OR-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _inclusive_or_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_DAMPERSAND)
	{
		ret |= c99_scan(c99);
		ret |= _inclusive_or_expr(c99);
	}
	return ret;
}


/* inclusive-OR-expr */
static int _inclusive_or_expr(C99 * c99)
	/* exclusive-OR-expr { "|" exclusive-OR-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _exclusive_or_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_BAR)
	{
		ret |= c99_scan(c99);
		ret |= _exclusive_or_expr(c99);
	}
	return ret;
}


/* exclusive-OR-expr */
static int _exclusive_or_expr(C99 * c99)
	/* AND-expr { "^" AND-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _and_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_XOR)
	{
		ret |= c99_scan(c99);
		ret |= _and_expr(c99);
	}
	return ret;
}


/* AND-expr */
static int _and_expr(C99 * c99)
	/* equality-expr { "&" equality-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _equality_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_AMPERSAND)
	{
		ret |= c99_scan(c99);
		ret |= _equality_expr(c99);
	}
	return ret;
}


/* equality-expr */
static int _equality_expr(C99 * c99)
	/* relational-expr { ("==" | "!=") relational-expr } */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _relational_expr(c99);
	while((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_DEQUALS
			|| code == C99_CODE_OPERATOR_NEQUALS)
	{
		ret |= c99_scan(c99);
		ret |= _relational_expr(c99);
	}
	return ret;
}


/* relational-expr */
static int _relational_expr(C99 * c99)
	/* shift-expr { ("<" | ">" | "<=" | ">=") shift-expr } */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _shift_expr(c99);
	while((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_LESS
			|| code == C99_CODE_OPERATOR_GREATER
			|| code == C99_CODE_OPERATOR_LEQUALS
			|| code == C99_CODE_OPERATOR_GEQUALS)
	{
		ret |= c99_scan(c99);
		ret |= _shift_expr(c99);
	}
	return ret;
}


/* shift-expr */
static int _shift_expr(C99 * c99)
	/* additive-expr { ("<<" | ">>") additive-expr } */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _additive_expr(c99);
	while((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_DLESS
			|| code == C99_CODE_OPERATOR_DGREATER)
	{
		ret |= c99_scan(c99);
		ret |= _additive_expr(c99);
	}
	return ret;
}


/* additive-expr */
static int _additive_expr(C99 * c99)
	/* multiplicative-expr { ("+" | "-") multiplicative-expr } */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _multiplicative_expr(c99);
	while((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_PLUS
			|| code == C99_CODE_OPERATOR_MINUS)
	{
		ret |= c99_scan(c99);
		ret |= _multiplicative_expr(c99);
	}
	return ret;
}


/* multiplicative-expr */
static int _multiplicative_expr(C99 * c99)
	/* cast-expr { ("*" | "/" | "%") cast-expr } */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _cast_expr(c99);
	while((code = token_get_code(c99->token)) == C99_CODE_OPERATOR_TIMES
			|| code == C99_CODE_OPERATOR_DIVIDE
			|| code == C99_CODE_OPERATOR_MODULO)
	{
		ret |= c99_scan(c99);
		ret |= _cast_expr(c99);
	}
	return ret;
}


/* cast-expr */
static int _cast_expr(C99 * c99)
	/* { "(" type-name ")" } unary-expr */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	while(token_get_code(c99->token) == C99_CODE_OPERATOR_LPAREN)
	{
		ret |= c99_scan(c99);
		ret |= _type_name(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	ret |= _unary_expr(c99);
	return ret;
}


/* compound-statement */
static int _compound_statement(C99 * c99)
	/* "{" [ block-item-list ] "}" */
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"{\" got \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	c99_scan(c99);
	if(token_in_set(c99->token, c99set_block_item_list))
		_block_item_list(c99);
	return c99_scan(c99);
}


/* block-item-list */
static int _block_item_list(C99 * c99)
	/* block-item { block-item } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _block_item(c99);
	while(token_in_set(c99->token, c99set_block_item))
		ret |= _block_item(c99);
	return ret;
}


/* block-item */
static int _block_item(C99 * c99)
	/* declaration | statement */
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_declaration))
		return _declaration(c99);
	else if(token_in_set(c99->token, c99set_statement))
		return _statement(c99);
	return 1; /* FIXME report error */
}


/* statement */
static int _statement(C99 * c99)
	/* labeled-statement
	 * compound-statement
	 * expression-statement
	 * selection-statement
	 * iteration-statement
	 * jump-statement */
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME labeled and expression statements conflict */
	if(c99->in_switch && token_in_set(c99->token, c99set_labeled_statement))
		return _labeled_statement(c99);
	else if(token_in_set(c99->token, c99set_compound_statement))
		return _compound_statement(c99);
	else if(token_in_set(c99->token, c99set_expression_statement))
		return _expression_statement(c99);
	else if(token_in_set(c99->token, c99set_selection_statement))
		return _selection_statement(c99);
	else if(token_in_set(c99->token, c99set_iteration_statement))
		return _iteration_statement(c99);
	else if(token_in_set(c99->token, c99set_jump_statement))
		return _jump_statement(c99);
	return _parse_error(c99, "Expected statement");
}


/* labeled-statement */
static int _labeled_statement(C99 * c99)
	/* identifier ":" statement
	 * case constant-expr ":" statement
	 * default ":" statement */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__,
			token_get_string(c99->token));
#endif
	if((code = token_get_code(c99->token)) == C99_CODE_IDENTIFIER)
		ret = _identifier(c99);
	else if(code == C99_CODE_KEYWORD_CASE)
	{
		ret = c99_scan(c99);
		ret |= _constant_expr(c99);
	}
	else /* default */
		ret = c99_scan(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_COLON);
	ret |= _statement(c99);
	return ret;
}


/* constant_expr */
static int _constant_expr(C99 * c99)
	/* conditional-expr */
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return _conditional_expr(c99);
}


/* expression-statement */
static int _expression_statement(C99 * c99)
	/* [ expression ] ";" */
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	if(token_in_set(c99->token, c99set_expression))
		ret = _expression(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	return ret;
}


/* expression */
static int _expression(C99 * c99)
	/* assignment-expr { "," assignment-expr } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _assignment_expr(c99);
	while(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		c99_scan(c99);
		ret |= _assignment_expr(c99);
	}
	return ret;
}


/* selection-statement */
static int _selection_statement(C99 * c99)
	/* if "(" expression ")" statement [ else statement ]
	 * switch "(" expression ")" statement */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((code = token_get_code(c99->token)) == C99_CODE_KEYWORD_SWITCH)
		c99->in_switch++;
	ret = c99_scan(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
	ret |= _expression(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	ret |= _statement(c99);
	if(code == C99_CODE_KEYWORD_SWITCH)
		c99->in_switch--;
	else if(code == C99_CODE_KEYWORD_IF
			&& token_get_code(c99->token) == C99_CODE_KEYWORD_ELSE)
	{
		ret |= c99_scan(c99);
		ret |= _statement(c99);
	}
	return ret;
}


/* iteration-statement */
static int _iteration_statement(C99 * c99)
	/* while "(" expression ")" statement
	 * do statement while "(" expression ")" ;
	 * for "(" [ (expr | declaration) ] ; [ expr ] ; [ expr ] ")" statement */
{
	int ret = 0;
	int code;

	/* FIXME complete */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((code = token_get_code(c99->token)) == C99_CODE_KEYWORD_WHILE)
	{
		ret = c99_scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _statement(c99);
	}
	else if(code == C99_CODE_KEYWORD_DO)
	{
		ret = c99_scan(c99);
		ret |= _statement(c99);
		ret |= _parse_check(c99, C99_CODE_KEYWORD_WHILE);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
	}
	else if(code == C99_CODE_KEYWORD_FOR)
	{
		ret = c99_scan(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_LPAREN);
		/* FIXME or declaration */
		if(token_get_code(c99->token) != C99_CODE_OPERATOR_SEMICOLON)
			ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
		if(token_get_code(c99->token) != C99_CODE_OPERATOR_SEMICOLON)
			ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
		if(token_get_code(c99->token) != C99_CODE_OPERATOR_SEMICOLON)
			ret |= _expression(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RPAREN);
		ret |= _statement(c99);
	}
	return ret;
}


/* jump-statement */
static int _jump_statement(C99 * c99)
	/* goto identifier ;
	 * continue ;
	 * break ;
	 * return [ expression ] ; */
{
	int ret;
	int code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__,
			token_get_string(c99->token));
#endif
	if((code = token_get_code(c99->token)) == C99_CODE_KEYWORD_GOTO)
	{
		c99_scan(c99);
		ret = _identifier(c99);
	}
	else if(code == C99_CODE_KEYWORD_RETURN)
	{
		ret = c99_scan(c99);
		if(token_in_set(c99->token, c99set_expression))
			ret = _expression(c99);
	}
	else /* continue or break */
		ret = c99_scan(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_SEMICOLON);
	return ret;
}


/* init-declarator-list */
static int _init_declarator_list(C99 * c99)
	/* init-declarator { init-declarator } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _init_declarator(c99);
	while(token_in_set(c99->token, c99set_init_declarator))
		ret |= _init_declarator(c99);
	return ret;
}


/* init-declarator */
static int _init_declarator(C99 * c99)
	/* declarator [ "=" initializer ] */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _declarator(c99);
	if(token_get_code(c99->token) == C99_CODE_OPERATOR_EQUALS)
	{
		ret |= c99_scan(c99);
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_get_code(c99->token) == C99_CODE_OPERATOR_LBRACE)
	{
		ret = c99_scan(c99);
		ret |= _initializer_list(c99);
		if(token_get_code(c99->token) == C99_CODE_COMMA)
			ret |= c99_scan(c99);
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

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_in_set(c99->token, c99set_designation))
		ret |= _designation(c99);
	ret |= _initializer(c99);
	while(token_get_code(c99->token) == C99_CODE_COMMA)
	{
		ret |= c99_scan(c99);
		if(token_in_set(c99->token, c99set_designation))
			ret |= _designation(c99);
		ret |= _initializer(c99);
	}
	return ret;
}


/* designation */
static int _designation(C99 * c99)
	/* designator-list "=" */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _designator_list(c99);
	ret |= _parse_check(c99, C99_CODE_OPERATOR_EQUALS);
	return ret;
}


/* designator-list */
static int _designator_list(C99 * c99)
	/* designator { designator } */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ret = _designator(c99);
	while(token_in_set(c99->token, c99set_designator))
		ret |= _designator(c99);
	return ret;
}


/* designator */
static int _designator(C99 * c99)
	/* "[" constant-expression "]"
	 * identifier */
{
	int ret;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(token_get_code(c99->token) == C99_CODE_OPERATOR_LBRACKET)
	{
		ret = c99_scan(c99);
		ret |= _constant_expr(c99);
		ret |= _parse_check(c99, C99_CODE_OPERATOR_RBRACKET);
	}
	else
		ret = _identifier(c99);
	return ret;
}


/* public */
/* functions */
/* useful */
/* c99_parse */
static int _parse_E(C99 * c99);

int c99_parse(C99 * c99)
{
	int ret;

	if(c99->flags & C99PREFS_E)
		return _parse_E(c99);
	if((ret = _translation_unit(c99)) != 0)
		unlink(c99->outfile);
	return ret;
}

static int _parse_E(C99 * c99)
{
	int ret;
	Token * token;
	int code;

	while((ret = cpp_scan(c99->cpp, &token)) == 0
			&& token != NULL)
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
