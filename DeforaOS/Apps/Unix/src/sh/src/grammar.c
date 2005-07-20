/* grammar.c */



#include "parser.h"
#include "grammar.h"


#ifdef DEBUG
static void grammar_debug(char const * func, Parser * p)
{
	fprintf(stderr, "%s: ", func);
	if(p->token == NULL)
	{
		fprintf(stderr, "/error/\n");
		return;
	}
	if(p->token->str != NULL)
		fprintf(stderr, "\"%s\"", p->token->str);
	else if(sTokenCode[p->token->code] != NULL)
		fprintf(stderr, "\"%s\"", sTokenCode[p->token->code]);
	fprintf(stderr, "\n");
}
#endif


/* functions */
/* complete_command */
static void list(Parser * p);
static void separator(Parser * p);
int complete_command(Parser * p)
	/* list [separator] */
{
#ifdef DEBUG
	grammar_debug("complete_command", p);
#endif
	if(!parser_check_set(p, CS_LIST))
		return 1;
	list(p);
	if(parser_test_set(p, CS_SEPARATOR))
		separator(p);
	/* FIXME: check if there are unused tokens? */
	return 0;
}


/* list */
static void and_or(Parser * p);
static void separator_op(Parser * p);
static void list(Parser * p)
	/* list separator_op and_or
	 * | and_or */
	/* and_or { separator_op and_or } */
{
#ifdef DEBUG
	grammar_debug("list", p);
#endif
	and_or(p);
	while(parser_test_set(p, CS_SEPARATOR_OP))
	{
		separator_op(p);
		if(!parser_check_set(p, CS_AND_OR))
			return;
		and_or(p);
	}
}


/* and_or */
static void pipeline(Parser * p);
static void linebreak(Parser * p);
static void and_or(Parser * p)
	/* pipeline
	 * | and_or AND_IF linebreak pipeline
	 * | and_or OR_IF linebreak pipeline */
	/* pipeline { (AND_IF | OR_IF) linebreak pipeline } */
{
#ifdef DEBUG
	grammar_debug("and_or", p);
#endif
	pipeline(p);
	while(parser_code(p) == TC_OP_AND_IF || parser_code(p) == TC_OP_OR_IF)
	{
		parser_scan(p);
		/* FIXME check set */
		if(!parser_check_set(p, CS_LINEBREAK))
		{
			parser_error(p, "expected linebreak");
			return;
		}
		linebreak(p);
		if(!parser_check_set(p, CS_PIPELINE))
		{
			parser_error(p, "expected pipeline");
			return;
		}
		pipeline(p);
	}
}


/* pipeline */
static void pipe_sequence(Parser * p);
static void pipeline(Parser * p)
	/* [Bang] pipe_sequence */
{
#ifdef DEBUG
	grammar_debug("pipeline", p);
#endif
	if(parser_code(p) != TC_RW_BANG)
		return pipe_sequence(p);
	parser_scan(p);
	if(!parser_check_set(p, CS_PIPE_SEQUENCE))
		return;
	pipe_sequence(p);
}


/* pipe_sequence */
static void command(Parser * p);
static void pipe_sequence(Parser * p)
	/* command
	 * pipe_sequence '|' linebreak command */
	/* command { '|' linebreak command } */
{
#ifdef DEBUG
	grammar_debug("pipe_sequence", p);
#endif
	command(p);
	while(parser_code(p) == TC_OP_BAR)
	{
		parser_scan(p);
		if(!parser_check_set(p, CS_LINEBREAK))
		{
			parser_error(p, "Expected linebreak");
			return;
		}
		linebreak(p);
		command(p);
	}
}


/* command */
static void compound_command(Parser * p);
static void redirect_list(Parser * p);
static void function_definition(Parser * p);
static void simple_command(Parser * p);
static void command(Parser * p)
	/* simple_command
	 * | compound_command
	 * | compound_command redirect_list
	 * | function_definition */
	/* simple_command
	 * | compound_command [redirect_list]
	 * | function_definition */
{
#ifdef DEBUG
	grammar_debug("command", p);
#endif
	parser_rule1(p);
	if(parser_test_set(p, CS_SIMPLE_COMMAND))
		return simple_command(p);
	if(parser_test_set(p, CS_COMPOUND_COMMAND))
	{
		compound_command(p);
		if(parser_check_set(p, CS_REDIRECT_LIST))
			return redirect_list(p);
		return;
	}
	if(parser_check_set(p, CS_FUNCTION_DEFINITION))
		return function_definition(p);
	parser_error(p, "expected a simple or compound command, or a function");
}


