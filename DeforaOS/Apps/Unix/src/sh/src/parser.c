/* parser.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tokenlist.h"
#include "parser.h"


/* types */
typedef TokenCode * CodeSet;


/* variables */
/* FIXME static Token token_null[1] = { TC_NULL, NULL }; */

static TokenCode CS_AND_OR[]		= {
	TC_RW_BANG,
	/* command */
	TC_TOKEN,
	TC_NULL
};
static TokenCode CS_CMD_PREFIX[]	= {
	TC_IONUMBER,
	TC_NULL
};
static TokenCode CS_CMD_SUFFIX[]	= {
	TC_IONUMBER,
	TC_NULL
};
static TokenCode CS_CMD_WORD[]		= {
	TC_TOKEN,
	TC_NULL
};
static TokenCode CS_COMMAND[]		= {
	/* simple command */
	TC_TOKEN,
	/* compound_command */
	TC_RW_LBRACE, /* SUBSHELL "(", */ TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	/* function_definition */
	TC_NULL
};
static TokenCode CS_COMPOUND_COMMAND[]	= {
	TC_RW_LBRACE, /* SUBSHELL "(", */
	TC_RW_FOR, TC_RW_CASE, TC_RW_IF, TC_RW_WHILE, TC_RW_UNTIL,
	TC_NULL
};
static TokenCode CS_FUNCTION_DEFINITION[] = {
	/* FIXME */ TC_NULL
};
static TokenCode CS_LIST[]		= {
	/* and_or */
	TC_RW_BANG, TC_TOKEN,
	/* separator_op */
	TC_OP_AMPERSAND, TC_OP_SEMICOLON,
	TC_NULL
};
static TokenCode CS_NEWLINE_LIST[]	= { TC_NEWLINE, TC_NULL }; /* FIXME */
static TokenCode CS_REDIRECT_LIST[]	= { TC_NULL }; /* FIXME */
static TokenCode CS_SEPARATOR[]		= { TC_NEWLINE, TC_NULL }; /* FIXME */
static TokenCode CS_SEPARATOR_OP[]	= { TC_OP_AMPERSAND, TC_OP_SEMICOLON, TC_NULL };
static TokenCode CS_SIMPLE_COMMAND[]	= {
	/* cmd_prefix */
	/* cmd_name */
	TC_WORD,
	TC_NULL };


/* functions */
static int token_in_set(Token * token, CodeSet set)
{
	int i;

	for(i = 0; set[i] != TC_NULL; i++)
		if(token->code == set[i])
			return 1;
	return 0;
}

static Token ** token_scan(Token ** tokens)
{
	return tokens + 1;
}

static Token ** token_check(Token ** tokens, TokenCode tokencode)
{
	if(tokens[0]->code != tokencode)
		fprintf(stderr, "Expected: %d (got: %d)\n",
				tokencode, tokens[0]->code);
	return token_scan(tokens);
}

static Token ** token_check_word(Token ** tokens, char * word)
{
	if(tokens[0]->code != TC_WORD || strcmp(word, tokens[0]->string) != 0)
	{
		fprintf(stderr, "Expected word \"%s\"\n", word);
		return NULL;
	}
	return &tokens[1];
}


/* parser */
/* complete_command */
static Token ** list(Token ** tokens);
static Token ** separator(Token ** tokens);
Token ** complete_command(TokenList * tokenlist)
	/* list [separator] */
{
	Token ** t;

	if(tokenlist == NULL)
		return NULL;
	t = tokenlist->tokens;
	if(!token_in_set(t[0], CS_LIST))
	{
		fprintf(stderr, "syntax error\n");
		return NULL;
	}
	t = list(t);
	if(token_in_set(t[0], CS_SEPARATOR))
		return separator(t);
	return t;
}


/* list */
static Token ** and_or(Token ** t);
static Token ** separator_op(Token ** tokens);
static Token ** list(Token ** tokens)
	/* list separator_op and_or
	 * | and_or */
	/* and_or { separator_op and_or } */
{
	tokens = and_or(tokens);
	while(token_in_set(tokens[0], CS_SEPARATOR_OP))
	{
		tokens = separator_op(tokens);
		tokens = separator_op(tokens);
	}
	return tokens;
/*	if(token_in_set(tokens[0], CS_AND_OR))
		return and_or(tokens);
	tokens = separator_op(tokens);
	tokens = and_or(tokens);
	while(token_in_set(tokens[0], CS_SEPARATOR_OP))
	{
		tokens = separator_op(tokens);
		tokens = and_or(tokens);
	}
	return tokens;*/
}


