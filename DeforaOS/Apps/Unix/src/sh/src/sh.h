/* sh.h */



#ifndef __SH_H
# define __SH_H


/* types */
/* Prefs */
typedef int Prefs;
#define PREFS_c 01
#define PREFS_i 02
#define PREFS_s 04


/* functions */
int sh_error(char * message, int ret);

#endif /* !__SH_H */
