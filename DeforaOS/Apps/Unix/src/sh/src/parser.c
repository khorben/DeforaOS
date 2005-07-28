/* parser.c */



#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <stdarg.h>
#include <string.h>
#include "token.h"
#include "scanner.h"
#include "builtin.h"
#include "parser.h"


/* Parser */
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

static void parser_scan(Parser * parser);
static void parser_exec(Parser * parser);
static int complete_command(Parser * parser);
int parser(Prefs * prefs, char const * string, FILE * fp, int argc,
		char * argv[])
{
	Parser parser;

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
		parser_exec(&parser);
		if(parser.token == NULL)
			parser_scan(&parser);
	}
	free(parser.tokens);
	return 0;
}


static void parser_error(Parser * parser, char * format, ...)
{
	va_list vl;

	fprintf(stderr, "%s", "sh: ");
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
		return; /* FIXME ? */
	if((p = realloc(parser->tokens, (parser->tokens_cnt+1)
					* sizeof(Token *))) == NULL)
	{
		sh_error("malloc", 0);
		return;
	}
	parser->tokens = p;
	parser->tokens[parser->tokens_cnt++] = parser->token;
}


static int _exec_cmd(Parser * parser, unsigned int * pos);
static void _exec_free(Parser * parser);
static void parser_exec(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "%s", "parser_exec()\n");
#endif
	/* FIXME */
	for(i = 0; i < parser->tokens_cnt; i++)
	{
		switch(parser->tokens[i]->code)
		{
			case TC_ASSIGNMENT_WORD:
			case TC_IO_NUMBER:
			case TC_OP_CLOBBER:
			case TC_OP_GREAT:
			case TC_OP_GREATAND:
			case TC_OP_LESS:
			case TC_OP_LESSAND:
			case TC_OP_LESSGREAT:
			case TC_WORD:
				_exec_cmd(parser, &i);
				break;
			case TC_EOI:
			case TC_NEWLINE:
				break;
			case TC_TOKEN:
			case TC_NULL:
#ifdef DEBUG
				fprintf(stderr, "%s%s%s%d%s%d%s", "sh: ",
						__FILE__, ", line ",__LINE__,
						": should not happen (",
						parser->tokens[i]->code, ")\n");
#endif
				break;
		}
	}
	_exec_free(parser);
}

static int _exec_cmd_env(char * envp[]);
static int _exec_cmd_builtin(int argc, char ** argv);
static int _exec_cmd_child(int argc, char ** argv);
static int _exec_cmd(Parser * parser, unsigned int * pos)
{
	char ** argv = NULL;
	unsigned int argv_cnt = 0;
	char ** envp = NULL;
	unsigned int envp_cnt = 0;
	char ** p;
	int ret;

	for(; *pos < parser->tokens_cnt; (*pos)++)
	{
		switch(parser->tokens[*pos]->code)
		{
			case TC_ASSIGNMENT_WORD:
				if((p = realloc(envp, sizeof(char *)
								* (envp_cnt+2)))
						== NULL)
					/* FIXME should not exit */
					exit(sh_error("malloc", 125));
				envp = p;
				envp[envp_cnt++] = parser->tokens[*pos]->string;
				continue;
			case TC_IO_NUMBER:
			case TC_OP_CLOBBER:
			case TC_OP_GREAT:
			case TC_OP_GREATAND:
			case TC_OP_LESS:
			case TC_OP_LESSAND:
			case TC_OP_LESSGREAT:
				/* FIXME */
				continue;
			case TC_WORD:
				if((p = realloc(argv, sizeof(char *)
								* (argv_cnt+2)))
						== NULL)
					/* FIXME should not exit */
					exit(sh_error("malloc", 125));
				argv = p;
				argv[argv_cnt++] = parser->tokens[*pos]->string;
				continue;
			default:
				break;
		}
	}
	if(envp != NULL)
		envp[envp_cnt] = NULL;
	ret = _exec_cmd_env(envp);
	free(envp);
	if(argv != NULL && ret == 0)
	{
		argv[argv_cnt] = NULL;
		/* FIXME look for builtins utilities (should be none) */
		/* FIXME look for functions */
		/* FIXME look for builtin utilities */
		if((ret = _exec_cmd_builtin(argv_cnt, argv)) >= 125) /* FIXME */
			ret = _exec_cmd_child(argv_cnt, argv);
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
		if(setenv(*e, p+1, 1) != 0)
			ret+=sh_error("setenv", 1);
		*p = '=';
	}
	return ret;
}

