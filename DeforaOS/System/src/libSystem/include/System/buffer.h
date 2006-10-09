/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_BUFFER_H
# define LIBSYSTEM_BUFFER_H


/* Buffer */
/* types */
typedef struct _Buffer
{
	unsigned int size;
	char * data;
} Buffer;


/* functions */
Buffer * buffer_new(unsigned int size, char * data);
void buffer_delete(Buffer * buffer);

/* returns */
char * buffer_data(Buffer const * buffer);
unsigned int buffer_length(Buffer const * buffer);

#endif /* !LIBSYSTEM_BUFFER_H */
