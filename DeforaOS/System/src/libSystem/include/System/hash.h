/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_HASH_H
# define LIBSYSTEM_HASH_H

# include "array.h"


/* Hash */
/* types */
typedef Array Hash;


/* functions */
Hash * hash_new(void);
void hash_delete(Hash * h);

/* useful */
void * hash_get(Hash * h, char const * name);
int hash_set(Hash * h, char const * name, void * data);

#endif /* !LIBSYSTEM_HASH_H */
