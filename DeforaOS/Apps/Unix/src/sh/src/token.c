/* token.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"


/* Token */
/* variables */
static Token _null_token = { TC_NULL, NULL, NULL };
Token * null_token = &_null_token;

/* token_new */
Token * token_new(TokenCode code, char * string)
{
	Token * token;

	if((token = malloc(sizeof(Token))) == NULL)
	{
		perror("malloc");
		return NULL;
	}
#ifdef DEBUG
	fprintf(stderr, "token_new(%d, \"%s\"): %p\n", code, string, token);
#endif
	if((token->string = strdup(string)) == NULL)
	{
		perror("strdup");
		free(token);
		return NULL;
	}
	token->code = code;
	token->next = NULL;
	return token;
}


void token_delete(Token * token)
{
	free(token->string);
	free(token);
}


/* useful */
void token_debug(Token * token)
{
	fprintf(stderr, "Token code: %d, string: %s, next: %p\n",
			token->code, token->string, token->next);
}

void token_distinct(Token * token)
{
	int i;

	for(i = TC_RW_IF; i <= TC_RW_IN; i++)
	{
		if(strcmp(sTokenCode[i], token->string) == 0)
		{
			token->code = i;
			return;
		}
	}
	/* FIXME assignment words */
	token->code = TC_WORD;
}
