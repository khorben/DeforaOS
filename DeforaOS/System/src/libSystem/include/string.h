/* string.h */



#ifndef _STRING_H
# define _STRING_H


/* String */
typedef char String;

String * string_new(String * string);
void string_delete(String * string);

/* useful */
int string_append(String ** string, String * append);
void string_cut(String * string, unsigned int length);
int string_length(String * string);

#endif /* !_STRING_H */
