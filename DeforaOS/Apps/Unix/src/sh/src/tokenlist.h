/* tokenlist.h */



#ifndef __TOKENLIST_H
# define __TOKENLIST_H

#include "token.h"


/* TokenList */
/* types */
typedef Token * TokenList;


/* variables */
extern TokenList * null_tokenlist;


/* functions */
TokenList * tokenlist_new(char * string);
void tokenlist_delete(TokenList * tokenlist);

/* returns */
TokenList * tokenlist_next(TokenList * tokenlist);
/* Token */
Token * tokenlist_first_token(TokenList * tokenlist);

/* useful */
TokenList * tokenlist_append(TokenList * tokenlist, Token * token);

#endif /* __TOKENLIST_H */
