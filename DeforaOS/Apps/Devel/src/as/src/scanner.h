/* scanner.h */



#ifndef __SCANNER_H
# define __SCANNER_H

# include "token.h"


/* functions */
Token * scan(FILE * fp);
Token * check(FILE * fp, TokenCode code);

#endif /* !__SCANNER_H */
