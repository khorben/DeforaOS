/* builtin.h */
/* FIXME
 * - implement these each in its own file?
 * - include their .h in this file? */
/* FIXME need to define and track down State
 * - define it in builtin.c? */



#ifndef __BUILTIN_H
# define __BUILTIN_H


/* functions */
int builtin_alias(int argc, char * argv[]);
int builtin_bg(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_cd(int argc, char * argv[]);
int builtin_exec(int argc, char * argv[]);
int builtin_exit(int argc, char * argv[]);
int builtin_fg(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_jobs(int argc, char * argv[]); /* FIXME only in XSI */
int builtin_read(int argc, char * argv[]);
int builtin_set(int argc, char * argv[]);
int builtin_umask(int argc, char * argv[]);
int builtin_unalias(int argc, char * argv[]);
int builtin_unset(int argc, char * argv[]);

#endif /* !__BUILTIN_H */
