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


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

void token_distinct(Token * token);

#endif /* __TOKEN_H */
