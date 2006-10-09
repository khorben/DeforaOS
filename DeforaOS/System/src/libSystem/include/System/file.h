/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#ifndef LIBSYSTEM_FILE_H
# define LIBSYSTEM_FILE_H


/* types */
typedef struct _File File;
typedef int FileMode; /* FIXME actually is an enumerated type */

/* functions */
File * file_new(char const * path, FileMode mode);
void file_delete(File * file);

/* useful */
ssize_t file_read(File * file, void * buf, size_t size, ssize_t count);
ssize_t file_write(File * file, void * buf, size_t size, ssize_t count);

#endif /* !LIBSYSTEM_FILE_H */
