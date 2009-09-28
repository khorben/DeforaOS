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



#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>
#include "token.h"
#include "scanner.h"
#include "builtin.h"
#include "job.h"
#include "parser.h"


/* Parser */
/* private */
/* types */
typedef struct _Parser
{
	Prefs * prefs;
	int argc;
	char ** argv;
	Scanner scanner;
	Token ** tokens;
	unsigned int tokens_cnt;
	Token * token;
} Parser;

/* public */
/* functions */
/* parser */
static void parser_free(Parser * parser);
static void parser_scan(Parser * parser);
static int parser_exec(Parser * parser, unsigned int * pos, int skip);
static int complete_command(Parser * parser);

int parser(Prefs * prefs, char const * string, FILE * fp, int argc,
		char * argv[])
{
	Parser parser;
	unsigned int pos;

	parser.prefs = prefs;
	parser.argc = argc;
	parser.argv = argv;
	scanner_init(&parser.scanner, prefs, fp, string);
	parser.tokens = NULL;
	parser.tokens_cnt = 0;
	for(parser_scan(&parser); parser.token != NULL;)
	{
		if(parser.token->code == TC_EOI)
		{
			token_delete(parser.token);
			break;
		}
		complete_command(&parser);
		if(parser.token != NULL)
			for(pos = 0; pos < parser.tokens_cnt; pos++)
				parser_exec(&parser, &pos, 0);
		parser_free(&parser);
		if(parser.token == NULL)
			parser_scan(&parser);
	}
	free(parser.tokens);
	return 0;
}


static void parser_free(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fputs("parser_free()\n", stderr);
#endif
	if(parser->token == NULL && parser->tokens_cnt > 0)
		/* FIXME ugly workaround for newlines */
		parser->token = parser->tokens[parser->tokens_cnt-1];
	for(i = 0; i < parser->tokens_cnt-1; i++)
		token_delete(parser->tokens[i]);
	free(parser->tokens);
	parser->tokens = NULL;
	parser->tokens_cnt = 0;
	if(parser->token == NULL)
		return;
	if(parser->token->code == TC_EOI)
		return;
	token_delete(parser->token);
	parser->token = NULL;
}


static void parser_error(Parser * parser, char * format, ...)
{
	va_list vl;

	fputs("sh: ", stderr);
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
	fputc('\n', stderr);
	parser->token = NULL;
}


static void parser_scan(Parser * parser)
{
	Token ** p;

	if((parser->token = scanner_next(&parser->scanner)) == NULL)
		return;
	if((p = realloc(parser->tokens, (parser->tokens_cnt + 1)
					* sizeof(*p))) == NULL)
	{
		sh_error("malloc", 0);
		return;
	}
	parser->tokens = p;
	parser->tokens[parser->tokens_cnt++] = parser->token;
}


static int parser_check(Parser * parser, TokenCode code)
{
	if(parser->token == NULL || parser->token->code != code)
	{
		parser_error(parser, "%s%s%s", "\"", sTokenCode[code],
				"\" expected");
		return 0;
	}
	parser_scan(parser);
	return 1;
}


static int parser_check_word(Parser * parser, char const * word)
{
	if(parser->token == NULL || parser->token->code != TC_TOKEN
			|| parser->token->string == NULL
			|| strcmp(parser->token->string, word) != 0)
	{
		parser_error(parser, "%s%s%s", "\"", word, "\" expected");
		return 0;
	}
	parser_scan(parser);
	return 1;
}


/* parser_exec */
static int _exec_cmd(Parser * parser, unsigned int * pos, int skip);
static int _exec_for(Parser * parser, unsigned int * pos, int skip);
static int _exec_if(Parser * parser, unsigned int * pos, int skip);
static int _exec_case(Parser * parser, unsigned int * pos, int skip);
static int _exec_until(Parser * parser, unsigned int * pos, int skip);
static int _exec_while(Parser * parser, unsigned int * pos, int skip);