static int _exec_cmd_builtin(int argc, char ** argv)
{
	struct {
		char * cmd;
		int (*func)(int, char**);
	} builtins[] = {
		{ "bg",    builtin_bg },
		{ "cd",    builtin_cd },
		{ "fg",    builtin_fg },
		{ "jobs",  builtin_jobs },
		{ "read",  builtin_read },
		{ "set",   builtin_set },
		{ "unset", builtin_unset },
		{ NULL,    NULL }
	};
	unsigned int i;

	for(i = 0; builtins[i].cmd != NULL; i++)
		if(strcmp(builtins[i].cmd, argv[0]) == 0)
			return builtins[i].func(argc, argv);
	return 125; /* FIXME */
}

static int _exec_cmd_child(int argc, char ** argv)
{
	pid_t pid;
	int status;
	int ret;

	if((pid = fork()) == -1)
		return sh_error("fork", 125);
	if(pid == 0)
	{
		execvp(argv[0], argv);
		exit(sh_error(argv[0], 125));
	}
	while((ret = waitpid(pid, &status, 0)) != -1)
		if(WIFEXITED(status))
			break;
	if(ret == -1)
		return sh_error("waitpid", 125);
	return WEXITSTATUS(status);
}

static void _exec_free(Parser * parser)
{
	unsigned int i;

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


/* rules */
static void parser_rule1(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "%s", "rule 1\n");
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


static void parser_rule7b(Parser * parser);
static void parser_rule7a(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "%s", "rule 7a\n");
#endif
	if(parser->token == NULL || parser->token->string == NULL)
		return;
	for(i = 0; parser->token->string[i] != '\0'; i++)
		if(parser->token->string[i] == '=')
			return parser_rule7b(parser);
	return parser_rule1(parser);
}


static void parser_rule7b(Parser * parser)
{
	unsigned int i;

#ifdef DEBUG
	fprintf(stderr, "%s", "rule 7b\n");
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
		if(p->token->code == TC_OP_AND_IF)
			;
		else if(p->token->code == TC_OP_OR_IF)
			;
		else
			return;
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
	/* FIXME */
	pipe_sequence(p);
}


/* pipe_sequence */
static void command(Parser * p);
static void pipe_sequence(Parser * p)
	/* command { '|' linebreak command } */
{
	/* FIXME */
	command(p);
}


/* command */
static void simple_command(Parser * p);
static void command(Parser * p)
	/* simple_command
	 * | compound_command [redirect_list]
	 * | function_definition */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "command()\n");
#endif
	parser_rule7a(p);
	/* FIXME */
	simple_command(p);
}


/* compound_command */


/* subshell */


/* compound_list */


/* term */


/* for_clause */


/* name */


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
static void io_redirect(Parser * p);
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


/* io_redirect */
static void io_redirect(Parser * p)
{
	/* FIXME */
}


/* linebreak */
static void newline_list(Parser * p);
static void linebreak(Parser * p)
	/* newline_list
	 * | */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "linebreak()\n");
#endif
	if(p->token != NULL && token_in_set(p->token, TS_NEWLINE_LIST))
		newline_list(p);
}


/* newline_list */
static void newline_list(Parser * p)
	/* NEWLINE { NEWLINE } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "newline_list()\n");
#endif
	if(p->token != NULL && p->token->code != TC_NEWLINE)
		return parser_error(p, "%s", "newline expected");
	/* FIXME
	parser_scan(p);
	while(p->token != NULL && p->token->code == TC_NEWLINE)
		parser_scan(p); */
	p->token = NULL;
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
		return newline_list(p);
	if(!token_in_set(p->token, TS_SEPARATOR_OP))
		return parser_error(p, "%s", "separator or newline expected");
	separator_op(p);
	linebreak(p);
}


/* sequential_sep */
/* FIXME */
