/* parser.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tokenlist.h"
#include "parser.h"


/* types */
typedef TokenCode * CodeSet;


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
static TokenCode CS_COMPOUND_LIST[]	= {
	TC_NULL
}; /* FIXME */
static TokenCode CS_FUNCTION_DEFINITION[] = {
	TC_NULL
}; /* FIXME */
static TokenCode CS_IN[]		= {
	TC_RW_IN,
	TC_NULL
};
static TokenCode CS_LINEBREAK[]		= {
	TC_ANY,
	TC_NULL
};
static TokenCode CS_LIST[]		= {
	/* and_or */
	TC_RW_BANG, TC_TOKEN,
	/* separator_op */
	TC_OP_AMPERSAND, TC_OP_SEMICOLON,
	TC_NULL
};
static TokenCode CS_NAME[]		= {
	TC_NAME,
	TC_NULL
};
static TokenCode CS_NEWLINE_LIST[]	= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
static TokenCode CS_PIPELINE[]		= {
	TC_RW_BANG,
	TC_NULL
}; /* FIXME */
static TokenCode CS_REDIRECT_LIST[]	= {
	TC_NULL
}; /* FIXME */
static TokenCode CS_SEPARATOR[]		= {
	TC_NEWLINE,
	TC_NULL
}; /* FIXME */
static TokenCode CS_SEPARATOR_OP[]	= {
	TC_OP_AMPERSAND,
	TC_OP_SEMICOLON,
	TC_NULL
};
static TokenCode CS_SIMPLE_COMMAND[]	= {
	/* cmd_prefix */
	/* cmd_name */
	TC_WORD,
	TC_NULL
};
static TokenCode CS_WORDLIST[]		= {
	TC_NULL
}; /* FIXME */


/* functions */
static int check(TokenList * tokenlist, TokenCode tokencode)
{
	if((*tokenlist)->code != tokencode)
	{
		fprintf(stderr, "Expected: %d (got: %d)\n",
				tokencode, (*tokenlist)->code);
		return 0;
	}
	return 1;
}

static int check_set(TokenList * tokenlist, CodeSet set)
{
	Token * t;
	int i;

	t = tokenlist_first_token(tokenlist);
	for(i = 0; set[i] != TC_NULL; i++)
		if(t->code == set[i])
			return 1;
	return 0;
}

static int check_word(TokenList * tokenlist, char * word)
{
	if((*tokenlist)->code != TC_WORD
			|| strcmp(word, (*tokenlist)->string) != 0)
	{
		fprintf(stderr, "Expected word \"%s\"\n", word);
		return 0;
	}
	return 1;
}

static TokenList * error(char * text)
{
	fprintf(stderr, "%s\n", text);
	return null_tokenlist;
}

#define scan(tokenlist) tokenlist_next(tokenlist)

#ifdef DEBUG
static void debug_rule(char * rule, TokenList * tokenlist)
{
	fprintf(stderr, "%s(): %s\n", rule, (*tokenlist)->string);
}
#endif


/* parser */
/* complete_command */
static TokenList * list(TokenList * tokenlist);
static TokenList * separator(TokenList * tokenlist);
int complete_command(TokenList * tokenlist)
	/* list [separator] */
{
	if(tokenlist == NULL)
		return 1;
	if(!check_set(tokenlist, CS_LIST))
	{
		error("Expected a list");
		return 1;
	}
	tokenlist = list(tokenlist);
	if(check_set(tokenlist, CS_SEPARATOR))
		tokenlist = separator(tokenlist);
	return tokenlist == null_tokenlist ? 1 : 0;
	/* FIXME: check if there are unused tokens? */
}


/* list */
static TokenList * and_or(TokenList * tokenlist);
static TokenList * separator_op(TokenList * tokenlist);
static TokenList * list(TokenList * tokenlist)
	/* list separator_op and_or
	 * | and_or */
	/* and_or { separator_op and_or } */
{
#ifdef DEBUG
	debug_rule("list", tokenlist);
#endif
	tokenlist = and_or(tokenlist);
	while(check_set(tokenlist, CS_SEPARATOR_OP))
	{
		if((tokenlist = separator_op(tokenlist)) == null_tokenlist)
			return null_tokenlist;
		if(!check_set(tokenlist, CS_AND_OR))
			return error("Expected and_or");
		tokenlist = and_or(tokenlist);
	}
	return tokenlist;
}


