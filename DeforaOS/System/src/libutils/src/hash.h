/* hash.h */



#ifndef ___HASH_H
# define ___HASH_H

# include "darray.h"


/* types */
typedef DArray Hash;


/* functions */
/* hash */
Hash * hash_new(void);
void hash_delete(Hash * h);


/* useful */
/* hash_get
 * RETURNS	NULL	success or the corresponding data == NULL
 * 		else	the corresponding data */
void * hash_get(Hash * h, char * name);

/* hash_get_index
 * RETURNS	NULL	success or the corresponding data == NULL
 * 		else	the corresponding data */
void * hash_get_index(Hash * h, unsigned int index);

/* hash_get_size
 * RETURNS	any	the size of the hash table */
unsigned int hash_get_size(Hash * h);

/* hash_set
 * RETURNS	-1	failure
 * 		0	success */
int hash_set(Hash * h, char * name, void * data);


#endif /* !___HASH_H */
