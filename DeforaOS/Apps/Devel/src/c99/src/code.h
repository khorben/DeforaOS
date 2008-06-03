/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef _C99_CODE_H
# define _C99_CODE_H

# include "c99.h"


/* Code */
/* protected */
typedef struct _Code Code;


/* public */
/* functions */
Code * code_new(C99Prefs const * prefs, char const * outfile);
int code_delete(Code * code);

/* useful */
/* functions */
int code_function_begin(Code * code, char const * name);
int code_function_call(Code * code, char const * name);
int code_function_end(Code * code);

/* types */
int code_type_add(Code * code, char const * name);
int code_type_get(Code * code, char const * name);

/* variables */
int code_variable_add(Code * code, char const * name);
int code_variable_get(Code * code, char const * name);

#endif /* !_C99_CODE_H */