/* compound_command */
static void brace_group(Parser * p);
static void subshell(Parser * p);
static void for_clause(Parser * p);
static void case_clause(Parser * p);
static void if_clause(Parser * p);
static void while_clause(Parser * p);
static void until_clause(Parser * p);
static void compound_command(Parser * p)
	/* brace_group
	 * | subshell
	 * | for_clause
	 * | case_clause
	 * | if_clause
	 * | while_clause
	 * | until_clause */
{
#ifdef DEBUG
	grammar_debug("compound_command", p);
#endif
	switch(parser_code(p))
	{
		case TC_RW_LBRACE:
			return brace_group(p);
		case TC_RW_FOR:
			return for_clause(p);
		case TC_RW_CASE:
			return case_clause(p);
		case TC_RW_IF:
			return if_clause(p);
		case TC_RW_WHILE:
			return while_clause(p);
		case TC_RW_UNTIL:
			return until_clause(p);
		default:
			break;
	}
	if(parser_check_word(p, "("))
		subshell(p);
}


/* subshell */
static void compound_list(Parser * p);
static void subshell(Parser * p)
	/* '(' compound_list ')' */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
	parser_check_word(p, ")");
}


/* compound_list */
static void newline_list(Parser * p);
static void term(Parser * p);
static void compound_list(Parser * p)
	/* term
	 * | newline_list term
	 * | term separator
	 * | newline_list term separator */
	/* [newline_list] term [separator] */
{
	/* FIXME optimize */
	if(parser_test_set(p, CS_NEWLINE_LIST))
		newline_list(p);
	if(!parser_check_set(p, CS_TERM))
		return;
	term(p);
	if(parser_test_set(p, CS_SEPARATOR))
		separator(p);
}

/* term */
static void term(Parser * p)
	/* term separator and_or
	 * | and_or */
	/* and_or { separator and_or } */
{
	/* FIXME */
	and_or(p);
	while(parser_test_set(p, CS_SEPARATOR))
	{
		separator(p);
		if(!parser_check_set(p, CS_AND_OR))
			return;
		and_or(p);
	}
}

/* for_clause */
static void do_group(Parser * p);
static void name(Parser * p);
static void in(Parser * p);
static void wordlist(Parser * p);
static void sequential_sep(Parser * p);
static void for_clause(Parser * p)
	/* For name linebreak do_group
	 * | For name linebreak in sequential_sep do_group
	 * | For name linebreak in wordlist sequential_sep do_group */
	/* For name linebreak [in [wordlist] sequential_sep] do_group */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_NAME))
		return;
	name(p);
	if(!parser_check_set(p, CS_LINEBREAK))
		return;
	linebreak(p);
	if(parser_test_set(p, CS_IN))
	{
		in(p);
		if(parser_test_set(p, CS_WORDLIST))
			wordlist(p);
		if(!parser_check_set(p, CS_SEQUENTIAL_SEP))
			return;
		sequential_sep(p);
	}
	do_group(p);
}


/* name */
static void name(Parser * p)
	/* NAME (rule 5) */
{
	/* FIXME */
	parser_scan(p);
}


/* in */
static void in(Parser * p)
	/* In (rule 6) */
{
	/* FIXME */
	parser_scan(p);
}


/* wordlist */
static void wordlist(Parser * p)
	/* wordlist WORD
	 * | WORD */
	/* WORD { WORD } */
{
	parser_scan(p);
/*	parser_distinct(p); */
	while(parser_code(p) == TC_WORD)
	{
		parser_scan(p);
/*		parser_distinct(p); */
	}
}


/* case_clause */
static void case_clause(Parser * p)
	/* Case WORD linebreak in linebreak case_list Esac
	 * | Case WORD linebreak in linebreak case_list_ns Esac
	 * | Case WORD linebreak in linebreak Esac */
	/* Case WORD linebreak in linebreak [(case_list | case_list_ns)] Esac */
{
	parser_scan(p);
/*	parser_distinct(p); */
	if(!parser_check(p, TC_WORD))
		return;
	parser_scan(p);
	if(!parser_check_set(p, CS_LINEBREAK))
		return;
	linebreak(p);
	if(!parser_check_set(p, CS_IN))
		return;
	in(p);
	if(!parser_check_set(p, CS_LINEBREAK))
		return;
	linebreak(p);
	/* FIXME */
	parser_check(p, TC_RW_ESAC);
}


