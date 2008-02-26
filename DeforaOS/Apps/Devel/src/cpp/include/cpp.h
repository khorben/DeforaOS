/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef CPP_CPP_H
# define CPP_CPP_H

# include <System.h>


/* Cpp */
/* types */
typedef struct _Cpp Cpp;

typedef enum _CppFilter { CPP_FILTER_TRIGRAPH = 1 } CppFilter;

typedef enum _CppCode
{
	CPP_CODE_DQUOTE = 0,
	CPP_CODE_OPERATOR_AEQUALS,
	CPP_CODE_OPERATOR_AMPERSAND,
	CPP_CODE_OPERATOR_BAR,
	CPP_CODE_OPERATOR_BEQUALS,
	CPP_CODE_OPERATOR_COLON,
	CPP_CODE_OPERATOR_DAMPERSAND,
	CPP_CODE_OPERATOR_DBAR,
	CPP_CODE_OPERATOR_DEQUALS,
	CPP_CODE_OPERATOR_DGREATER,
	CPP_CODE_OPERATOR_DIVIDE,
	CPP_CODE_OPERATOR_DLESS,
	CPP_CODE_OPERATOR_DMINUS,
	CPP_CODE_OPERATOR_DOT,
	CPP_CODE_OPERATOR_DPLUS,
	CPP_CODE_OPERATOR_EQUALS,
	CPP_CODE_OPERATOR_GREATER,
	CPP_CODE_OPERATOR_INVERSE,
	CPP_CODE_OPERATOR_LBRACE,
	CPP_CODE_OPERATOR_LBRACKET,
	CPP_CODE_OPERATOR_LESS,
	CPP_CODE_OPERATOR_LPAREN,
	CPP_CODE_OPERATOR_MASK,
	CPP_CODE_OPERATOR_MEQUALS,
	CPP_CODE_OPERATOR_MGREATER,
	CPP_CODE_OPERATOR_MINUS,
	CPP_CODE_OPERATOR_MODULO,
	CPP_CODE_OPERATOR_MORE,
	CPP_CODE_OPERATOR_MULTIPLY,
	CPP_CODE_OPERATOR_NOT,
	CPP_CODE_OPERATOR_OR,
	CPP_CODE_OPERATOR_PEQUALS,
	CPP_CODE_OPERATOR_PLUS,
	CPP_CODE_OPERATOR_RBRACE,
	CPP_CODE_OPERATOR_RBRACKET,
	CPP_CODE_OPERATOR_RPAREN,
	CPP_CODE_OPERATOR_SEMICOLON,
	CPP_CODE_OPERATOR_XOR,
	CPP_CODE_SQUOTE,
	CPP_CODE_WHITESPACE,
	CPP_CODE_WORD,			/* FIXME numbers and keywords? */
					/* FIXME comma? */
	CPP_CODE_UNKNOWN
} CppCode;


/* functions */
Cpp * cpp_new(void);
void cpp_delete(Cpp * cpp);

/* accessors */
char const * cpp_get_filename(Cpp * cpp);

/* useful */
void cpp_filter_disable(Cpp * cpp, CppFilter filter);
void cpp_filter_enable(Cpp * cpp, CppFilter filter);

int cpp_parse(Cpp * cpp, char const * pathname);

int cpp_scan(Cpp * cpp, Token ** token);

#endif /* !CPP_CPP_H */
