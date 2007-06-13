/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"


/* types */
typedef struct _State
{
	char * infile;
	FILE * infp;
	Token * token;
	unsigned int line;
	unsigned int errors;
	Code * code;
	char * instruction;
	CodeOperand * operands;
	int operands_cnt;
} State;


/* parser */
static int _parser_fatal(State * state, char const * message);
static void _parser_error(State * state, char const * format, ...);
static void _parser_warning(State * state, char const * message);
static void _parser_scan(State * state);
static int _parser_check(State * state, TokenCode code);
static void _as(State * state);
int parser(Code * code, char * infile, FILE * infp)
	/* as */
{
	State state = { infile, infp, scan(infp), 1, 0, code, NULL, NULL, 0 };

	if(state.token != NULL)
		_as(&state);
	if(state.token == NULL)
		return _parser_fatal(&state, "Could not initialize scanner");
	if(state.token->code != TC_EOF)
	{
		if(state.token->string != NULL)
		{
			_parser_warning(&state, "Parse error near token:");
			_parser_warning(&state, state.token->string);
		}
		_parser_error(&state, "%s", "Unhandled syntax error, exiting");
	}
	if(state.errors)
		fprintf(stderr, "%s%s%s%d%s", "as: ", infile,
				": Compilation failed with ",
				state.errors, " error(s)\n");
	token_delete(state.token);
	free(state.operands);
	return state.errors;
}

static int _parser_fatal(State * state, char const * message)
{
	fprintf(stderr, "%s%s%s%u%s%s%s", "as: ", state->infile, ", line ",
			state->line, ": ", message, "\n");
	return 2;
}

/* static void _parser_error(State * state, char const * message) */
static void _parser_error(State * state, char const * format, ...)
{
	va_list ap;

	fprintf(stderr, "%s%s%s%u%s", "as: ", state->infile, ", line ",
			state->line, ": ");
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fputc('\n', stderr);
	state->errors++;
}

static void _parser_warning(State * state, char const * message)
{
	fprintf(stderr, "%s%s%s%u%s%s%s", "as: ", state->infile, ", line ",
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
		_parser_error(state, "%s", "Parse error");
	else
		ret = 1;
	_parser_scan(state);
	return ret;
}


/* as */
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


/* newline */
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


/* space */
static void _space(State * state)
	/* SPACE { SPACE } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_space()\n");
#endif
	_parser_check(state, TC_SPACE);
	while(state->token != NULL && state->token->code == TC_SPACE)
		_parser_scan(state);
}


/* section_list */
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


/* section */
static void _section(State * state)
	/* "." WORD newline */
{
	char * section = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_section()\n");
#endif
	_parser_check(state, TC_DOT);
	if(state->token == NULL)
		return;
	if(state->token->code == TC_WORD && state->token->string != NULL)
		section = strdup(state->token->string);
	_parser_check(state, TC_WORD);
	_newline(state);
#ifdef DEBUG
	if(section)
		fprintf(stderr, "%s%s%s", "Entering section \"", section,
				"\"\n");
#endif
	format_section(state->code->format, state->code->fp, section);
	free(section);
}


/* instruction_list */
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


/* function */
static void _function(State * state)
	/* WORD ":" newline */
{
	char * function = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_instruction_list()\n");
#endif
	if(state->token == NULL)
		return;
	if(state->token->code == TC_WORD && state->token->string != NULL)
		function = strdup(state->token->string);
	_parser_check(state, TC_WORD);
	_parser_check(state, TC_COLON);
	_newline(state);
#ifdef DEBUG
	if(function)
		fprintf(stderr, "%s%s%s", "Entering function: \"", function,
				"\"\n");
#endif
	free(function);
}


/* instruction */
static void _operator(State * state);
static void _operand_list(State * state);
static void _instruction(State * state)
	/* operator [ space [ operand_list ] ] newline */
{
	CodeError error;
	int i;

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
	if(state->instruction != NULL)
	{
		if((error = code_instruction(state->code, state->instruction,
				state->operands, state->operands_cnt))
				!= CE_SUCCESS)
		{
			if(error == CE_UNKNOWN_INSTRUCTION)
				_parser_error(state, "%s \"%s\"",
						code_error[error],
						state->instruction);
			else
				_parser_error(state, "%s", code_error[error]);
		}
		free(state->instruction);
		for(i = 0; i < state->operands_cnt; i++)
			free(state->operands[i].value);
		state->operands_cnt = 0;
	}
	_newline(state);
}


/* operator */
static void _operator(State * state)
	/* WORD */
{
	char * instruction = NULL;

#ifdef DEBUG
	fprintf(stderr, "%s", "_operator()\n");
#endif
	if(state->token == NULL)
		return;
	if(state->token->string != NULL)
		instruction = strdup(state->token->string);
	if(_parser_check(state, TC_WORD) && instruction)
	{
		state->instruction = instruction;
#ifdef DEBUG
		fprintf(stderr, "%s%s%s", "Operator \"", instruction, "\"\n");
#endif
	}
}


/* operand_list */
static void _operand(State * state);
static void _operand_list(State * state)
	/* operand [ space ] { "," [ space ] operand [ space ] } */
{
#ifdef DEBUG
	fprintf(stderr, "%s", "_operand_list()\n");
#endif
	_operand(state);
	if(token_in_set(state->token, TS_SPACE))
		_space(state);
	while(state->token != NULL && state->token->code == TC_COMMA)
	{
		_parser_scan(state);
		if(token_in_set(state->token, TS_SPACE))
			_space(state);
		_operand(state);
		if(token_in_set(state->token, TS_SPACE))
			_space(state);
	}
}


/* operand */
static void _operand(State * state)
	/* WORD | NUMBER | IMMEDIATE | REGISTER */
{
	CodeOperand * p;

	if(state->token == NULL)
		return;
#ifdef DEBUG
	fprintf(stderr, "%s", "_operand()\n");
	fprintf(stderr, "%s%s\"\n", "New operand: \"", state->token->string);
#endif
	if(state->token->string != NULL)
	{
		if((p = realloc(state->operands, (state->operands_cnt+1)
						* sizeof(CodeOperand))) == NULL)
			_parser_error(state, "%s", "Internal error");
		else
		{
			state->operands = p;
			state->operands[state->operands_cnt].type
				= state->token->code;
			/* FIXME necessary already here? */
			state->operands[state->operands_cnt].value
				= strdup(state->token->string);
			state->operands_cnt++;
		}
	}
	_parser_scan(state);
}