/* and_or */
static Token ** pipeline(Token ** tokens);
static Token ** linebreak(Token ** tokens);
static Token ** and_or(Token ** tokens)
	/* pipeline
	 * | and_or AND_IF linebreak pipeline
	 * | and_or OR_IF linebreak pipeline */
	/* pipeline { (AND_IF | OR_IF) linebreak pipeline } */
{
	tokens = pipeline(tokens);
	while(tokens[0]->code == TC_OP_AND_IF || tokens[0]->code == TC_OP_OR_IF)
	{
		tokens = token_scan(tokens);
		tokens = linebreak(tokens);
		tokens = pipeline(tokens);
	}
	return tokens;
/*	if(token_in_set(t[0], CS_AND_OR))
	{
		t = and_or(t);
		if(t[0]->code == TC_OP_AND_IF)
			t = token_scan(t);
		else
			t = token_check(t, TC_OP_OR_IF);
		t = linebreak(t);
	}
	return pipeline(t);*/
}


/* pipeline */
static Token ** pipe_sequence(Token ** tokens);
static Token ** pipeline(Token ** tokens)
	/* [Bang] pipe_sequence */
{
#ifdef DEBUG
	fprintf(stderr, "pipeline(): %s\n", tokens[0]->string);
#endif
	if(tokens[0]->code == TC_RW_BANG)
		tokens = token_scan(tokens);
	return pipe_sequence(tokens);
}


/* pipe_sequence */
static Token ** command(Token ** tokens);
static Token ** pipe_sequence(Token ** tokens)
	/* command
	 * pipe_sequence '|' linebreak command */
	/* { command } '|' command */
{
#ifdef DEBUG
	fprintf(stderr, "pipe_sequence(): %s\n", tokens[0]->string);
#endif
	while(token_in_set(*tokens, CS_COMMAND))
		tokens = command(tokens);
	if(tokens[0]->code == TC_WORD && strcmp("|", tokens[0]->string) == 0)
	{
		tokens = token_scan(tokens);
		return command(tokens);
	}
	return tokens;
}


/* command */
static Token ** compound_command(Token ** tokens);
static Token ** redirect_list(Token ** tokens);
static Token ** function_definition(Token ** tokens);
static Token ** simple_command(Token ** tokens);
static Token ** command(Token ** tokens)
	/* simple_command
	 * | compound_command
	 * | compound_command redirect_list
	 * | function_definition */
	/* simple_command
	 * | compound_command [redirect_list]
	 * | function_definition */
{
#ifdef DEBUG
	fprintf(stderr, "command(): %s\n", tokens[0]->string);
#endif
	token_distinct(tokens[0]);
	if(token_in_set(tokens[0], CS_COMPOUND_COMMAND))
	{
		tokens = compound_command(tokens);
		if(token_in_set(tokens[0], CS_REDIRECT_LIST))
			return redirect_list(tokens);
		return tokens;
	}
	if(token_in_set(tokens[0], CS_SIMPLE_COMMAND))
		return simple_command(tokens);
	if(token_in_set(tokens[0], CS_FUNCTION_DEFINITION))
		return function_definition(tokens);
	return NULL;
}


/* compound_command */
static Token ** brace_group(Token ** tokens);
static Token ** subshell(Token ** tokens);
static Token ** for_clause(Token ** tokens);
static Token ** case_clause(Token ** tokens);
static Token ** if_clause(Token ** tokens);
static Token ** while_clause(Token ** tokens);
static Token ** until_clause(Token ** tokens);
static Token ** compound_command(Token ** tokens)
	/* brace_group
	 * | subshell
	 * | for_clause
	 * | case_clause
	 * | if_clause
	 * | while_clause
	 * | until_clause */
{
#ifdef DEBUG
	fprintf(stderr, "compound_command(): %s\n", tokens[0]->string);
#endif
	/* FIXME */
	switch(tokens[0]->code)
	{
		case TC_RW_LBRACE:
			return brace_group(tokens);
		case TC_RW_FOR:
			return for_clause(tokens);
		case TC_RW_CASE:
			return case_clause(tokens);
		case TC_RW_IF:
			return if_clause(tokens);
		case TC_RW_WHILE:
			return while_clause(tokens);
		case TC_RW_UNTIL:
			return until_clause(tokens);
		default:
			return NULL;
	}
	if(token_check_word(tokens, "(") != NULL)
		return subshell(tokens);
	return NULL;
}