static int parser_exec(Parser * parser, unsigned int * pos, int skip)
{
	int ret = skip;
	int skiplocal = skip;

#ifdef DEBUG
	fprintf(stderr, "%s%u%s%s", "parser_exec(", *pos, ")",
			skip ? " skip\n" : "\n");
#endif
	while(*pos < parser->tokens_cnt)
		switch(parser->tokens[*pos]->code)
		{
			case TC_OP_AND_IF:
				skiplocal = skip || ret != 0;
				(*pos)++;
				break;
			case TC_OP_OR_IF:
				skiplocal = skip || ret == 0;
				(*pos)++;
				break;
			case TC_ASSIGNMENT_WORD:
			case TC_IO_NUMBER:
			case TC_OP_DLESS:
			case TC_OP_DGREAT:
			case TC_OP_LESSAND:
			case TC_OP_GREATAND:
			case TC_OP_LESSGREAT:
			case TC_OP_CLOBBER:
			case TC_OP_LESS:
			case TC_OP_GREAT:
			case TC_WORD:
				ret = _exec_cmd(parser, pos, skiplocal);
				break;
			case TC_RW_IF:
				ret = _exec_if(parser, pos, skiplocal);
				break;
			case TC_RW_CASE:
				ret = _exec_case(parser, pos, skiplocal);
				break;
			case TC_RW_WHILE:
				ret = _exec_while(parser, pos, skiplocal);
				break;
			case TC_RW_UNTIL:
				ret = _exec_until(parser, pos, skiplocal);
				break;
			case TC_RW_FOR:
				ret = _exec_for(parser, pos, skiplocal);
				break;
			case TC_EOI:
			case TC_NEWLINE:
			case TC_OP_SEMICOLON:
				if(skiplocal != skip)
					skiplocal = 0;
				(*pos)++;
				break;
			default:
#ifdef DEBUG
				fprintf(stderr, "%s%u%s%s", "parser_exec(",
						*pos, ")",
						skip ? " skip\n" : "\n");
#endif
				assert(0);
				return ret;
		}
	return ret;
}


/* exec_cmd */
static int _exec_cmd_env(char * envp[]);
static int _exec_cmd_builtin(int argc, char ** argv, uint8_t * bg_error);
static int _exec_cmd_child(int argc, char ** argv, uint8_t * bg_error,
		char * inf, char * outf); /* XXX group args in a struct? */

static int _exec_cmd(Parser * parser, unsigned int * pos, int skip)
{
	int ret = 1;
	char ** argv = NULL;
	unsigned int argv_cnt = 0;
	char ** envp = NULL;
	unsigned int envp_cnt = 0;
	char ** p;
	uint8_t bg_error = 0;
	char * inf = NULL;
	char * outf = NULL;

	if(!skip && (envp = sh_export()) != NULL)
		for(envp_cnt = 0; envp[envp_cnt] != NULL; envp_cnt++);
	for(; *pos < parser->tokens_cnt; (*pos)++)
	{
		switch(parser->tokens[*pos]->code)
		{
			case TC_ASSIGNMENT_WORD:
				if(skip)
					break;
				if((p = realloc(envp, sizeof(*p)*(envp_cnt+2)))
						== NULL)
					/* FIXME should not exit */
					exit(sh_error("malloc", 125));
				envp = p;
				envp[envp_cnt++] = parser->tokens[*pos]->string;
				break;
			case TC_IO_NUMBER:
			case TC_OP_BAR:
			case TC_OP_CLOBBER:
			case TC_OP_GREATAND:
			case TC_OP_LESSAND:
			case TC_OP_LESSGREAT:
				/* FIXME implement */
				break;
			case TC_OP_GREAT:
				outf = parser->tokens[++(*pos)]->string;
				break;
			case TC_OP_LESS:
				inf = parser->tokens[++(*pos)]->string;
				break;
			case TC_WORD:
				if(skip)
					break;
				if((p = realloc(argv, sizeof(*p)*(argv_cnt+2)))
						== NULL)
					/* FIXME should not exit */
					exit(sh_error("malloc", 125));
				argv = p;
				argv[argv_cnt++] = parser->tokens[*pos]->string;
				break;
			case TC_OP_AMPERSAND:
				bg_error = 1;
			default:
				ret = -1;
				break;
		}
		if(ret == -1)
			break;
	}
	if(skip)
		return skip;
	if(envp != NULL)
		envp[envp_cnt] = NULL;
	/* FIXME assignment words should affect only a child...? */
	ret = _exec_cmd_env(envp);
	free(envp);
	if(argv != NULL && ret == 0)
	{
		argv[argv_cnt] = NULL;
		/* FIXME look for builtins utilities (should be none) */
		/* FIXME look for functions */
		if(_exec_cmd_builtin(argv_cnt, argv, &bg_error) != 0)
			if(_exec_cmd_child(argv_cnt, argv, &bg_error, inf, outf)
					!= 0)
				ret = 1;
		if(ret == 0)
			ret = bg_error;
	}
	free(argv);
	return ret;
}

