/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_STRING_H
# define LIBSYSTEM_STRING_H


/* String */
/* types */
typedef char String;


/* functions */
String * string_new(String const * string);
void string_delete(String * string);

/* returns */
int string_length(String const * string);

/* useful */
int string_append(String ** string, String const * append);
void string_cut(String * string, unsigned int length);

int string_compare(String const * string, String const * string2);
int string_compare_length(String const * string, String const * string2,
		unsigned int length);

String * string_find(String const * string, String const * key);

#endif /* !LIBSYSTEM_STRING_H */
