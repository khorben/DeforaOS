/* sh.h */



#ifndef __SH_H
# define __SH_H

#include "prefs.h"


/* sh */
int sh_file(struct prefs * p, char * file);
int sh_string(struct prefs * p, char * string, int argc, char * argv[]);

#endif /* __SH_H */
