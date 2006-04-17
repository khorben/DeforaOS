/* string.h */



#ifndef LIBSYSTEM_STRING_H
# define LIBSYSTEM_STRING_H


/* String */
typedef char String;

String * string_new(String const * string);
void string_delete(String * string);

/* useful */
int string_append(String ** string, String const * append);
int string_compare(String const * string, String const * string2);
int string_compare_length(String const * string, String const * string2,
		unsigned int length);
void string_cut(String * string, unsigned int length);
String const * string_find(String const * string, String const * key);
int string_length(String const * string);

#endif /* !LIBSYSTEM_STRING_H */
