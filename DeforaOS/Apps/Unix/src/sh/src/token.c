/* token.c */



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
	"||",
	";;",
	"<<",
	">>",
	"<&",
	">&",
	"<>",
	"<<-",
	">|",
	"&",
	"|",
	";",
	"<",
	">",
	"if",
	"in"
};

Token * token_new(TokenCode code, char * string)
{
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s%d%s%s%s", "token_new(", code, ", ",
			string != NULL ? string : "NULL", ")\n");
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
	free(token->string);
	free(token);
}


int token_in_set(Token * token, TokenSet set)
{
	TokenCode set_cmd_name[] = { TC_WORD, TC_NULL };
	TokenCode set_cmd_prefix[] = {
		TC_IO_NUMBER, TC_ASSIGNMENT_WORD, TC_NULL
	};
	TokenCode set_cmd_suffix[] = { TC_IO_NUMBER, TC_WORD, TC_NULL };
	TokenCode set_cmd_word[] = { TC_WORD, TC_NULL };
	TokenCode set_io_redirect[] = { TC_IO_NUMBER, TC_NULL };
	TokenCode set_newline_list[] = { TC_NEWLINE, TC_NULL };
	TokenCode set_separator[] = { TC_NEWLINE, TC_NULL };
	TokenCode set_separator_op[] = {
		TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NULL
	};
	TokenCode * sets[TS_LAST+1] = {
		set_cmd_name,
		set_cmd_prefix,
		set_cmd_suffix,
		set_cmd_word,
		set_io_redirect,
		set_newline_list,
		set_separator,
		set_separator_op
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