/* if_clause */
static void if_clause(Parser * p)
	/* If compound_list Then compound_list else_part Fi
	 * If compound_list Then compound_list Fi */
	/* If compound_list Then compound_list [else_part] Fi */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
/*	parser_distinct(p); */
	if(!parser_check(p, TC_RW_THEN))
		return;
	compound_list(p);
	/* FIXME */
/*	parser_distinct(p); */
	parser_check(p, TC_RW_FI);
}


/* while_clause */
static void while_clause(Parser * p)
	/* While compound_list do_group */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
	if(!parser_check_set(p, CS_DO_GROUP))
		return;
	do_group(p);
}


/* until_clause */
static void until_clause(Parser * p)
	/* Until compound_list do_group */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
	if(!parser_check_set(p, CS_DO_GROUP))
		return;
	do_group(p);
}


/* brace_group */
static void brace_group(Parser * p)
	/* Lbrace compound_list Rbrace */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
	parser_check(p, TC_RW_RBRACE);
}


/* do_group */
static void do_group(Parser * p)
	/* Do compound_list Done */
{
	parser_scan(p);
	if(!parser_check_set(p, CS_COMPOUND_LIST))
		return;
	compound_list(p);
	parser_check(p, TC_RW_DONE);
}


/* function_definition */
static void function_body(Parser * p);
static void fname(Parser * p);
static void function_definition(Parser * p)
	/* fname '(' ')' linebreak function_body */
{
#ifdef DEBUG
	grammar_debug("function_definition", p);
#endif
	fname(p);
	if(!parser_check_word(p, "(") || !parser_check_word(p, ")"))
		return;
	if(!parser_check_set(p, CS_LINEBREAK))
		return;
	linebreak(p);
	if(!parser_check_set(p, CS_FUNCTION_BODY))
		return;
	function_body(p);
}


/* function_body */
static void function_body(Parser * p)
	/* compound_command [redirect_list] (rule 9) */
{
	/* FIXME */
	compound_command(p);
	if(parser_test_set(p, CS_REDIRECT_LIST))
		redirect_list(p);
}


/* fname */
static void fname(Parser * p)
	/* NAME (rule 8) */
{
	/* FIXME */
	parser_scan(p);
}


/* simple_command */
static void cmd_prefix(Parser * p);
static void cmd_word(Parser * p);
static void cmd_suffix(Parser * p);
static void cmd_name(Parser * p);
static void simple_command(Parser * p)
	/* cmd_prefix cmd_word cmd_suffix
	 * | cmd_prefix cmd_word
	 * | cmd_prefix
	 * | cmd_name cmd_suffix
	 * | cmd_name */
	/* cmd_prefix [cmd_word [cmd_suffix]]
	 * | cmd_name [cmd_suffix] */
{
#ifdef DEBUG
	grammar_debug("simple_command", p);
#endif
	if(parser_test_set(p, CS_CMD_PREFIX))
	{
		cmd_prefix(p);
		if(parser_test_set(p, CS_CMD_WORD))
		{
			cmd_word(p);
			parser_rule1(p);
			if(parser_test_set(p, CS_CMD_SUFFIX))
				cmd_suffix(p);
		}
		return;
	}
	if(!parser_test_set(p, CS_CMD_NAME))
	{
		parser_error(p, "expected a prefix or a name");
		return;
	}
	cmd_name(p);
	parser_rule1(p);
	if(parser_test_set(p, CS_CMD_SUFFIX))
		cmd_suffix(p);
}


/* cmd_name */
static void cmd_name(Parser * p)
	/* WORD (rule 7a) */
{
#ifdef DEBUG
	grammar_debug("cmd_name", p);
#endif
	parser_rule7a(p);
	/* FIXME */
	parser_scan(p);
}


/* cmd_word */
static void cmd_word(Parser * p)
	/* WORD (rule 7b) */
{
#ifdef DEBUG
	grammar_debug("cmd_word", p);
#endif
	parser_rule7b(p);
	/* FIXME */
	parser_scan(p);
}


