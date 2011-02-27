/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include "token.h"
#include "sh.h"


/* Token */
/* public */
/* variables */
char const * sTokenCode[TC_COUNT] =
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


/* functions */
/* token_new */
Token * token_new(TokenCode code, char * string)
{
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s%u%s%s%s", "token_new(", code, ", ",
			string != NULL ? string : sTokenCode[code], ")\n");
#endif
	if((t = malloc(sizeof(*t))) == NULL)
	{
		sh_error("malloc", 0);
		return NULL;
	}
	t->code = code;
	t->string = string;
	return t;
}


/* token_delete */
void token_delete(Token * token)
{
#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s", "token_delete(", token->code, ", ",
			token->string != NULL ? token->string : "NULL", ")\n");
#endif
	free(token->string);
	free(token);
}


/* useful */
/* token_in_set */
int token_in_set(Token * token, TokenSet set)
{
	static TokenCode set_and_or[] = {
		TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE,
		TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
		/* FIXME io_file too? */
		TC_IO_NUMBER, TC_ASSIGNMENT_WORD, TC_WORD, TC_NULL
	};
	static TokenCode set_cmd_name[] = { TC_WORD, TC_NULL };
	static TokenCode set_cmd_prefix[] = {
		TC_IO_NUMBER, TC_ASSIGNMENT_WORD, TC_NULL
	};
	/* cmd_suffix: WORD, io_redirect */
	static TokenCode set_cmd_suffix[] = {
		TC_WORD, TC_IO_NUMBER, TC_OP_LESS, TC_OP_LESSAND, TC_OP_GREAT,
		TC_OP_GREATAND, TC_NULL };
	static TokenCode set_cmd_word[] = { TC_WORD, TC_NULL };
	static TokenCode set_compound_command[] = {
		TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE,
		TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL, TC_NULL
	};
	static TokenCode set_else_part[] = { TC_RW_ELSE, TC_RW_ELIF, TC_NULL };
	static TokenCode set_function_definition[] = { TC_NULL }; /* FIXME */
	static TokenCode set_io_file[] = {
		TC_OP_LESS, TC_OP_LESSAND, TC_OP_GREAT, TC_OP_GREATAND, TC_NULL
	};
	static TokenCode set_io_here[] = {
		TC_OP_DLESS, TC_OP_DLESSDASH, TC_NULL
	};
	/* io_redirect: IO_NUMBER, io_file, io_here */
	static TokenCode set_io_redirect[] = {
		TC_IO_NUMBER, TC_OP_LESS, TC_OP_LESSAND, TC_OP_GREAT,
		TC_OP_GREATAND, TC_OP_DLESS, TC_OP_DLESSDASH, TC_NULL };
	static TokenCode set_newline_list[] = { TC_NEWLINE, TC_NULL };
	static TokenCode set_separator[] = {
		TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NEWLINE, TC_NULL
	};
	static TokenCode set_separator_op[] = {
		TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NULL
	};
	static TokenCode set_wordlist[] = { TC_WORD, TC_NULL };
	static TokenCode * sets[TS_COUNT] = {
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
		set_io_redirect, /* redirect_list */
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