static int _exec_cmd_env(char * envp[])
	/* FIXME share code with the builtin set? */
{
	char ** e;
	char * p;
	int ret = 0;

	if(envp == NULL)
		return 0;
	for(e = envp; *e != NULL; e++)
	{
		for(p = *e; *p != '\0' && *p != '='; p++);
		if(*p == '\0')
			continue;
		*p = '\0';
		if(setenv(*e, p + 1, 1) != 0)
			ret |= sh_error("setenv", 1);
		*p = '=';
	}
	return ret;
}

static int _exec_cmd_builtin(int argc, char ** argv, uint8_t * bg_error)
	/* FIXME should handle redirection */
{
	struct
	{
		char * cmd;
		int (*func)(int, char**);
	} builtins[] =
	{
		{ "bg",     builtin_bg		},
		{ "cd",     builtin_cd		},
		{ "exit",   builtin_exit	},
		{ "export", builtin_export	},
		{ "fg",     builtin_fg		},
		{ "jobs",   builtin_jobs	},
		{ "read",   builtin_read	},
		{ "set",    builtin_set		},
		{ "umask",  builtin_umask	},
		{ "unset",  builtin_unset	},
		{ NULL,	    NULL		}
	};
	unsigned int i;

	for(i = 0; builtins[i].cmd != NULL; i++)
		if(strcmp(builtins[i].cmd, argv[0]) == 0)
		{
			/* FIXME if(*bg_error != 0) jobs_new(); */
			*bg_error = builtins[i].func(argc, argv);
			return 0;
		}
	return -1;
}

static int _exec_cmd_child(int argc, char ** argv, uint8_t * bg_error,
		char * inf, char * outf)
{
	pid_t pid;
	int infd = -1;
	int outfd = -1;

	assert(argv[argc] == NULL);
	if(inf != NULL && (infd = open(inf, O_RDONLY)) < 0)
		return sh_error(inf, 1);
	if(outf != NULL && (outfd = open(outf, O_WRONLY | O_CREAT | O_TRUNC,
					0666)) < 0)
		return sh_error(outf, 1);
	if((pid = fork()) == -1)
	{
		if(infd != -1)
			close(infd);
		if(outfd != -1)
			close(outfd);
		return sh_error("fork", 1);
	}
	if(pid != 0)
	{
		if(infd != -1)
			close(infd);
		if(outfd != -1)
			close(outfd);
		if(*bg_error != 0)
			*bg_error = job_add(argv[0], pid, JS_RUNNING);
		else
			*bg_error = job_add(argv[0], pid, JS_WAIT);
		return 0;
	}
	if(infd != -1 && dup2(infd, 0) == -1)
		_exit(sh_error(inf, -1));
	if(outfd != -1 && dup2(outfd, 1) == -1)
		_exit(sh_error(outf, -1));
	execvp(argv[0], argv);
	_exit(sh_error(argv[0], -1));
	return 1;
}

