/* inetd.h */



#ifndef __INETD_H
# define __INETD_H

# include "config.h"


/* types */
typedef struct _InetdState {
	int debug;
	int queue;
	Config * config;
} InetdState;


/* variables */
extern InetdState * inetd_state;


/* functions */
int inetd_error(char const * message, int ret);

#endif /* !__INETD_H */
