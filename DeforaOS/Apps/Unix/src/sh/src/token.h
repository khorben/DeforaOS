/* token.h */



#ifndef __TOKEN_H
# define __TOKEN_H

#include "tokencode.h"


/* Token */
/* types */
typedef struct _Token {
	TokenCode code;
	char * string;
	struct _Token * next;
} Token;


/* variables */
extern Token * null_token;


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

/* useful */
void token_debug(Token * token);
void token_distinct(Token * token);

#endif /* __TOKEN_H */
