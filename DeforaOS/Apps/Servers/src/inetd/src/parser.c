/* parser.c */



#include "parser.h"


/* types */
typedef struct _State
{
	int prefs;
	char * filename;
	FILE * fp;
	unsigned int line : 1;
} State;


static void _config(State * state);
int parser(int prefs, char * filename, FILE * fp)
{
	State state;

	state.prefs = prefs;
	state.filename = filename;
	state.fp = fp;
	_config(&state);
	return 1; /* FIXME */
}

static void _newline(State * state);
static void _service_list(State * state);
static void _config(State * state)
	/* { newline } service_list { newline } */
{
	/* FIXME uncomment */
/*	while(token_in_set(state->token, TS_NEWLINE))
		_newline(state); */
	_service_list(state);
/*	while(token_in_set(state->token, TS_NEWLINE))
		_newline(state); */
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

static void _service(State * state);
static void _service_list(State * state)
	/* { service { newline } } */
{
	while(token_in_set(state->token, TS_SERVICE))
	{
		_service(state);
		while(token_in_set(state->token, TS_NEWLINE))
			_newline(state);
	}
}

static void _service_name(State * state);
static void _socket(State * state);
static void _protocol(State * state);
static void _wait(State * state);
static void _id(State * state);
static void _program(State * state);
static void _service(State * state)
	/* service_name space socket space protocol space wait space
	 * id space program newline */
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
}

static void _service_name(State * state);
{
}

static void _socket(State * state);
{
}

static void _protocol(State * state);
{
}

static void _wait(State * state);
{
}

static void _id(State * state);
{
}

static void _program(State * state)
{
}
