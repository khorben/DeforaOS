/* parser.h */



#ifndef __PARSER_H
# define __PARSER_H

# include <stdio.h>
# include "sh.h"


/* functions */
int parser(Prefs * prefs, char const * string, FILE * fp, int argc,
		char * argv[]);

#endif /* !__PARSER_H */
