/* token.c */



#include <assert.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include "token.h"


/* Token */
/* variables */
char * sTokenCode[TC_NULL] = {
	"TC_EOF",
	"TC_NEWLINE",
	"TC_NUMBER",
	"TC_SPACE",
	"TC_TAB",
	"TC_WORD"
};
TokenCode TS_NEWLINE[] = { TC_SPACE, TC_TAB, TC_NEWLINE, TC_NULL };
TokenCode TS_NEWLINE_LIST[] = { TC_SPACE, TC_TAB, TC_NEWLINE, TC_NULL };
TokenCode TS_SERVICE[] = { TC_WORD, TC_NULL };
TokenCode TS_SPACE[] = { TC_SPACE, TC_TAB, TC_NULL };


/* functions */
/* token_new */
Token * token_new(TokenCode code, char * string)
{
	Token * t;

	if((t = malloc(sizeof(Token))) == NULL)
		return NULL;
	t->code = code;
	t->string = NULL;
	if(string != NULL)
		if((t->string = strdup(string)) == NULL)
		{
			free(t);
			return NULL;
		}
#ifdef DEBUG
	fprintf(stderr, "%s%s%s%s%s", "token_new(", sTokenCode[code],
			", \"", string == NULL ? "NULL" : string, "\")\n");
#endif
	return t;
}

void token_delete(Token * t)
{
	assert(t != NULL);
	free(t->string);
	free(t);
}


int token_in_set(Token * t, TokenSet ts)
{
	assert(t != NULL);
	for(; *ts != TC_NULL && t->code != *ts; ts++);
	return *ts != TC_NULL;
}
