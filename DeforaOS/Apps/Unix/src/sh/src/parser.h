/* parser.h */



#ifndef _PARSER_H
# define _PARSER_H

# include <stdio.h>
# include "scanner.h"


/* types */
typedef struct _Parser {
	Scanner scanner;
	Token * token;
} Parser;


/* functions */
Parser * parser_new(FILE * fp);
Parser * parser_new_from_string(char const * string);
void parser_delete(Parser * parser);

/* returns */
TokenCode parser_code(Parser * parser);

/* useful */
void parser_error(Parser * parser, char const * format, ...);
int parser_parse(Parser * parser);
void parser_scan(Parser * parser);

/* tests */
int parser_test(Parser * parser, TokenCode tokencode);
int parser_test_set(Parser * parser, TokenCode codeset[]);
int parser_test_word(Parser * parser, char const * word);

/* checks */
int parser_check(Parser * parser, TokenCode tokencode);
int parser_check_set(Parser * parser, TokenCode codeset[]);
int parser_check_word(Parser * parser, char const * word);

/* rules */
void parser_rule1(Parser * parser);
void parser_rule7a(Parser * parser);
void parser_rule7b(Parser * parser);

#endif /* !_PARSER_H */