/* and_or */
static TokenList * pipeline(TokenList * tokenlist);
static TokenList * linebreak(TokenList * tokenlist);
static TokenList * and_or(TokenList * tokenlist)
	/* pipeline
	 * | and_or AND_IF linebreak pipeline
	 * | and_or OR_IF linebreak pipeline */
	/* pipeline { (AND_IF | OR_IF) linebreak pipeline } */
{
#ifdef DEBUG
	debug_rule("and_or", tokenlist);
#endif
	tokenlist = pipeline(tokenlist);
	while(tokenlist_first_token(tokenlist)->code == TC_OP_AND_IF
			|| tokenlist_first_token(tokenlist)->code == TC_OP_OR_IF)
	{
		tokenlist = tokenlist_next(tokenlist);
		if((tokenlist = linebreak(tokenlist)) == null_tokenlist)
			return null_tokenlist;
		if(!check_set(tokenlist, CS_PIPELINE))
			return null_tokenlist;
		tokenlist = pipeline(tokenlist);
	}
	return tokenlist;
}


/* pipeline */
static TokenList * pipe_sequence(TokenList * tokenlist);
static TokenList * pipeline(TokenList * tokenlist)
	/* [Bang] pipe_sequence */
{
#ifdef DEBUG
	debug_rule("pipeline", tokenlist);
#endif
	if(tokenlist_first_token(tokenlist)->code == TC_RW_BANG)
		tokenlist = tokenlist_next(tokenlist);
	return pipe_sequence(tokenlist);
}


/* pipe_sequence */
static TokenList * command(TokenList * tokenlist);
static TokenList * pipe_sequence(TokenList * tokenlist)
	/* command
	 * pipe_sequence '|' linebreak command */
	/* command { '|' linebreak command } */
{
#ifdef DEBUG
	debug_rule("pipe_sequence", tokenlist);
#endif
	command(tokenlist);
	while(tokenlist_first_token(tokenlist)->code == TC_OP_BAR)
	{
		if((tokenlist = linebreak(tokenlist)) == NULL)
			return NULL;
		if(!check_set(tokenlist, CS_COMMAND))
			return null_tokenlist;
		tokenlist = command(tokenlist);
	}
	return tokenlist;
}


/* command */
static TokenList * compound_command(TokenList * tokenlist);
static TokenList * redirect_list(TokenList * tokenlist);
static TokenList * function_definition(TokenList * tokenlist);
static TokenList * simple_command(TokenList * tokenlist);
static TokenList * command(TokenList * tokenlist)
	/* simple_command
	 * | compound_command
	 * | compound_command redirect_list
	 * | function_definition */
	/* simple_command
	 * | compound_command [redirect_list]
	 * | function_definition */
{
#ifdef DEBUG
	debug_rule("command", tokenlist);
#endif
	token_distinct(tokenlist_first_token(tokenlist));
	if(check_set(tokenlist, CS_SIMPLE_COMMAND))
		return simple_command(tokenlist);
	if(check_set(tokenlist, CS_COMPOUND_COMMAND))
	{
		tokenlist = compound_command(tokenlist);
		if(check_set(tokenlist, CS_REDIRECT_LIST))
			return redirect_list(tokenlist);
		return tokenlist;
	}
	if(check_set(tokenlist, CS_FUNCTION_DEFINITION))
		return function_definition(tokenlist);
#ifdef DEBUG
	fprintf(stderr, "file %s, line %d: should never be reached\n",
			__FILE__, __LINE__);
#endif
	return null_tokenlist;
}


/* compound_command */
static TokenList * brace_group(TokenList * tokenlist);
static TokenList * subshell(TokenList * tokenlist);
static TokenList * for_clause(TokenList * tokenlist);
static TokenList * case_clause(TokenList * tokenlist);
static TokenList * if_clause(TokenList * tokenlist);
static TokenList * while_clause(TokenList * tokenlist);
static TokenList * until_clause(TokenList * tokenlist);
static TokenList * compound_command(TokenList * tokenlist)
	/* brace_group
	 * | subshell
	 * | for_clause
	 * | case_clause
	 * | if_clause
	 * | while_clause
	 * | until_clause */
{
#ifdef DEBUG
	debug_rule("compound_command", tokenlist);
#endif
	switch(tokenlist_first_token(tokenlist)->code)
	{
		case TC_RW_LBRACE:
			return brace_group(tokenlist);
		case TC_RW_FOR:
			return for_clause(tokenlist);
		case TC_RW_CASE:
			return case_clause(tokenlist);
		case TC_RW_IF:
			return if_clause(tokenlist);
		case TC_RW_WHILE:
			return while_clause(tokenlist);
		case TC_RW_UNTIL:
			return until_clause(tokenlist);
		default:
			return null_tokenlist;
	}
	if(check_word(tokenlist, "("))
		return subshell(tokenlist);
	return null_tokenlist;
}