static int _exec_for(Parser * parser, unsigned int * pos, int skip)
{
	Token * name;
	unsigned int count = 0;
	unsigned int i;
	unsigned int p;

	name = parser->tokens[++(*pos)];
	(*pos)++;
	if(parser->tokens[*pos]->code == TC_RW_IN)
		for((*pos)++; parser->tokens[*pos]->code == TC_WORD; (*pos)++)
			count++;
	for((*pos)++; parser->tokens[*pos]->code != TC_RW_DO; (*pos)++);
	p = ++(*pos);
	for(i = 0; i < count; i++)
	{
		if(setenv(name->string, parser->tokens[p-2-count+i]->string, 1)
				!= 0)
			skip = sh_error("setenv", 1);
		*pos = p;
		parser_exec(parser, pos, skip);
		if(skip != 0)
			break;
	}
	assert(parser->tokens[*pos]->code == TC_RW_DONE);
	(*pos)++;
	return skip;
}

static int _exec_if(Parser * parser, unsigned int * pos, int skip)
{
	int execd = 0;

	while(*pos < parser->tokens_cnt)
	{
		switch(parser->tokens[*pos]->code)
		{
			case TC_RW_IF:
			case TC_RW_ELIF:
				(*pos)++;
				skip = parser_exec(parser, pos, skip
						&& execd != 0);
				(*pos)--;
				continue;
			case TC_RW_ELSE:
				if(parser->tokens[(*pos)+1]->code == TC_RW_IF)
					break;
				skip = skip && execd != 0;
			case TC_RW_THEN:
				(*pos)++;
				parser_exec(parser, pos, skip);
				if(skip == 0)
					execd = 1;
				(*pos)--;
				continue;
			case TC_RW_FI:
				return 0;
			default:
				assert(0);
				break;
		}
		(*pos)++;
	}
	return skip;
}

static int _exec_case(Parser * parser, unsigned int * pos, int skip)
{
	/* FIXME actually implement */
	for((*pos)++; parser->tokens[(*pos)++]->code != TC_RW_ESAC;);
	return skip;
}

static int _exec_until(Parser * parser, unsigned int * pos, int skip)
{
	unsigned int test;

	for(test = ++(*pos);; *pos = test)
	{
		skip = parser_exec(parser, pos, skip) == 0 || skip;
		(*pos)++;
		parser_exec(parser, pos, skip);
		if(skip != 0)
			break;
	}
	assert(parser->tokens[*pos]->code == TC_RW_DONE);
	(*pos)++;
	return skip;
}

static int _exec_while(Parser * parser, unsigned int * pos, int skip)
{
	unsigned int test;

	for(test = ++(*pos);; *pos = test)
	{
		skip = parser_exec(parser, pos, skip) != 0 || skip;
		(*pos)++;
		parser_exec(parser, pos, skip);
		if(skip != 0)
			break;
	}
	assert(parser->tokens[*pos]->code == TC_RW_DONE);
	(*pos)++;
	return skip;
}


/* rules */
static void parser_rule1(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fputs("rule 1\n", stderr);
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	for(i = TC_RW_IF; i <= TC_RW_IN; i++)
		if(strcmp(parser->token->string, sTokenCode[i]) == 0)
		{
			parser->token->code = i;
			return;
		}
	parser->token->code = TC_WORD;
}


static void parser_rule4(Parser * parser)
{
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	if(strcmp(parser->token->string, sTokenCode[TC_RW_ESAC]) == 0)
		parser->token->code = TC_RW_ESAC;
	else
		parser->token->code = TC_WORD;
}


static void parser_rule5(Parser * parser)
{
	unsigned int i;
	int c;

#ifdef DEBUG
	fputs("rule 5\n", stderr);
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	c = parser->token->string[0];
	if(isdigit(c))
	{
		parser->token->code = TC_WORD;
		return;
	}
	for(i = 0; (c = parser->token->string[i]) != '\0'; i++)
		if(!isalnum(c) && c != '_')
		{
			parser->token->code = TC_WORD;
			return;
		}
	parser->token->code = TC_NAME;
}


static void parser_rule6_for(Parser * parser)
{
#ifdef DEBUG
	fputs("rule 6 (for)\n", stderr);
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	if(strcmp(sTokenCode[TC_RW_DO], parser->token->string) == 0)
		parser->token->code = TC_RW_DO;
	if(strcmp(sTokenCode[TC_RW_IN], parser->token->string) == 0)
		parser->token->code = TC_RW_IN;
	else
		parser->token->code = TC_WORD;
}


