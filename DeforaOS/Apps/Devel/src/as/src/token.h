/* token.h */



#ifndef __TOKEN_H
# define __TOKEN_H


/* types */
typedef enum _TokenCode
{
	TC_COLON = 0,
	TC_COMMA,
	TC_DOT,
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
extern TokenCode TS_FUNCTION[];
extern TokenCode TS_INSTRUCTION[];
extern TokenCode TS_INSTRUCTION_LIST[];
extern TokenCode TS_NEWLINE[];
extern TokenCode TS_OPERAND[];
extern TokenCode TS_OPERAND_LIST[];
extern TokenCode TS_OPERATOR[];
extern TokenCode TS_SECTION[];
extern TokenCode TS_SECTION_LIST[];
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
