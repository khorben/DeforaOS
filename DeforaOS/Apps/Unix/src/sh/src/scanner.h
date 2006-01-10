/* scanner.h */



#ifndef __SCANNER_H
# define __SCANNER_H

# include <stdio.h>
# include "token.h"
# include "sh.h"


/* Scanner */
typedef enum _ScannerPrompt { SP_PS1 = 0, SP_PS2, SP_PS4 } ScannerPrompt;
# define SP_LAST SP_PS4
typedef struct _Scanner
{
	FILE * fp;
	ScannerPrompt prompt;
	const char * string;
	int (*next)(struct _Scanner *);
} Scanner;

void scanner_init(Scanner * scanner, Prefs * prefs, FILE * fp,
		char const * string);
Token * scanner_next(Scanner * scanner);

#endif /* !__SCANNER_H */
