/* array.h */



#ifndef ___ARRAY_H
# define ___ARRAY_H

# include "array.h"


/* Array */
/* types */
typedef struct _Array {
	void ** data;
	unsigned int size;
} Array;


/* functions */
Array * array_new(void);
void array_delete(Array * array);

unsigned int array_get_size(Array * array);
int array_set_size(Array * array, unsigned int size);

void * array_get(Array * array, unsigned int pos);
int array_set(Array * array, unsigned int pos, void * data);

int array_append(Array * array, void * data);

# endif /* !___ARRAY_H */
