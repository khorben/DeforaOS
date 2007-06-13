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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include "token.h"
#include "sh.h"


/* Token */
char const * sTokenCode[TC_LAST+1] =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"&&",
	"&",
	"||",
	"|",
	";;",
	";",
	"<<-",
	"<<",
	"<&",
	"<>",
	"<",
	">>",
	">&",
	">|",
	">",
	"if",
	"then",
	"else",
	"elif",
	"fi",
	"do",
	"done",
	"case",
	"esac",
	"while",
	"until",
	"for",
	"{",
	"}",
	"!",
	"in"
};

Token * token_new(TokenCode code, char * string)
{
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s", "token_new(", code, ", ",
			string != NULL ? string : sTokenCode[code], ")\n");
#endif
	if((t = malloc(sizeof(Token))) == NULL)
	{
		sh_error("malloc", 0);
		return NULL;
	}
	t->code = code;
	t->string = string;
	return t;
}


void token_delete(Token * token)
{
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s", "token_delete(", token->code, ", ",
			token->string != NULL ? token->string : "NULL", ")\n");
#endif
	free(token->string);
	free(token);
}


int token_in_set(Token * token, TokenSet set)
{
	TokenCode set_and_or[] = {
		TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE,
		TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
		/* FIXME io_file too? */
		TC_IO_NUMBER, TC_ASSIGNMENT_WORD, TC_WORD, TC_NULL
	};
	TokenCode set_cmd_name[] = { TC_WORD, TC_NULL };
	TokenCode set_cmd_prefix[] = {
		TC_IO_NUMBER, TC_ASSIGNMENT_WORD, TC_NULL
	};
	TokenCode set_cmd_suffix[] = { TC_IO_NUMBER, TC_WORD, TC_NULL };
	TokenCode set_cmd_word[] = { TC_WORD, TC_NULL };
	TokenCode set_compound_command[] = {
		TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE,
		TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL, TC_NULL
	};
	TokenCode set_else_part[] = { TC_RW_ELSE, TC_RW_ELIF, TC_NULL };
	/* FIXME */
	TokenCode set_function_definition[] = { TC_NULL };
	TokenCode set_io_file[] = {
		TC_OP_LESS, TC_OP_LESSAND, TC_OP_GREAT, TC_OP_GREATAND,
		TC_OP_DGREAT, TC_OP_LESSGREAT, TC_OP_CLOBBER, TC_NULL
	};
	TokenCode set_io_here[] = { TC_OP_DLESS, TC_OP_DLESSDASH, TC_NULL };
	TokenCode set_io_redirect[] = { TC_IO_NUMBER, TC_NULL };
	TokenCode set_newline_list[] = { TC_NEWLINE, TC_NULL };
	TokenCode * set_redirect_list = set_io_redirect;
	TokenCode set_separator[] = {
		TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NEWLINE, TC_NULL
	};
	TokenCode set_separator_op[] = {
		TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NULL
	};
	TokenCode set_wordlist[] = { TC_WORD, TC_NULL };
	TokenCode * sets[TS_LAST+1] = {
		set_and_or,
		set_cmd_name,
		set_cmd_prefix,
		set_cmd_suffix,
		set_cmd_word,
		set_compound_command,
		set_else_part,
		set_function_definition,
		set_io_file,
		set_io_here,
		set_io_redirect,
		set_newline_list,
		set_redirect_list,
		set_separator,
		set_separator_op,
		set_wordlist
	};
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s", "token_in_set(token, ", set, ")\n");
#endif
	for(i = 0; sets[set][i] != TC_NULL; i++)
		if(sets[set][i] == token->code)
			return 1;
	return 0;
}
