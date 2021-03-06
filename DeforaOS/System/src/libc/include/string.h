/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#ifndef LIBC_STRING_H
# define LIBC_STRING_H

# include "stddef.h"


/* functions */
void * memchr(void const * s, int c, size_t n);
int memcmp(void const * s1, void const * s2, size_t n);
void * memcpy(void * dest, void const * src, size_t n);
void * memmove(void * dest, void const * src, size_t n);
void * memset(void * dest, int c, size_t n);
char * strcat(char * dest, char const * src);
char * strchr(char const * s, int c);
int strcmp(char const * s1, char const * s2);
int strcoll(char const * s1, char const * s2);
char * strcpy(char * dest, char const * src);
size_t strcspn(char const * s1, char const * s2);
char * strdup(char const * s);
char * strerror(int errnum);
int strerror_r(int errnum, char * strerrbuf, size_t buflen);
size_t strlen(char const * s);
char * strncat(char * dest, char const * src, size_t n);
int strncmp(char const * s1, char const * s2, size_t n);
char * strncpy(char * dest, char const * src, size_t n);
size_t strnlen(char const * s, size_t max);
char * strpbrk(char const * s1, char const * s2);
char * strrchr(char const * s, int c);
size_t strspn(char const * s1, char const * s2);
char * strstr(char const * s1, char const * s2);
char * strtok(char * s1, char const * s2);
size_t strxfrm(char * s1, char const * s2, size_t n);

#endif /* !LIBC_STRING_H */
