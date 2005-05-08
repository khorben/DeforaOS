/* code.h */



#ifndef __CODE_H
# define __CODE_H

# include <stdio.h>
# include "arch/arch.h"
# include "format/format.h"


/* types */
typedef struct _Code
{
	Arch * arch;
	Format * format;
	char * filename;
	FILE * fp;
} Code;


/* functions */
Code * code_new(char * arch, char * format, char * filename);
void code_delete(Code * code);

#endif /* !__CODE_H */