/* subshell */
static Token ** subshell(Token ** tokens)
{
	/* FIXME */
	return NULL;
}


/* for_clause */
static Token ** for_clause(Token ** tokens)
{
	/* FIXME */
	return NULL;
}


/* case_clause */
static Token ** case_clause(Token ** tokens)
{
	/* FIXME */
	return NULL;
}


/* if_clause */
static Token ** if_clause(Token ** tokens)
{
	/* FIXME */
	return NULL;
}


/* while_clause */
static Token ** do_group(Token ** tokens);
static Token ** while_clause(Token ** tokens)
	/* While compound_list do_group */
{
	/* FIXME */
	tokens = token_scan(tokens);
/*	tokens = compound_list */
	return do_group(tokens);
}


/* until_clause */
static Token ** until_clause(Token ** tokens)
	/* Until compound_list do_group */
{
	/* FIXME */
	tokens = token_scan(tokens);
/*	tokens = compound_list */
	return do_group(tokens);
}


/* brace_group */
static Token ** brace_group(Token ** tokens)
	/* Lbrace compound_list Rbrace */
{
	/* FIXME */
	tokens = token_scan(tokens);
/*	tokens = compound_list(tokens); */
	return token_check(tokens, TC_RW_RBRACE);
}


/* do_group */
static Token ** do_group(Token ** tokens)
	/* Do compound_list Done */
{
	/* FIXME */
	tokens = token_scan(tokens);
/*	tokens = compound_list(tokens); */
	return token_check(tokens, TC_RW_DONE);
}


/* function_definition */
static Token ** function_body(Token ** tokens);
static Token ** fname(Token ** tokens);
static Token ** function_definition(Token ** tokens)
	/* fname '(' ')' linebreak function_body */
{
#ifdef DEBUG
	fprintf(stderr, "function_definition(): %s\n", tokens[0]->string);
#endif
	tokens = fname(tokens);
	tokens = token_check_word(tokens, "(");
	tokens = token_check_word(tokens, ")");
	tokens = token_check(tokens, TC_NEWLINE);
	return function_body(tokens);
}


/* function_body */
static Token ** function_body(Token ** tokens)
	/* compound_command [redirect_list] (rule 9) */
{
#ifdef DEBUG
	fprintf(stderr, "function_body(): %s\n", tokens[0]->string);
#endif
	tokens = compound_command(tokens);
	if(token_in_set(tokens[0], CS_REDIRECT_LIST))
		return redirect_list(tokens);
	return tokens;
}


/* fname */
static Token ** fname(Token ** tokens)
	/* NAME (rule 8) */
{
#ifdef DEBUG
	fprintf(stderr, "fname(): %s\n", tokens[0]->string);
#endif
	/* FIXME */
	return token_scan(tokens);
}


/* simple_command */
static Token ** cmd_prefix(Token ** tokens);
static Token ** cmd_word(Token ** tokens);
static Token ** cmd_suffix(Token ** tokens);
static Token ** cmd_name(Token ** tokens);
static Token ** simple_command(Token ** tokens)
	/* cmd_prefix cmd_word cmd_suffix
	 * | cmd_prefix cmd_word
	 * | cmd_prefix
	 * | cmd_name cmd_suffix
	 * | cmd_name */
	/* cmd_prefix [cmd_word [cmd_suffix]]
	 * | cmd_name [cmd_suffix] */
{
#ifdef DEBUG
	fprintf(stderr, "simple_command(): %s\n", tokens[0]->string);
#endif
	if(tokens[0]->code == TC_WORD)
	{
		tokens = cmd_name(tokens);
		if(token_in_set(tokens[0], CS_CMD_SUFFIX))
			return cmd_suffix(tokens);
		return tokens;
	}
	tokens = cmd_prefix(tokens);
	if(token_in_set(tokens[0], CS_CMD_WORD))
	{
		tokens = cmd_word(tokens);
		if(token_in_set(tokens[0], CS_CMD_SUFFIX))
			tokens = cmd_suffix(tokens);
	}
	return tokens;
}