static void parser_rule7b(Parser * parser);
static void parser_rule7a(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fputs("rule 7a\n", stderr);
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	for(i = 0; parser->token->string[i] != '\0'; i++)
		if(parser->token->string[i] == '=')
		{
			parser_rule7b(parser);
			return;
		}
	parser_rule1(parser);
}


static void parser_rule7b(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fputs("rule 7b\n", stderr);
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	switch(parser->token->string[0])
	{
		case '=':
		case '\0':
			parser->token->code = TC_WORD;
			return;
		default:
			break;
	}
	for(i = 1; parser->token->string[i] != '\0'; i++)
		if(parser->token->string[i] == '=')
		{
			parser->token->code = TC_ASSIGNMENT_WORD;
			return;
		}
	parser->token->code = TC_WORD;
}


static void parser_rule8(Parser * parser)
{
#ifdef DEBUG
	fputs("rule 8\n", stderr);
#endif
	if(parser->token == NULL)
		return;
	parser_rule1(parser);
	if(parser->token->code != TC_WORD)
		return;
	parser_rule5(parser);
	if(parser->token->code != TC_NAME)
		return;
	parser_rule7a(parser);
}


/* complete_command */
static void list(Parser * p);
static void separator(Parser * p);
static int complete_command(Parser * p)
	/* list [separator] */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "complete_command()\n");
#endif
	list(p);
	if(p->token != NULL && token_in_set(p->token, TS_SEPARATOR))
		separator(p);
	return 0;
}


/* list */
static void and_or(Parser * p);
static void separator_op(Parser * p);
static void list(Parser * p)
	/* and_or { separator_op and_or } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "list()\n");
#endif
	and_or(p);
	while(p->token != NULL && token_in_set(p->token, TS_SEPARATOR_OP))
	{
		separator_op(p);
		and_or(p);
	}
}


/* and_or */
static void pipeline(Parser * p);
static void linebreak(Parser * p);
static void and_or(Parser * p)
	/* pipeline { (AND_IF | OR_IF) linebreak pipeline } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "and_or()\n");
#endif
	pipeline(p);
	while(p->token != NULL)
	{
		if(p->token->code != TC_OP_AND_IF
				&& p->token->code != TC_OP_OR_IF)
			return;
		parser_scan(p);
		linebreak(p);
		pipeline(p);
	}
}


/* pipeline */
static void pipe_sequence(Parser * p);
static void pipeline(Parser * p)
	/* [Bang] pipe_sequence */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "pipeline()\n");
#endif
	if(p->token != NULL && p->token->code == TC_RW_BANG)
		parser_scan(p);
	pipe_sequence(p);
}


/* pipe_sequence */
static void command(Parser * p);
static void pipe_sequence(Parser * p)
	/* command { '|' linebreak command } */
{
	command(p);
	while(p->token != NULL && p->token->code == TC_OP_BAR)
	{
		parser_scan(p);
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
	 * | compound_command [redirect_list]
	 * | function_definition */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "command()\n");
#endif
	if(p->token == NULL)
		return;
	parser_rule7a(p);
	if(token_in_set(p->token, TS_COMPOUND_COMMAND))
	{
		compound_command(p);
		if(p->token != NULL && token_in_set(p->token, TS_REDIRECT_LIST))
			redirect_list(p);
	}
	else if(token_in_set(p->token, TS_FUNCTION_DEFINITION))
		function_definition(p);
	else
		simple_command(p);
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
	fprintf(stderr, "%s", "compound_command()\n");
#endif
	if(p->token == NULL)
		return;
	switch(p->token->code)
	{
		case TC_RW_LBRACE:
			brace_group(p);
			return;
		case TC_RW_FOR:
			for_clause(p);
			return;
		case TC_RW_CASE:
			case_clause(p);
			return;
		case TC_RW_IF:
			if_clause(p);
			return;
		case TC_RW_WHILE:
			while_clause(p);
			return;
		case TC_RW_UNTIL:
			until_clause(p);
			return;
		default:
			break;
	}
	if(p->token != NULL && p->token->code == TC_TOKEN
			&& p->token->string != NULL
			&& strcmp("(", p->token->string) == 0)
		subshell(p);
}


