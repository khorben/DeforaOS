/* parser.h */



#ifndef __PARSER_H
# define __PARSER_H

#include "tokenlist.h"


/* functions */
Token ** complete_command(TokenList * token, int argc, char * argv[]);

#endif /* __PARSER_H */