/* cmd_name */
static Token ** cmd_name(Token ** tokens)
	/* WORD (rule 7a) */
{
#ifdef DEBUG
	fprintf(stderr, "cmd_name(): %s\n", tokens[0]->string);
#endif
	return NULL;
}


/* cmd_word */
static Token ** cmd_word(Token ** tokens)
	/* WORD (rule 7b) */
{
#ifdef DEBUG
	fprintf(stderr, "cmd_word(): %s\n", tokens[0]->string);
#endif
	return NULL;
}


/* cmd_prefix */
static Token ** cmd_prefix(Token ** tokens)
	/* io_redirect
	 * | cmd_prefix io_redirect
	 * | ASSIGNMENT_WORD
	 * | cmd_prefix ASSIGNMENT_WORD */
{
#ifdef DEBUG
	fprintf(stderr, "cmd_prefix(): %s\n", tokens[0]->string);
#endif
	return NULL;
}


/* cmd_suffix */
static Token ** cmd_suffix(Token ** tokens)
	/* io_redirect
	 * | cmd_suffix io_redirect
	 * | WORD
	 * | cmd_suffix WORD */
	/* io_redirect { io_redirect }
	 * | WORD { WORD } */
{
#ifdef DEBUG
	fprintf(stderr, "cmd_suffix(): %s\n", tokens[0]->string);
#endif
	return NULL;
}


/* redirect_list */
static Token ** io_redirect(Token ** tokens);
static Token ** redirect_list(Token ** tokens)
	/* io_redirect
	 * | redirect_list io_redirect */
	/* io_redirect { io_redirect } */
{
#ifdef DEBUG
	fprintf(stderr, "redirect_list(): %s\n", tokens[0]->string);
#endif
	tokens = io_redirect(tokens);
	while(token_in_set(tokens[0], CS_REDIRECT_LIST))
		tokens = io_redirect(tokens);
	return tokens;
}


/* io_redirect */
static Token ** io_redirect(Token ** tokens)
	/* io_file
	 * | IO_NUMBER io_file
	 * | io_here
	 * | IO_NUMBER io_here */
{
#ifdef DEBUG
	fprintf(stderr, "io_redirect(): %s\n", tokens[0]->string);
#endif
	return NULL;
}


/* newline_list */
static Token ** newline_list(Token ** t)
	/* NEWLINE { NEWLINE } */
{
#ifdef DEBUG
	fprintf(stderr, "newline_list()\n");
#endif
	t = token_check(t, TC_NEWLINE);
	while(t[0]->code == TC_NEWLINE)
		t = token_scan(t);
	return t;
}


/* linebreak */
static Token ** linebreak(Token ** tokens)
	/* newline_list | */
{
#ifdef DEBUG
	fprintf(stderr, "linebreak()\n");
#endif
	if(tokens[0]->code == TC_NEWLINE)
		tokens = token_scan(tokens);
	return tokens;
}


/* separator_op */
static Token ** separator_op(Token ** tokens)
	/* '&' | ';' */
{
#ifdef DEBUG
	fprintf(stderr, "separator_op(): %s\n", tokens[0]->string);
#endif
	if(tokens[0]->code == TC_OP_AMPERSAND)
		return token_scan(tokens);
	if(tokens[0]->code == TC_OP_SEMICOLON)
		return token_scan(tokens);
#ifdef DEBUG
	fprintf(stderr, "separator_op(): should not happen!\n");
#endif
	return NULL;
}


/* separator */
static Token ** separator(Token ** tokens)
	/* (separator_op linebreak | newline_list) */
{
#ifdef DEBUG
	fprintf(stderr, "separator()\n");
#endif
	if(token_in_set(*tokens, CS_NEWLINE_LIST))
		return newline_list(tokens);
	tokens = separator_op(tokens);
	return linebreak(tokens);
}
