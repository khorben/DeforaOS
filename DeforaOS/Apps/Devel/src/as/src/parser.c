/* parser.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"


/* types */
typedef struct _State
{
	int prefs;
	char * infile;
	FILE * infp;
	char * outfile;
	FILE * outfp;
	Token * token;
	unsigned int line;
	unsigned int errors;
} State;

/* parser */
static int _parser_fatal(State * state, char * message);
static void _parser_error(State * state, char * message);
static void _parser_warning(State * state, char * message);
static void _parser_scan(State * state);
static int _parser_check(State * state, TokenCode code);
static void _as(State * state);
int parser(int prefs, char * infile, FILE * infp, char * outfile, FILE * outfp)
	/* as */
{
	State state;

	state.prefs = prefs;
	state.infile = infile;
	state.infp = infp;
	state.outfile = outfile;
	state.outfp = outfp;
	state.token = scan(infp);
	state.line = 1;
	state.errors = 0;
	if(state.token != NULL)
		_as(&state);
	if(state.token == NULL)
		return _parser_fatal(&state, infile);
	if(state.token->code != TC_EOF)
	{
		if(state.token->string != NULL)
		{
			_parser_warning(&state, "parse error near token:");
			_parser_warning(&state, state.token->string);
		}
		_parser_error(&state, "unhandled syntax error, exiting");
	}
	if(state.errors)
		fprintf(stderr, "%s%s%s%d%s", "as: ", infile,
				": Compilation failed with ",
				state.errors, " error(s)\n");
	token_delete(state.token);
	return state.errors;
}

static int _parser_fatal(State * state, char * message)
{
	fprintf(stderr, "%s%s%u%s", state->infile, ", line ", state->line,
			": ");
	perror(message);
	return 2;
}

static void _parser_error(State * state, char * message)
{
	fprintf(stderr, "%s%s%u%s%s%s", state->infile, ", line ",
			state->line, ": ", message, "\n");
	state->errors++;
}

static void _parser_warning(State * state, char * message)
{
	fprintf(stderr, "%s%s%u%s%s%s", state->infile, ", line ",
			state->line, ": ", message, "\n");
}

static void _parser_scan(State * state)
{
	token_delete(state->token);
	state->token = scan(state->infp);
}

static int _parser_check(State * state, TokenCode code)
{
	int ret = 0;

	if(state->token == NULL || state->token->code != code)
		_parser_error(state, "parse error");
	else
		ret = 1;
	_parser_scan(state);
	return ret;
}

static void _newline(State * state);
static void _section_list(State * state);
static void _as(State * state)
	/* { newline } section_list { newline } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_as()\n");
#endif
	while(token_in_set(state->token, TS_NEWLINE))
		_newline(state);
	_section_list(state);
	while(token_in_set(state->token, TS_NEWLINE))
		_newline(state);
}

static void _space(State * state);
static void _newline(State * state)
	/* [ space ] NEWLINE */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_newline()\n");
#endif
	if(token_in_set(state->token, TS_SPACE))
		_space(state);
	if(_parser_check(state, TC_NEWLINE))
		state->line++;
}

static void _space(State * state)
	/* SPACE { SPACE } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_space()\n");
#endif
	_parser_check(state, TC_SPACE);
	while(state->token->code == TC_SPACE)
		_parser_scan(state);
}

static void _section(State * state);
static void _instruction_list(State * state);
static void _section_list(State * state)
	/* { section instruction_list } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_section_list()\n");
#endif
	while(token_in_set(state->token, TS_SECTION))
	{
		_section(state);
		_instruction_list(state);
	}
}

static void _section(State * state)
	/* "." WORD newline */
{
	char * section = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_section()\n");
#endif
	_parser_check(state, TC_DOT);
	if(state->token->code == TC_WORD && state->token->string != NULL)
		section = strdup(state->token->string);
	_parser_check(state, TC_WORD);
	_newline(state);
#ifdef DEBUG
	if(section)
		fprintf(stderr, "%s%s%s", "Entering section \"", section,
				"\"\n");
#endif
	free(section);
}

static void _function(State * state);
static void _instruction(State * state);
static void _instruction_list(State * state)
	/* { (function | space instruction | [space] newline) } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_instruction_list()\n");
#endif
	while(token_in_set(state->token, TS_INSTRUCTION_LIST))
	{
		if(token_in_set(state->token, TS_FUNCTION))
			_function(state);
		else if(token_in_set(state->token, TS_SPACE))
		{
			_space(state);
			if(token_in_set(state->token, TS_INSTRUCTION))
				_instruction(state);
		}
		else
			_newline(state);
	}
}

static void _function(State * state)
	/* WORD ":" newline */
{
	char * function = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_instruction_list()\n");
#endif
	if(state->token->code == TC_WORD && state->token->string != NULL)
		function = strdup(state->token->string);
	_parser_check(state, TC_WORD);
	_parser_check(state, TC_COLON);
	_newline(state);
#ifdef DEBUG
	if(function)
		fprintf(stderr, "%s%s%s", "Entering function: \"", function, "\"\n");
#endif
	free(function);
}

static void _operator(State * state);
static void _operand_list(State * state);
static void _instruction(State * state)
	/* operator [ space [ operand_list ] ] newline */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_instruction()\n");
#endif
	_operator(state);
	if(token_in_set(state->token, TS_SPACE))
	{
		_space(state);
		if(token_in_set(state->token, TS_OPERAND_LIST))
			_operand_list(state);
	}
	_newline(state);
}

static void _operator(State * state)
	/* WORD */
{
	char * operator = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_operator()\n");
#endif
	if(state->token->code == TC_WORD && state->token->string != NULL)
		operator = strdup(state->token->string);
	_parser_check(state, TC_WORD);
#ifdef DEBUG
	if(operator)
		fprintf(stderr, "%s%s%s", "Operator \"", operator, "\"\n");
#endif
	free(operator);
}

static void _operand(State * state);
static void _operand_list(State * state)
	/* operand { "," { space } operand } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_operand_list()\n");
#endif
	_operand(state);
	while(state->token->code == TC_COMMA)
	{
		_parser_scan(state);
		while(token_in_set(state->token, TS_SPACE))
			_space(state);
		_operand(state);
	}
}

static void _operand(State * state)
	/* (WORD | NUMBER) */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_operand()\n");
	fprintf(stderr, "%s%s%s", "New operand: \"", state->token->string,
			"\"\n");
#endif
	_parser_scan(state);
}

