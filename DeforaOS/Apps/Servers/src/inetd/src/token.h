/* token.h */



#ifndef __TOKEN_H
# define __TOKEN_H


/* types */
typedef enum _TokenCode
{
	TC_EOF,
	TC_NEWLINE,
	TC_NUMBER,
	TC_SPACE,
	TC_TAB,
	TC_WORD,
	TC_NULL
} TokenCode;
#ifdef DEBUG
extern char * sTokenCode[TC_NULL];
#endif

typedef TokenCode * TokenSet;
extern TokenCode TS_NEWLINE[];
extern TokenCode TS_PROGRAM_ARGUMENT[];
extern TokenCode TS_SERVICE[];
extern TokenCode TS_SPACE[];

typedef struct _Token
{
	char * string;
	TokenCode code;
} Token;


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

int token_in_set(Token * token, TokenSet set);

#endif /* !__TOKEN_H */
