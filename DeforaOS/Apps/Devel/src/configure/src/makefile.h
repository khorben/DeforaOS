/* makefile.h */



#ifndef __MAKEFILE_H
# define __MAKEFILE_H

# include <System.h>
# include "configure.h"

/* FIXME should be:
ARRAY(Config *, config);
but it can't be included multiple times */
typedef Array configArray;
extern configArray * configarray_new(void);

/* functions */
int makefile(Configure * configure, Config * config, String * directory,
		configArray * ca, int from, int to);

#endif