/* cmd_prefix */
static void io_redirect(Parser * p);
static void cmd_prefix(Parser * p)
	/* io_redirect
	 * | cmd_prefix io_redirect
	 * | ASSIGNMENT_WORD
	 * | cmd_prefix ASSIGNMENT_WORD */
	/* [cmd_prefix] (ASSIGNMENT_WORD | io_redirect) */
{
#ifdef DEBUG
	grammar_debug("cmd_prefix", p);
#endif
	if(parser_test_set(p, CS_CMD_PREFIX))
		cmd_prefix(p);
	if(parser_code(p) == TC_ASSIGNMENT_WORD)
		return parser_scan(p);
	if(!parser_test_set(p, CS_IO_REDIRECT))
		return;
	io_redirect(p);
}


/* cmd_suffix */
static void cmd_suffix(Parser * p)
	/* io_redirect
	 * | cmd_suffix io_redirect
	 * | WORD
	 * | cmd_suffix WORD */
	/* io_redirect | WORD { io_redirect | WORD } */
{
	for(;; parser_scan(p))
	{
#ifdef DEBUG
		grammar_debug("cmd_suffix", p);
#endif
		if(parser_code(p) == TC_WORD)
			;
		else if(parser_test_set(p, CS_IO_REDIRECT))
			io_redirect(p);
		else
			break;
		parser_rule1(p);
	}
}


/* redirect_list */
static void redirect_list(Parser * p)
	/* io_redirect
	 * | redirect_list io_redirect */
	/* io_redirect { io_redirect } */
{
	io_redirect(p);
	while(parser_test_set(p, CS_REDIRECT_LIST))
		io_redirect(p);
}


/* io_redirect */
static void io_file(Parser * p);
static void io_here(Parser * p);
static void io_redirect(Parser * p)
	/* io_file
	 * | IO_NUMBER io_file
	 * | io_here
	 * | IO_NUMBER io_here */
	/* [IO_NUMBER] (io_file | io_here) */
{
	if(parser_code(p) == TC_IO_NUMBER)
		parser_scan(p);
	if(parser_test_set(p, CS_IO_FILE))
		return io_file(p);
	if(!parser_check_set(p, CS_IO_HERE))
		return;
	io_here(p);
}


/* io_file */
static void filename(Parser * p);
static void io_file(Parser * p)
	/* '<' filename
	 * | LESSAND filename
	 * | '>' filename
	 * | GREATAND filename */
{
	/* FIXME */
	parser_scan(p);
	filename(p);
}


/* filename */
static void filename(Parser * p)
	/* WORD (rule 2) */
{
#ifdef DEBUG
	grammar_debug("filename", p);
#endif
	/* FIXME */
	parser_scan(p);
}


/* io_here */
static void here_end(Parser * p);
static void io_here(Parser * p)
	/* DLESS here_end
	 * | DLESSDASH here_end */
	/* (DLESS | DLESSDASH) here_end */
{
	/* FIXME */
	parser_scan(p);
	here_end(p);
}


/* here_end */
static void here_end(Parser * p)
	/* WORD (rule 3) */
{
	parser_scan(p);
}


/* newline_list */
static void newline_list(Parser * p)
	/* NEWLINE { NEWLINE } */
{
	if(!parser_check(p, TC_NEWLINE))
		return;
	while(parser_code(p) == TC_NEWLINE)
		parser_scan(p);
}


/* linebreak */
static void linebreak(Parser * p)
	/* newline_list
	 * | */
{
	if(parser_test_set(p, CS_NEWLINE_LIST))
		newline_list(p);
}


/* separator_op */
static void separator_op(Parser * p)
	/* '&' | ';' */
{
	if(parser_code(p) == TC_OP_AMPERSAND
			|| parser_check(p, TC_OP_SEMICOLON))
		parser_scan(p);
}


/* separator */
static void separator(Parser * p)
	/* separator_op linebreak
	 * | newline_list */
{
	if(parser_test_set(p, CS_NEWLINE_LIST))
		return newline_list(p);
	if(!parser_check_set(p, CS_SEPARATOR_OP))
		return;
	separator_op(p);
	if(!parser_check_set(p, CS_LINEBREAK))
		return;
	linebreak(p);
}


/* sequential_sep */
static void sequential_sep(Parser * p)
	/* ";" linebreak
	 * | newline_list */
{
	if(parser_test_word(p, ";"))
	{
		parser_scan(p);
		if(!parser_check_set(p, CS_LINEBREAK))
			return;
		return linebreak(p);
	}
	newline_list(p);
}
