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



#include <string.h>
#include <ctype.h>
#include "common.h"
#include "c99.h"
#include "../config.h"


/* private */
/* variables */
static char * _tokens[C99_CODE_COUNT] =
{
	NULL, ",", "\"",
	/* directives */
	NULL, "#define", "#elif", "#else", "#endif", "#error", "#if", "#ifdef",
	"#ifndef", "#include", "#pragma", "#undef", "#warning",
	/* operators */
	"&=", "&", "|", "|=", ":", "&&", "||", "==", ">>=", ">>", "##", "/=",
	"/", "<<=", "<<", "--", ".", "...", "++", "=", ">=", ">", "#", "{",
	"[", "<=", "<", "(", "-=", "->", "-", "%=", "%", "!=", "!", "+=", "+",
	"?", "}", "]", ")", ";", "*=", "~", "*", "^=", "^",
	/* more codes */
	"'", "whitespace", "newline", "comment", "word", "unknown", "constant",
	"identifier",
	/* keywords */
	"auto", "break", "case", "char", "const", "continue", "default", "do",
	"double", "else", "enum", "extern", "float", "for", "goto", "if",
	"inline", "int", "long", "register", "restrict", "return", "short",
	"signed", "sizeof", "static", "struct", "switch", "typedef", "union",
	"unsigned", "void", "volatile", "while", "_Bool", "_Complex",
	"_Imaginary", "name"
};


/* protected */
/* functions */
/* useful */
/* scan */
static int _scan_skip_meta(C99 * c99);
static void _meta_error(C99 * c99, TokenCode code);

int scan(C99 * c99)
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
	if(token_get_code(c99->token) == C99_CODE_SQUOTE)
	{
		token_set_code(c99->token, C99_CODE_CONSTANT);
		return 0;
	}
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
	if(isalpha(c) || c == '_')
		token_set_code(c99->token, code_type_get(c99->code, string) >= 0
				? C99_CODE_TYPEDEF_NAME : C99_CODE_IDENTIFIER);
	else if(isdigit(c)) /* FIXME make a stricter check? */
		token_set_code(c99->token, C99_CODE_CONSTANT);
	return 0;
}

static int _scan_skip_meta(C99 * c99)
{
	TokenCode code;

	while(cpp_scan(c99->cpp, &c99->token) == 0)
	{
		if(code_token(c99->code, c99->token) != 0)
			return 1;
		if(c99->token == NULL)
			return 0;
		if((code = token_get_code(c99->token)) != C99_CODE_WHITESPACE
				&& (code < C99_CODE_META_FIRST
					|| code > C99_CODE_META_LAST))
			return 0;
		if(code == C99_CODE_META_ERROR || code == C99_CODE_META_WARNING)
			_meta_error(c99, code);
		token_delete(c99->token);
	}
	return 1;
}

static void _meta_error(C99 * c99, TokenCode code)
{
	if(code == C99_CODE_META_ERROR)
		c99->error_cnt++;
	else
		c99->warning_cnt++;
	fprintf(stderr, "%s%s%s%u%s%s%s%s\n", PACKAGE ": ",
			token_get_filename(c99->token), ":",
			token_get_line(c99->token), ": ",
			(code == C99_CODE_META_ERROR) ? "error" : "warning",
			": ", token_get_string(c99->token));
}


/* accessors */
char const * tokencode_get_string(TokenCode code)
{
	return _tokens[code];
}
