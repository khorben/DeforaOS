/* token.c */



#include <stdlib.h>
#include <stdio.h>
#include "token.h"


/* Token */
Token * token_new(TokenCode tokencode, char * string)
{
	Token * token;

	if((token = malloc(sizeof(Token))) == NULL)
	{
		fprintf(stderr, "%s", "sh: ");
		perror("malloc");
		return NULL;
	}
	token->code = tokencode;
	token->str = string;
	return token;
}

void token_delete(Token * token)
{
	free(token->str);
	free(token);
}
