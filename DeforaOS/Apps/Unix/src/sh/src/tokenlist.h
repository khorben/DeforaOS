/* tokenlist.h */



#ifndef __TOKENLIST_H
# define __TOKENLIST_H

#include "token.h"


/* TokenList */
/* types */
typedef struct _TokenList {
	Token ** tokens;
	int size;
} TokenList;


/* functions */
TokenList * tokenlist_new(void);
TokenList * tokenlist_new_from_string(char * string);
void tokenlist_delete(TokenList * tokenlist);

/* useful */
void tokenlist_append(TokenList * tokenlist, Token * token);

#endif /* __TOKENLIST_H */
