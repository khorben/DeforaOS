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

typedef enum _CppFilter
{
	CPP_FILTER_NONE		= 0,
	CPP_FILTER_TRIGRAPH	= 1,
	CPP_FILTER_WHITESPACE	= 2,
	CPP_FILTER_COMMENT	= 4
} CppFilter;

typedef enum _CppCode
{
	CPP_CODE_NULL = TC_NULL,
	CPP_CODE_COMMA,
	CPP_CODE_DQUOTE,
	CPP_CODE_META_DATA,
	CPP_CODE_META_DEFINE,
	CPP_CODE_META_ELIF,
	CPP_CODE_META_ELSE,
	CPP_CODE_META_ENDIF,
	CPP_CODE_META_ERROR,
	CPP_CODE_META_IF,
	CPP_CODE_META_IFDEF,
	CPP_CODE_META_IFNDEF,
	CPP_CODE_META_INCLUDE,
	CPP_CODE_META_PRAGMA,
	CPP_CODE_META_UNDEF,
	CPP_CODE_META_WARNING,
	CPP_CODE_OPERATOR_AEQUALS,
	CPP_CODE_OPERATOR_AMPERSAND,
	CPP_CODE_OPERATOR_BAR,
	CPP_CODE_OPERATOR_BEQUALS,
	CPP_CODE_OPERATOR_COLON,
	CPP_CODE_OPERATOR_DAMPERSAND,
	CPP_CODE_OPERATOR_DBAR,
	CPP_CODE_OPERATOR_DEQUALS,
	CPP_CODE_OPERATOR_DGEQUALS,
	CPP_CODE_OPERATOR_DGREATER,
	CPP_CODE_OPERATOR_DHASH,
	CPP_CODE_OPERATOR_DIVEQUALS,
	CPP_CODE_OPERATOR_DIVIDE,
	CPP_CODE_OPERATOR_DLEQUALS,
	CPP_CODE_OPERATOR_DLESS,
	CPP_CODE_OPERATOR_DMINUS,
	CPP_CODE_OPERATOR_DOT,
	CPP_CODE_OPERATOR_DOTDOTDOT,
	CPP_CODE_OPERATOR_DPLUS,
	CPP_CODE_OPERATOR_EQUALS,
	CPP_CODE_OPERATOR_GEQUALS,
	CPP_CODE_OPERATOR_GREATER,
	CPP_CODE_OPERATOR_HASH,
	CPP_CODE_OPERATOR_LBRACE,
	CPP_CODE_OPERATOR_LBRACKET,
	CPP_CODE_OPERATOR_LEQUALS,
	CPP_CODE_OPERATOR_LESS,
	CPP_CODE_OPERATOR_LPAREN,
	CPP_CODE_OPERATOR_MEQUALS,
	CPP_CODE_OPERATOR_MGREATER,
	CPP_CODE_OPERATOR_MINUS,
	CPP_CODE_OPERATOR_MODEQUALS,
	CPP_CODE_OPERATOR_MODULO,
	CPP_CODE_OPERATOR_NEQUALS,
	CPP_CODE_OPERATOR_NOT,
	CPP_CODE_OPERATOR_PEQUALS,
	CPP_CODE_OPERATOR_PLUS,
	CPP_CODE_OPERATOR_QUESTION,
	CPP_CODE_OPERATOR_RBRACE,
	CPP_CODE_OPERATOR_RBRACKET,
	CPP_CODE_OPERATOR_RPAREN,
	CPP_CODE_OPERATOR_SEMICOLON,
	CPP_CODE_OPERATOR_TEQUALS,
	CPP_CODE_OPERATOR_TILDE,
	CPP_CODE_OPERATOR_TIMES,
	CPP_CODE_OPERATOR_XEQUALS,
	CPP_CODE_OPERATOR_XOR,
	CPP_CODE_SQUOTE,
	CPP_CODE_WHITESPACE,
	CPP_CODE_NEWLINE,
	CPP_CODE_COMMENT,
	CPP_CODE_WORD,			/* FIXME numbers and keywords? */
	CPP_CODE_UNKNOWN
} CppCode;
# define CPP_CODE_LAST		CPP_CODE_UNKNOWN
# define CPP_CODE_COUNT		(CPP_CODE_LAST + 1)
# define CPP_CODE_META_FIRST	CPP_CODE_META_DATA
# define CPP_CODE_META_LAST	CPP_CODE_META_WARNING


/* functions */
Cpp * cpp_new(char const * filename, int filters);
void cpp_delete(Cpp * cpp);

/* accessors */
char const * cpp_get_filename(Cpp * cpp);

int cpp_is_defined(Cpp * cpp, char const * name);

/* useful */
int cpp_define_add(Cpp * cpp, char const * name, char const * value);
char const * cpp_define_get(Cpp * cpp, char const * name);
int cpp_define_remove(Cpp * cpp, char const * name);

int cpp_path_add(Cpp * cpp, char const * path);

int cpp_scan(Cpp * cpp, Token ** token);

#endif /* !CPP_CPP_H */
