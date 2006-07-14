/* makefile.h */



#ifndef CONFIGURE_MAKEFILE_H
# define CONFIGURE_MAKEFILE_H

# include <System.h>
# include "configure.h"


/* types */
/* FIXME should be:
ARRAY(Config *, config);
but it can't be included multiple times */
typedef Array configArray;
extern configArray * configarray_new(void);


/* functions */
int makefile(Configure * configure, String * directory, configArray * ca,
	       	int from, int to);
		

#endif /* !CONFIGURE_MAKEFILE_H */
