/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_ARRAY_H
# define LIBSYSTEM_ARRAY_H


/* Array */
/* macros */
# define ARRAY(type, name) \
	typedef Array name ## Array; \
	Array * name ## array_new(void) { return array_new(sizeof(type)); }


/* types */
typedef struct _Array
{
	char * data;
	unsigned int count;
	unsigned int size;
} Array;

typedef void (*ArrayApplyFunc)(void * data, void * userdata);


/* functions */
Array * array_new(unsigned int size);
void array_delete(Array * array);

/* returns */
unsigned int array_count(Array * array);

/* useful */
void * array_get(Array * array, unsigned int pos);
int array_get_copy(Array * array, unsigned int pos, void * data);
int array_set(Array * array, unsigned int pos, void * data);

int array_append(Array * array, void * data);
int array_remove_pos(Array * array, unsigned int pos);

void array_apply(Array * array, ArrayApplyFunc func, void * userdata);

#endif /* !LIBSYSTEM_ARRAY_H */
