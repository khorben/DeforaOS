/* array.h */



#ifndef _ARRAY_H
# define _ARRAY_H


# define ARRAY(type) \
	typedef Array type ## Array; \
	Array * type ## ArrayNew(void) { return array_new(sizeof(type)); }


/* types */
typedef struct _Array Array;

typedef void (*ArrayApplyFunc)(void * data, void * userdata);


/* functions */
Array * array_new(unsigned int size);
void array_delete(Array * array);

/* useful */
int array_append(Array * array, void * data);
void array_apply(Array * array, ArrayApplyFunc * func, void * userdata);
unsigned int array_count(Array * array);

#endif /* !_ARRAY_H */
