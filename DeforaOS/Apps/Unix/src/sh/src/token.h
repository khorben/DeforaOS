/* token.h */



#ifndef _TOKEN_H
# define _TOKEN_H

# include "tokencode.h"


/* types */
typedef struct _Token {
	TokenCode code;
	char * str;
} Token;


/* functions */
Token * token_new(TokenCode tokencode, char * string);
void token_delete(Token * token);

#endif /* !_TOKEN_H */
