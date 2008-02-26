/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef SH_TOKEN_H
# define SH_TOKEN_H


/* Token */
/* types */
typedef enum _TokenCode
{
	TC_EOI = 0,
	TC_TOKEN,
	TC_WORD,
	TC_ASSIGNMENT_WORD,
	TC_NAME,
	TC_NEWLINE,
	TC_IO_NUMBER,
	TC_OP_AND_IF,
	TC_OP_AMPERSAND,
	TC_OP_OR_IF,
	TC_OP_BAR,
	TC_OP_DSEMI,
	TC_OP_SEMICOLON,
	TC_OP_DLESSDASH,
	TC_OP_DLESS,
	TC_OP_LESSAND,
	TC_OP_LESSGREAT,
	TC_OP_LESS,
	TC_OP_DGREAT,
	TC_OP_GREATAND,
	TC_OP_CLOBBER,
	TC_OP_GREAT,
	TC_RW_IF,
	TC_RW_THEN,
	TC_RW_ELSE,
	TC_RW_ELIF,
	TC_RW_FI,
	TC_RW_DO,
	TC_RW_DONE,
	TC_RW_CASE,
	TC_RW_ESAC,
	TC_RW_WHILE,
	TC_RW_UNTIL,
	TC_RW_FOR,
	TC_RW_LBRACE,
	TC_RW_RBRACE,
	TC_RW_BANG,
	TC_RW_IN,
	TC_NULL
} TokenCode;
# define TC_LAST TC_RW_IN
# define TC_COUNT (TC_LAST + 1)
extern char const * sTokenCode[TC_COUNT];

typedef struct _Token
{
	TokenCode code;
	char * string;
} Token;

typedef enum _TokenSet
{
	TS_AND_OR = 0,
	TS_CMD_NAME,
	TS_CMD_PREFIX,
	TS_CMD_SUFFIX,
	TS_CMD_WORD,
	TS_COMPOUND_COMMAND,
	TS_ELSE_PART,
	TS_FUNCTION_DEFINITION,
	TS_IO_FILE,
	TS_IO_HERE,
	TS_IO_REDIRECT,
	TS_NEWLINE_LIST,
	TS_REDIRECT_LIST,
	TS_SEPARATOR,
	TS_SEPARATOR_OP,
	TS_WORDLIST
} TokenSet;
# define TS_LAST TS_WORDLIST


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

int token_in_set(Token * token, TokenSet set);

#endif /* !SH_TOKEN_H */
