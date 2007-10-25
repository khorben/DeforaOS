/* $Id$ */



#ifndef DIRECTORY_DIRECTORY_H
# define DIRECTORY_DIRECTORY_H

# include <System.h>


/* Directory */
/* types */
typedef struct _Directory Directory;


/* functions */
Directory * directory_new(Event * event);
void directory_delete(Directory * directory);

#endif /* !DIRECTORY_DIRECTORY_H */
