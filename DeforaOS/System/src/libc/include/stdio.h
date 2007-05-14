/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */



#ifndef LIBC_STDIO_H
# define LIBC_STDIO_H

# include "stddef.h"


/* constants */
/* size of <stdio.h> buffers */
# define BUFSIZ 8192

/* end-of-file return value */
# define EOF (-1)


/* types */
typedef struct _FILE FILE;

# ifndef va_list
#  define va_list va_list
typedef void * va_list;
# endif


/* variables */
/* standard input, output, and error streams */
extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;


/* functions */
void clearerr(FILE * file);
int fclose(FILE * file);
FILE * fdopen(int fildes, char const * mode);
int feof(FILE * file);
int fflush(FILE * file);
int fgetc(FILE * file);
char * fgets(char * str, int size, FILE * file);
int fileno(FILE * file);
FILE * fopen(char const * path, char const * mode);
int fprintf(FILE * file, char const * format, ...);
int fputc(int c, FILE * file);
int fputs(char const * str, FILE * file);
size_t fread(void * ptr, size_t size, size_t nb, FILE * file);
FILE * freopen(char const * path, char const * mode, FILE * file);
size_t fwrite(void const * ptr, size_t size, size_t nb, FILE * file);
int getc(FILE * file);
int getchar(void);
void perror(char const * str);
int printf(char const * format, ...);
int putc(int c, FILE * file);
int putchar(int c);
int puts(char const * string);
int rename(char const * from, char const * to);
int snprintf(char * str, size_t size, char const * format, ...);
int sprintf(char * str, char const * format, ...);
int vfprintf(FILE * file, char const * format, va_list arg);
int vsnprintf(char * str, size_t size, char const * format, va_list arg);
int vsprintf(char * str, char const * format, va_list arg);

#endif /* !LIBC_STDIO_H */
