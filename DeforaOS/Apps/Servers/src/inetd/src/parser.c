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

static void _newline(State * state)
{
}

static void _service_list(State * state)
{
}