/* subshell */
static TokenList * compound_list(TokenList * tokenlist);
static TokenList * subshell(TokenList * tokenlist)
	/* '(' compound_list ')' */
{
#ifdef DEBUG
	debug_rule("subshell", tokenlist);
#endif
	tokenlist = tokenlist_next(tokenlist);
	tokenlist = compound_list(tokenlist);
	return check_word(tokenlist, ")")
		? tokenlist_next(tokenlist) : null_tokenlist;
}


/* compound_list */
static TokenList * compound_list(TokenList * tokenlist)
{
#ifdef DEBUG
	debug_rule("compound_list", tokenlist);
#endif
	/* FIXME */
	return null_tokenlist;
}


/* for_clause */
static TokenList * do_group(TokenList * tokenlist);
static TokenList * name(TokenList * tokenlist);
static TokenList * in(TokenList * tokenlist);
static TokenList * wordlist(TokenList * tokenlist);
static TokenList * sequential_sep(TokenList * tokenlist);
static TokenList * for_clause(TokenList * tokenlist)
	/* For name linebreak do_group
	 * | For name linebreak in sequential_sep do_group
	 * | For name linebreak in wordlist sequential_sep do_group */
	/* For name linebreak [in [wordlist] sequential_sep] do_group */
{
#ifdef DEBUG
	debug_rule("for_clause", tokenlist);
#endif
	tokenlist = tokenlist_next(tokenlist);
	if(!check_set(tokenlist, CS_NAME))
		return null_tokenlist;
	if((tokenlist = name(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	if(!check_set(tokenlist, CS_LINEBREAK))
		return null_tokenlist;
	if((tokenlist = linebreak(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	if(check_set(tokenlist, CS_IN))
	{
		if((tokenlist = in(tokenlist)) == null_tokenlist)
			return null_tokenlist;
		if(check_set(tokenlist, CS_WORDLIST))
			if((tokenlist = wordlist(tokenlist)) == null_tokenlist)
				return null_tokenlist;
		if((tokenlist = sequential_sep(tokenlist)) == null_tokenlist)
			return null_tokenlist;
	}
	return do_group(tokenlist);
}


/* name */
static TokenList * name(TokenList * tokenlist)
	/* NAME (rule 5) */
{
	return tokenlist_next(tokenlist);
}


/* in */
static TokenList * in(TokenList * tokenlist)
	/* In (rule 6) */
{
	return tokenlist_next(tokenlist);
}


/* wordlist */
static TokenList * wordlist(TokenList * tokenlist)
	/* wordlist WORD
	 * | WORD */
	/* WORD { WORD } */
{
	tokenlist = tokenlist_next(tokenlist);
	token_distinct(tokenlist_first_token(tokenlist));
	while(check(tokenlist, TC_WORD))
	{
		tokenlist = tokenlist_next(tokenlist);
		token_distinct(tokenlist_first_token(tokenlist));
	}
	return tokenlist;
}


/* case_clause */
static TokenList * case_clause(TokenList * tokenlist)
	/* Case WORD linebreak in linebreak case_list Esac
	 * | Case WORD linebreak in linebreak case_list_ns Esac
	 * | Case WORD linebreak in linebreak Esac */
	/* Case WORD linebreak in linebreak [(case_list | case_list_ns)] Esac */
{
	tokenlist = tokenlist_next(tokenlist);
	token_distinct(tokenlist_first_token(tokenlist));
	if(!check(tokenlist, TC_WORD))
		return null_tokenlist;
	tokenlist = tokenlist_next(tokenlist);
	if((tokenlist = linebreak(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	if((tokenlist = in(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	if((tokenlist = linebreak(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	/* FIXME */
	return check(tokenlist, TC_RW_ESAC) ? scan(tokenlist) : null_tokenlist;
}


/* if_clause */
static TokenList * if_clause(TokenList * tokenlist)
	/* If compound_list Then compound_list else_part Fi
	 * If compound_list Then compound_list Fi */
	/* If compound_list Then compound_list [else_part] Fi */
{
#ifdef DEBUG
	debug_rule("if_clause", tokenlist);
#endif
	tokenlist = scan(tokenlist);
	if((tokenlist = compound_list(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	token_distinct(tokenlist_first_token(tokenlist));
	if(!check(tokenlist, TC_RW_THEN))
		return null_tokenlist;
	tokenlist = compound_list(scan(tokenlist));
	/* FIXME */
	token_distinct(tokenlist_first_token(tokenlist));
	return check(tokenlist, TC_RW_FI) ? scan(tokenlist) : null_tokenlist;
}


/* while_clause */
static TokenList * while_clause(TokenList * tokenlist)
	/* While compound_list do_group */
{
#ifdef DEBUG
	debug_rule("while_clause", tokenlist);
#endif
	tokenlist = tokenlist_next(tokenlist);
	if((tokenlist = compound_list(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	return do_group(tokenlist);
}


/* until_clause */
static TokenList * until_clause(TokenList * tokenlist)
	/* Until compound_list do_group */
{
#ifdef DEBUG
	debug_rule("while_clause", tokenlist);
#endif
	tokenlist = tokenlist_next(tokenlist);
	if((tokenlist = compound_list(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	return do_group(tokenlist);
}


/* brace_group */
static TokenList * brace_group(TokenList * tokenlist)
	/* Lbrace compound_list Rbrace */
{
	tokenlist = scan(tokenlist);
	if(!check_set(tokenlist, CS_COMPOUND_LIST))
		return null_tokenlist;
	tokenlist = compound_list(tokenlist);
	return check(tokenlist, TC_RW_RBRACE) ? scan(tokenlist) : null_tokenlist;
}


/* do_group */
static TokenList * do_group(TokenList * tokenlist)
	/* Do compound_list Done */
{
	tokenlist = scan(tokenlist);
	if(!check_set(tokenlist, CS_COMPOUND_LIST))
		return null_tokenlist;
	tokenlist = compound_list(tokenlist);
	return check(tokenlist, TC_RW_DONE)
		? scan(tokenlist) : null_tokenlist;
}


/* function_definition */
static TokenList * function_body(TokenList * tokenlist);
static TokenList * fname(TokenList * tokenlist);
static TokenList * function_definition(TokenList * tokenlist)
	/* fname '(' ')' linebreak function_body */
{
#ifdef DEBUG
	debug_rule("function_definition", tokenlist);
#endif
	tokenlist = fname(tokenlist);
	if(!check_word(tokenlist, "(") || !check_word(tokenlist, ")"))
		return null_tokenlist;
	if(!check_set(tokenlist, CS_LINEBREAK))
		return null_tokenlist;
	if((tokenlist = linebreak(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	return function_body(tokenlist);
}


/* function_body */
static TokenList * function_body(TokenList * tokenlist)
	/* compound_command [redirect_list] (rule 9) */
{
#ifdef DEBUG
	debug_rule("function_body", tokenlist);
#endif
	tokenlist = compound_command(tokenlist);
	if(check_set(tokenlist, CS_REDIRECT_LIST))
		return redirect_list(tokenlist);
	return tokenlist;
}


/* fname */
static TokenList * fname(TokenList * tokenlist)
	/* NAME (rule 8) */
{
#ifdef DEBUG
	debug_rule("fname", tokenlist);
#endif
	return tokenlist_next(tokenlist);
}


/* simple_command */
static TokenList * cmd_prefix(TokenList * tokenlist);
static TokenList * cmd_word(TokenList * tokenlist);
static TokenList * cmd_suffix(TokenList * tokenlist);
static TokenList * cmd_name(TokenList * tokenlist);
static TokenList * simple_command(TokenList * tokenlist)
	/* cmd_prefix cmd_word cmd_suffix
	 * | cmd_prefix cmd_word
	 * | cmd_prefix
	 * | cmd_name cmd_suffix
	 * | cmd_name */
	/* cmd_prefix [cmd_word [cmd_suffix]]
	 * | cmd_name [cmd_suffix] */
{
#ifdef DEBUG
	debug_rule("simple_command", tokenlist);
#endif
	if(check_set(tokenlist, CS_CMD_PREFIX))
	{
		tokenlist = cmd_prefix(tokenlist);
		if(check_set(tokenlist, CS_CMD_WORD))
		{
			tokenlist = cmd_word(tokenlist);
			if(check_set(tokenlist, CS_CMD_SUFFIX))
				tokenlist = cmd_suffix(tokenlist);
		}
		return tokenlist;
	}
	tokenlist = cmd_name(tokenlist);
	if(check_set(tokenlist, CS_CMD_SUFFIX))
		tokenlist = cmd_suffix(tokenlist);
	return tokenlist;
}


/* cmd_name */
static TokenList * cmd_name(TokenList * tokenlist)
	/* WORD (rule 7a) */
{
#ifdef DEBUG
	debug_rule("cmd_name", tokenlist);
#endif
	return tokenlist_next(tokenlist);
}


/* cmd_word */
static TokenList * cmd_word(TokenList * tokenlist)
	/* WORD (rule 7b) */
{
#ifdef DEBUG
	debug_rule("cmd_word", tokenlist);
#endif
	return tokenlist_next(tokenlist);
}


/* cmd_prefix */
static TokenList * cmd_prefix(TokenList * tokenlist)
	/* io_redirect
	 * | cmd_prefix io_redirect
	 * | ASSIGNMENT_WORD
	 * | cmd_prefix ASSIGNMENT_WORD */
{
#ifdef DEBUG
	debug_rule("cmd_prefix", tokenlist);
#endif
	return null_tokenlist;
}


/* cmd_suffix */
static TokenList * cmd_suffix(TokenList * tokenlist)
	/* io_redirect
	 * | cmd_suffix io_redirect
	 * | WORD
	 * | cmd_suffix WORD */
	/* io_redirect { io_redirect }
	 * | WORD { WORD } */
{
#ifdef DEBUG
	debug_rule("cmd_suffix", tokenlist);
#endif
	return null_tokenlist;
}


/* redirect_list */
static TokenList * io_redirect(TokenList * tokenlist);
static TokenList * redirect_list(TokenList * tokenlist)
	/* io_redirect
	 * | redirect_list io_redirect */
	/* io_redirect { io_redirect } */
{
#ifdef DEBUG
	debug_rule("redirect_list", tokenlist);
#endif
	tokenlist = io_redirect(tokenlist);
	while(check_set(tokenlist, CS_REDIRECT_LIST))
		tokenlist = io_redirect(tokenlist);
	return tokenlist;
}


/* io_redirect */
static TokenList * io_redirect(TokenList * tokenlist)
	/* io_file
	 * | IO_NUMBER io_file
	 * | io_here
	 * | IO_NUMBER io_here */
{
#ifdef DEBUG
	debug_rule("redirect_list", tokenlist);
#endif
	return null_tokenlist;
}


/* newline_list */
static TokenList * newline_list(TokenList * tokenlist)
	/* NEWLINE { NEWLINE } */
{
#ifdef DEBUG
	debug_rule("newline_list", tokenlist);
#endif
	tokenlist = tokenlist_next(tokenlist);
	while(check(tokenlist, TC_NEWLINE))
		tokenlist = tokenlist_next(tokenlist);
	return tokenlist;
}


/* linebreak */
static TokenList * linebreak(TokenList * tokenlist)
	/* newline_list
	 * | */
{
#ifdef DEBUG
	debug_rule("newline_list", tokenlist);
#endif
	if(check_set(tokenlist, CS_NEWLINE_LIST))
		tokenlist = newline_list(tokenlist);
	return tokenlist;
}


/* separator_op */
static TokenList * separator_op(TokenList * tokenlist)
	/* '&' | ';' */
{
#ifdef DEBUG
	debug_rule("separator_op", tokenlist);
#endif
	if(check(tokenlist, TC_OP_AMPERSAND))
		return scan(tokenlist);
	return scan(tokenlist);
}


/* separator */
static TokenList * separator(TokenList * tokenlist)
	/* separator_op linebreak
	 * | newline_list) */
{
#ifdef DEBUG
	debug_rule("separator", tokenlist);
#endif
	if(check_set(tokenlist, CS_NEWLINE_LIST))
		return newline_list(tokenlist);
	if((tokenlist = separator_op(tokenlist)) == null_tokenlist)
		return null_tokenlist;
	if(!check_set(tokenlist, CS_LINEBREAK))
		return error("Expected a linebreak");
	return linebreak(tokenlist);
}


/* sequential_sep */
static TokenList * sequential_sep(TokenList * tokenlist)
	/* ";" linebreak
	 * | newline_list */
{
	if(check_word(tokenlist, ";"))
	{
		tokenlist = scan(tokenlist);
		if(!check_set(tokenlist, CS_LINEBREAK))
			return error("Expected a linebreak");
		return linebreak(tokenlist);
	}
	return newline_list(tokenlist);
}
