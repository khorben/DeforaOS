/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef SH_SH_H
# define SH_SH_H


/* types */
/* Prefs */
typedef int Prefs;
#define PREFS_c 0x1
#define PREFS_i 0x2
#define PREFS_s 0x4


/* variables */
extern char ** export;


/* functions */
int sh_error(char * message, int ret);
char ** sh_export(void);


/* debugging */
# ifdef DEBUG
#  define malloc(a) dbg_malloc(a, __FILE__, __LINE__)
#  define realloc(a, b) dbg_realloc(a, b, __FILE__, __LINE__)
#  define free(a) dbg_free(a, __FILE__, __LINE__)
void * dbg_malloc(size_t size, char * file, int line);
void * dbg_realloc(void * ptr, size_t size, char * file, int line);
void dbg_free(void * ptr, char * file, int line);
# endif /* DEBUG */

#endif /* !SH_SH_H */
