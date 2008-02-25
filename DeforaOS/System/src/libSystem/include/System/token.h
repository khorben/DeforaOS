/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef LIBSYSTEM_TOKEN_H
# define LIBSYSTEM_TOKEN_H


/* Token */
/* types */
typedef struct _Token Token;
typedef unsigned int TokenCode;
# define TC_NULL 0
typedef TokenCode * TokenSet;


/* functions */
void token_delete(Token * token);


/* accessors */
unsigned int token_get_col(Token * token);
void token_set_col(Token * token, unsigned int col);

unsigned int token_get_line(Token * token);
void token_set_line(Token * token, unsigned int line);

int token_get_code(Token * token);
void token_set_code(Token * token, int code);

char const * token_get_string(Token * token);
int token_set_string(Token * token, char const * string);


/* useful */
int token_in_set(Token * token, TokenSet set);

#endif /* !LIBSYSTEM_TOKEN_H */
