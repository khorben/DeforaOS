/* as.h */



#ifndef __AS_H
# define __AS_H

# define AS_FILENAME_DEFAULT "a.out"


/* functions */
/* useful */
int as_error(char * msg, int ret);

/* plugins helpers */
void * as_plugin_new(char * type, char * name, char * description);
void as_plugin_delete(void * plugin);
void as_plugin_list(char * type, char * description);

#endif /* !__AS_H */