/* subshell */
static void compound_list(Parser * p);
static void subshell(Parser * p)
	/* '(' compound_list ')' */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "subshell()\n");
#endif
	parser_scan(p);
	compound_list(p);
	parser_check_word(p, ")");
}


/* compound_list */
static void newline_list(Parser * p);
static void term(Parser * p);
static void compound_list(Parser * p)
	/* [newline_list] term [separator] */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "compound_list()\n");
#endif
	if(p->token != NULL && token_in_set(p->token, TS_NEWLINE_LIST))
		newline_list(p);
	term(p);
	if(p->token != NULL && token_in_set(p->token, TS_SEPARATOR))
		separator(p);
}


/* term */
static void term(Parser * p)
	/* and_or { separator and_or } */
{
	and_or(p);
	while(p->token != NULL && token_in_set(p->token, TS_SEPARATOR))
	{
		separator(p);
		parser_rule1(p);
		if(p->token != NULL && !token_in_set(p->token, TS_AND_OR))
			break;
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
	/* For name linebreak [in [wordlist] sequential_sep] do_group */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "for_clause()\n");
#endif
	parser_scan(p);
	name(p);
	linebreak(p);
	parser_rule6_for(p);
	if(p->token != NULL && p->token->code == TC_RW_IN)
	{
		parser_scan(p);
		parser_rule1(p);
		if(p->token != NULL && token_in_set(p->token, TS_WORDLIST))
			wordlist(p);
		sequential_sep(p);
	}
	do_group(p);
}


/* name */
static void name(Parser * p)
	/* NAME  (rule 5) */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "name()\n");
#endif
	parser_rule5(p);
	parser_check(p, TC_NAME);
}


/* in */
static void in(Parser * p)
	/* In  (rule 6) */
{
	parser_scan(p);
}


/* wordlist */
static void wordlist(Parser * p)
	/* WORD { WORD } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "wordlist()\n");
#endif
	parser_scan(p);
	for(parser_rule1(p); p->token != NULL && p->token->code == TC_WORD;
			parser_rule1(p))
		parser_scan(p);
}


/* case_clause */
static void case_list(Parser * p);
static void case_clause(Parser * p)
	/* Case WORD linebreak in linebreak case_list Esac */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "case_clause()\n");
#endif
	parser_scan(p);
	if(!parser_check(p, TC_TOKEN))
		return;
	linebreak(p);
	in(p);
	linebreak(p);
	parser_rule4(p);
	case_list(p);
	parser_check(p, TC_RW_ESAC);
}


/* case_list */
static void case_item(Parser * p);
static void case_list(Parser * p)
	/* { case_item } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "case_list()\n");
#endif
	while(p->token != NULL && p->token->code != TC_RW_ESAC)
		case_item(p);
}


/* case_item */
static void pattern(Parser * p);
static void case_item(Parser * p)
	/* ["("] pattern ")" (linebreak | compound_list) [DSEMI] linebreak */
{
	if(p->token != NULL && p->token->code == TC_TOKEN
			&& p->token->string != NULL
			&& strcmp(p->token->string, "(") == 0)
		parser_scan(p); /* FIXME also check for the code? */
	pattern(p);
	parser_check_word(p, ")"); /* FIXME not detected yet */
	linebreak(p);
	compound_list(p);
	if(p->token != NULL && p->token->code == TC_OP_DSEMI)
		parser_scan(p);
	linebreak(p);
}


/* pattern */
static void pattern(Parser * p)
	/* WORD  (rule 4) { '|' WORD } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "pattern()\n");
#endif
	parser_scan(p);
	while(p->token != NULL && p->token->code == TC_OP_BAR)
	{
		parser_scan(p); /* '|' */
		parser_scan(p); /* WORD */
	}
}


/* if_clause */
static void else_part(Parser * p);
static void if_clause(Parser * p)
	/* If compound_list Then compound_list [else_part] Fi */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "if_clause()\n");
