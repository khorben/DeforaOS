/* buffer.h */



#ifndef _BUFFER_H
# define _BUFFER_H


/* Buffer */
typedef struct _Buffer
{
	unsigned int size;
	char * data;
} Buffer;

Buffer * buffer_new(unsigned int size, char * data);
void buffer_delete(Buffer * buffer);

/* returns */
char * buffer_data(Buffer const * buffer);
unsigned int buffer_length(Buffer const * buffer);

#endif /* !_BUFFER_H */
