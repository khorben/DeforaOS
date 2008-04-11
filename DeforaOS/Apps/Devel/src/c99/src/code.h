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


/* Code */
/* protected */
typedef struct _Code Code;

typedef enum _CodeContext
{
	CODE_CONTEXT_UNDEFINED = 0,
	CODE_CONTEXT_FUNCTION_NAME
} CodeContext;


/* public */
/* functions */
Code * code_new(char const * target);
void code_delete(Code * code);

/* accessors */
int code_set_context(Code * code, CodeContext context);
int code_set_identifier(Code * code, char const * name);

/* useful */
int code_is_type(Code * code, char const * name);

#endif /* !_C99_CODE_H */
