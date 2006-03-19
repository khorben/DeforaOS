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


/* debugging */
# ifdef DEBUG
#  define malloc(a) dbg_malloc(a, __FILE__, __LINE__)
#  define realloc(a, b) dbg_realloc(a, b, __FILE__, __LINE__)
#  define free(a) dbg_free(a, __FILE__, __LINE__)
void * dbg_malloc(size_t size, char * file, int line);
void * dbg_realloc(void * ptr, size_t size, char * file, int line);
void dbg_free(void * ptr, char * file, int line);
# endif /* DEBUG */

#endif /* !__SH_H */
