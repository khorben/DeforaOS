/* hash.h */



#ifndef _HASH_H_
# define _HASH_H_

# include "array.h"


/* Hash */
/* types */
typedef Array Hash;


/* functions */
/* hash */
Hash * hash_new(void);
void hash_delete(Hash * h);


/* useful */
void * hash_get(Hash * h, char const * name);
int hash_set(Hash * h, char const * name, void * data);


#endif /* !_HASH_H_ */
