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



#include <string.h>
#include <ctype.h>
#include "common.h"
#include "c99.h"


/* private */
/* variables */
static char * _tokens[] =
{
	NULL, ",", "\"",
	/* directives */
	"#define", "#elif", "#else", "#endif", "#error", "#if", "#ifdef",
	"#ifndef", "#include", "#line", "#pragma", "#undef", "#warning",
	/* operators */
	"&=", "&", "|", "|=", ":", "&&", "||", "==", ">>=", ">>", "##", "/=",
	"/", "<<=", "<<", "--", ".", "...", "++", "=", ">=", ">", "#", "{",
	"[", "<=", "<", "(", "-=", "->", "-", "%=", "%", "!=", "!", "+=", "+",
	"?", "}", "]", ")", ";", "*=", "~", "*", "^=", "^",
	/* more codes */
	"'", "whitespace", "word", "constant", "identifier",
	/* keywords */
	"auto", "break", "case", "char", "const", "continue", "default", "do",
	"double", "else", "enum", "extern", "float", "for", "goto", "if",
	"inline", "int", "long", "register", "restrict", "return", "short",
	"signed", "sizeof", "static", "struct", "switch", "typedef", "union",
	"unsigned", "void", "volatile", "while", "_Bool", "_Complex",
	"_Imaginary"
};


/* protected */
/* functions */
/* accessors */
char const * code_get_string(TokenCode code)
{
	return _tokens[code];
}


/* public */
/* functions */
/* useful */
/* c99_scan */
static int _scan_skip_meta(C99 * c99);

int c99_scan(C99 * c99)
{
	int ret;
	char const * string;
	size_t i;
	int c;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(c99->token != NULL)
		token_delete(c99->token);
	if((ret = _scan_skip_meta(c99)) != 0
			|| c99->token == NULL)
		return ret;
	if(token_get_code(c99->token) != C99_CODE_WORD)
		return 0;
	if((string = token_get_string(c99->token)) == NULL)
		return 0;
	for(i = C99_CODE_KEYWORD_FIRST; i <= C99_CODE_KEYWORD_LAST; i++)
		if(strcmp(_tokens[i], string) == 0)
		{
			token_set_code(c99->token, i);
			return 0;
		}
	c = string[0];
	if(isalpha(c))
		token_set_code(c99->token, C99_CODE_IDENTIFIER);
	else if(isdigit(c)) /* FIXME make a stricter check? */
		token_set_code(c99->token, C99_CODE_CONSTANT);
	return 0;
}

static int _scan_skip_meta(C99 * c99)
{
	int ret;
	int code;

	while((ret = cpp_scan(c99->cpp, &c99->token)) == 0)
	{
		if(c99->token == NULL)
			return 0;
		if((code = token_get_code(c99->token)) != C99_CODE_WHITESPACE
				&& (code < C99_CODE_META_FIRST
					|| code > C99_CODE_META_LAST))
			break;
		token_delete(c99->token);
	}
	return ret;
}
