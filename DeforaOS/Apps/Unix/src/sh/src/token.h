/* token.h */



#ifndef __TOKEN_H
# define __TOKEN_H


/* Token */
typedef enum _TokenCode
{
	TC_EOI = 0,
	TC_TOKEN,
	TC_WORD,
	TC_ASSIGNMENT_WORD,
	TC_NAME,
	TC_NEWLINE,
	TC_IO_NUMBER,
	TC_OP_AND_IF,
	TC_OP_OR_IF,
	TC_OP_DSEMI,
	TC_OP_DLESS,
	TC_OP_DGREAT,
	TC_OP_LESSAND,
	TC_OP_GREATAND,
	TC_OP_LESSGREAT,
	TC_OP_DLESSDASH,
	TC_OP_CLOBBER,
	TC_OP_AMPERSAND,
	TC_OP_BAR,
	TC_OP_SEMICOLON,
	TC_OP_LESS,
	TC_OP_GREAT,
	TC_RW_IF,
	TC_RW_THEN,
	TC_RW_ELSE,
	TC_RW_ELIF,
	TC_RW_FI,
	TC_RW_DO,
	TC_RW_DONE,
	TC_RW_CASE,
	TC_RW_ESAC,
	TC_RW_WHILE,
	TC_RW_UNTIL,
	TC_RW_FOR,
	TC_RW_LBRACE,
	TC_RW_RBRACE,
	TC_RW_BANG,
	TC_RW_IN,
	TC_NULL
} TokenCode;
# define TC_LAST TC_RW_IN
extern char const * sTokenCode[TC_LAST+1];

typedef struct _Token
{
	TokenCode code;
	char * string;
} Token;

typedef enum _TokenSet
{
	TS_CMD_NAME = 0,
	TS_CMD_PREFIX,
	TS_CMD_SUFFIX,
	TS_CMD_WORD,
	TS_COMPOUND_COMMAND,
	TS_ELSE_PART,
	TS_IO_FILE,
	TS_IO_HERE,
	TS_IO_REDIRECT,
	TS_NEWLINE_LIST,
	TS_REDIRECT_LIST,
	TS_SEPARATOR,
	TS_SEPARATOR_OP,
	TS_WORDLIST
} TokenSet;
# define TS_LAST TS_WORDLIST


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

int token_in_set(Token * token, TokenSet set);

#endif /* !__TOKEN_H */
