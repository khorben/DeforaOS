/* parser.c */



#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "service.h"
#include "parser.h"


/* types */
typedef struct _State
{
	char * filename;
	FILE * fp;
	Token * token;
	unsigned int line;
	Config * config;
	Service service;
} State;


static void _config(State * state);
Config * parser(char * filename)
{
	State state;

	if((state.config = config_new()) == NULL)
		return NULL; /* FIXME be verbose */
	if((state.fp = fopen(filename, "r")) == NULL)
		return NULL; /* FIXME be verbose */
	if((state.token = scan(state.fp)) == NULL)
	{
		fclose(state.fp);
		return NULL; /* FIXME be verbose */
	}
	state.filename = filename;
	state.line = 1;
	_config(&state);
	token_delete(state.token);
	fclose(state.fp);
	return state.config;
}

static void _parser_error(State * state, char * message)
{
	fprintf(stderr, "%s%s%d%s%s\n", state->filename, ", line ", state->line,
			": ", message);
}

static void _parser_scan(State * state)
{
	token_delete(state->token);
	state->token = scan(state->fp);
}

static int _parser_check(State * state, TokenCode code)
{
	int ret = 0;

	if(state->token == NULL || state->token->code != code)
		_parser_error(state, "parse error"); /* FIXME tell expected */
	else
		ret = 1;
	_parser_scan(state);
	return ret;
}

static void _newline(State * state);
static void _service(State * state);
static void _config(State * state)
	/* { newline | service } */
{
	for(;;)
	{
		if(token_in_set(state->token, TS_NEWLINE))
			_newline(state);
		else if(token_in_set(state->token, TS_SERVICE))
			_service(state);
		else if(state->token->code == TC_EOF)
			break;
		else
		{
			/* FIXME be verbose */
			break;
		}
	}
}

static void _space(State * state);
static void _newline(State * state)
	/* [ space ] NEWLINE */
{
	if(token_in_set(state->token, TS_SPACE))
		_space(state);
	if(_parser_check(state, TC_NEWLINE))
		state->line++;
}

static void _space(State * state)
	/* SPACE { SPACE } */
{
	_parser_check(state, TC_SPACE);
	while(state->token->code == TC_SPACE)
		_parser_scan(state);
}

static void _service_name(State * state);
static void _socket(State * state);
static void _protocol(State * state);
static void _wait(State * state);
static void _id(State * state);
static void _program(State * state);
static void _service(State * state)
	/* service_name space socket space protocol space wait space id space
	 * program newline */
{
	_service_name(state);
	_space(state);
	_socket(state);
	_space(state);
	_protocol(state);
	_space(state);
	_wait(state);
	_space(state);
	_id(state);
	_space(state);
	_program(state);
	_newline(state);
	/* FIXME not so elegant */
	if(state->service.name != NULL)
	{
		config_service_add(state->config,
				service_new(state->service.name,
				state->service.socket, state->service.proto,
				state->service.wait, state->service.id,
				state->service.program));
		free(state->service.name);
	}
}

static void _service_name(State * state)
	/* WORD */
{
	char * p;

	state->service.name = strdup(state->token->string);
	_parser_check(state, TC_WORD);
}

static void _socket(State * state)
	/* "stream" | "dgram" */
{
	if(strcmp("stream", state->token->string) == 0)
		state->service.socket = SS_STREAM;
	else if(strcmp("dgram", state->token->string) == 0)
		state->service.socket = SS_DGRAM;
	else
		_parser_error(state, "stream or dgram expected");
	_parser_check(state, TC_WORD); /* FIXME use test result */
}

static void _protocol(State * state)
	/* "tcp" | "udp" */
{
	if(strcmp("tcp", state->token->string) == 0)
		state->service.proto = SP_TCP;
	else if(strcmp("udp", state->token->string) == 0)
		state->service.proto = SP_UDP;
	else
		_parser_error(state, "Expected \"tcp\" or \"udp\"");
	_parser_check(state, TC_WORD); /* FIXME use test result */
}

static void _wait(State * state)
	/* "wait" | "nowait" */
{
	if(strcmp("wait", state->token->string) == 0)
		state->service.wait = SW_WAIT;
	else if(strcmp("nowait", state->token->string) == 0)
		state->service.wait = SW_NOWAIT;
	else
		_parser_error(state, "Expected \"wait\" or \"nowait\"");
	_parser_check(state, TC_WORD); /* FIXME use test result */
}

static void _id(State * state)
	/* user [ "." group ] */
{
	/* FIXME */
	_parser_check(state, TC_WORD);
}

static void _program_argument(State * state);
static void _program_name(State * state);
static void _program(State * state)
	/* program_name [ space ] { program_argument space } */
{
	state->service.program = NULL;
	_program_name(state);
	if(token_in_set(state->token, TS_SPACE))
		_space(state);
	while(token_in_set(state->token, TS_PROGRAM_ARGUMENT))
	{
		_program_argument(state);
		_space(state);
	}
}

static void _program_name(State * state)
	/* WORD */
{
	if((state->service.program = malloc(2 * sizeof(char*))) != NULL)
	{
		state->service.program[0] = strdup(state->token->string);
		state->service.program[1] = NULL;
	}
	_parser_check(state, TC_WORD); /* FIXME use test result */
}

static void _program_argument(State * state)
	/* WORD */
{
	unsigned int i;
	char ** p;

	if(state->service.program == NULL)
	{
		_parser_check(state, TC_WORD);
		return; /* FIXME */
	}
	for(i = 0; state->service.program[i] != NULL; i++);
	if((p = realloc(state->service.program, sizeof(char) * (i+1))) == NULL)
	{
		_parser_check(state, TC_WORD);
		return; /* FIXME */
	}
	state->service.program = p;
	state->service.program[i-1] = strdup(state->token->string);
	state->service.program[i] = NULL;
	_parser_check(state, TC_WORD);
}