#endif
	parser_scan(p);
	compound_list(p);
	if(!parser_check(p, TC_RW_THEN))
		return;
	compound_list(p);
	if(p->token != NULL && token_in_set(p->token, TS_ELSE_PART))
		else_part(p);
	parser_check(p, TC_RW_FI);
}


/* else_part */
static void else_part(Parser * p)
	/* Elif compound_list Then else_part
	 * | Else compound_list */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "else_part()\n");
#endif
	if(p->token == NULL)
		return;
	if(p->token->code == TC_RW_ELIF)
	{
		parser_scan(p);
		compound_list(p);
		parser_check(p, TC_RW_THEN);
		else_part(p);
	}
	else if(p->token->code == TC_RW_ELSE)
	{
		parser_scan(p);
		compound_list(p);
	}
}


/* while_clause */
static void while_clause(Parser * p)
	/* While compound_list do_group */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "while_clause()\n");
#endif
	parser_scan(p);
	compound_list(p);
	do_group(p);
}


/* until_clause */
static void until_clause(Parser * p)
	/* Until compound_list do_group */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "until_clause()\n");
#endif
	parser_scan(p);
	compound_list(p);
	do_group(p);
}


/* function_definition */
static void fname(Parser * p);
static void function_body(Parser * p);
static void function_definition(Parser * p)
	/* fname "(" ")" linebreak function_body */
{
	fname(p);
	parser_scan(p);
	parser_check_word(p, "(");
	parser_check_word(p, ")");
	linebreak(p);
	function_body(p);
}


/* function_body */
static void io_redirect(Parser * p);
static void function_body(Parser * p)
	/* compound_command [redirect_list]  (rule 9) */
{
	compound_command(p);
	if(p->token != NULL && token_in_set(p->token, TS_IO_REDIRECT))
		io_redirect(p);
}


/* fname */
static void fname(Parser * p)
	/* NAME  (rule 8) */
{
	parser_rule8(p); /* FIXME untested */
	parser_scan(p);
}


/* brace_group */
static void brace_group(Parser * p)
	/* Lbrace compound_list Rbrace */
{
	parser_scan(p);
	compound_list(p);
	parser_check(p, TC_RW_RBRACE);
}


/* do_group */
static void do_group(Parser * p)
	/* Do compound_list Done */
{
	parser_rule1(p);
	parser_check(p, TC_RW_DO);
	compound_list(p);
	parser_check(p, TC_RW_DONE);
}


/* simple_command */
static void cmd_prefix(Parser * p);
static void cmd_word(Parser * p);
static void cmd_suffix(Parser * p);
static void cmd_name(Parser * p);

static void simple_command(Parser * p)
	/* cmd_prefix [cmd_word [cmd_suffix]]
	 * | cmd_name [cmd_suffix] */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "simple_command()\n");
#endif
	if(p->token == NULL)
		return;
	if(token_in_set(p->token, TS_CMD_PREFIX))
	{
		cmd_prefix(p);
		if(p->token == NULL || !token_in_set(p->token, TS_CMD_WORD))
			return;
		cmd_word(p);
	}
	else if(!token_in_set(p->token, TS_CMD_NAME))
		/* FIXME parser_error(p, "%s", "prefix or name expected"); */
		return;
	else
		cmd_name(p);
	if(p->token == NULL)
		return;
	parser_rule1(p);
	if(token_in_set(p->token, TS_CMD_SUFFIX))
		cmd_suffix(p);
}


/* cmd_name */
static void cmd_name(Parser * p)
	/* WORD  (rule 7a) */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "cmd_name()\n");
#endif
	parser_scan(p);
}


/* cmd_word */
static void cmd_word(Parser * p)
	/* WORD  (rule 7b) */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "cmd_word()\n");
#endif
	parser_scan(p);
}


