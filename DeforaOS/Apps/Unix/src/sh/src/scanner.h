/* scanner.h */



#ifndef __SCANNER_H
# define __SCANNER_H

# include <stdio.h>
# include "token.h"
# include "sh.h"


/* Scanner */
typedef struct _Scanner
{
	FILE * fp;
	const char * string;
	int (* next)(struct _Scanner *);
} Scanner;

void scanner_init(Scanner * scanner, Prefs * prefs, FILE * fp,
		char const * string);
Token * scanner_next(Scanner * scanner);

#endif /* !__SCANNER_H */
