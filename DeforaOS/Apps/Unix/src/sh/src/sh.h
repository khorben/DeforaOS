/* sh.h */



#ifndef __SH_H
# define __SH_H


/* types */
/* Prefs */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_i 0x2
#define PREFS_s 0x4


/* variables */
extern char ** export;


/* functions */
int sh_error(char * message, int ret);
char ** sh_export(void);

#endif /* !__SH_H */
