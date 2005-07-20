/* scanner.h */



#ifndef __SCANNER_H
# define __SCANNER_H

# include <stdio.h>
# include "token.h"


/* types */
typedef struct _Scanner {
	FILE * fp;
	char const * str;
	int c;
} Scanner;


/* functions */
void scanner_init(Scanner * scanner, FILE * fp, char const * string);

/* useful */
Token * scanner_next(Scanner * scanner);

#endif /* !__SCANNER_H */