/* cmd_prefix */
static void cmd_prefix(Parser * p)
	/* (ASSIGNMENT_WORD | io_redirect) { ASSIGNMENT_WORD | io_redirect } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "cmd_prefix()\n");
#endif
	while(p->token != NULL)
	{
		if(p->token->code == TC_ASSIGNMENT_WORD)
			parser_scan(p);
		else if(token_in_set(p->token, TS_IO_REDIRECT))
			io_redirect(p);
		else
			return;
		parser_rule7b(p);
	}
}


/* cmd_suffix */
static void cmd_suffix(Parser * p)
	/* { WORD | io_redirect } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "cmd_suffix()\n");
#endif
	for(; p->token != NULL; parser_rule1(p))
		if(token_in_set(p->token, TS_IO_REDIRECT))
			io_redirect(p);
		else if(p->token->code == TC_WORD)
			parser_scan(p);
		else
			break;
}


/* redirect_list */
static void redirect_list(Parser * p)
	/* io_redirect { io_redirect } */
{
	io_redirect(p);
	while(p->token != NULL && token_in_set(p->token, TS_IO_REDIRECT))
		io_redirect(p);
}


/* io_redirect */
static void io_file(Parser * p);
static void io_here(Parser * p);
static void io_redirect(Parser * p)
	/* [IO_NUMBER] (io_file | io_here) */
{
#ifdef DEBUG
	fputs("cmd_suffix()\n", stderr);
#endif
	if(p->token == NULL)
		return;
	if(p->token->code == TC_IO_NUMBER)
		parser_scan(p);
	if(p->token == NULL)
		return;
	if(token_in_set(p->token, TS_IO_FILE))
		io_file(p);
	else if(token_in_set(p->token, TS_IO_HERE))
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
#ifdef DEBUG
	fputs("io_file()\n", stderr);
#endif
	parser_scan(p);
	/* FIXME check if it's an appropriate token for filename */
	filename(p);
}


/* filename */
static void filename(Parser * p)
	/* WORD  (rule 2) */
{
#ifdef DEBUG
	fputs("filename()\n", stderr);
#endif
	parser_scan(p);
}


/* io_here */
static void here_end(Parser * p);
static void io_here(Parser * p)
	/* (DLESS | DLESSDASH) here_end */
{
	parser_scan(p);
	here_end(p);
}


/* here_end */
static void here_end(Parser * p)
	/* WORD  (rule 3) */
{
	/* FIXME */
	parser_scan(p);
}


/* linebreak */
static void newline_list(Parser * p);
static void linebreak(Parser * p)
	/* newline_list
	 * | */
{
#ifdef DEBUG
	fputs("linebreak()\n", stderr);
#endif
	if(p->token != NULL && token_in_set(p->token, TS_NEWLINE_LIST))
		newline_list(p);
}


/* newline_list */
static void newline_list(Parser * p)
	/* NEWLINE { NEWLINE } */
{
#ifdef DEBUG
	fputs("newline_list()\n", stderr);
#endif
	if(p->token != NULL && p->token->code != TC_NEWLINE)
	{
		parser_error(p, "%s", "newline expected");
		return;
	}
	/* FIXME
	parser_scan(p);
	while(p->token != NULL && p->token->code == TC_NEWLINE)
		parser_scan(p); */
}


/* separator_op */
static void separator_op(Parser * p)
	/* '&' | ';' */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "separator_op()\n");
#endif
	if(p->token == NULL)
		return;
	if(p->token->code == TC_OP_AMPERSAND || p->token->code
			== TC_OP_SEMICOLON)
		parser_scan(p);
	else
		parser_error(p, "%s", "\"&\" or \";\" expected");
}


/* separator */
static void separator(Parser * p)
	/* separator_op linebreak
	 * | newline_list */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "separator()\n");
#endif
	if(p->token == NULL)
		return;
	if(token_in_set(p->token, TS_NEWLINE_LIST))
	{
		newline_list(p);
		return;
	}
	if(!token_in_set(p->token, TS_SEPARATOR_OP))
	{
		parser_error(p, "%s", "separator or newline expected");
		return;
	}
	separator_op(p);
	linebreak(p);
}


/* sequential_sep */
static void sequential_sep(Parser * p)
		/* ";" linebreak
		 * | newline_list */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "sequential_sep()\n");
#endif
	if(p->token != NULL && p->token->code == TC_OP_SEMICOLON)
	{
		parser_scan(p);
		linebreak(p);
		return;
	}
	newline_list(p);
}
