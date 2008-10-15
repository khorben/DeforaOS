/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#ifndef LIBSYSTEM_TOKEN_H
# define LIBSYSTEM_TOKEN_H

# include "string.h"


/* Token */
/* types */
typedef struct _Token Token;
typedef unsigned int TokenCode;
# define TC_NULL 0
typedef TokenCode * TokenSet;


/* functions */
void token_delete(Token * token);


/* accessors */
String const * token_get_filename(Token * token);
int token_set_filename(Token * token, String const * filename);

unsigned int token_get_col(Token * token);
void token_set_col(Token * token, unsigned int col);

unsigned int token_get_line(Token * token);
void token_set_line(Token * token, unsigned int line);

TokenCode token_get_code(Token * token);
void token_set_code(Token * token, int code);

String const * token_get_string(Token * token);
int token_set_string(Token * token, String const * string);

void * token_get_data(Token * token);
void token_set_data(Token * token, void * data);


/* useful */
int token_in_set(Token * token, TokenSet set);

#endif /* !LIBSYSTEM_TOKEN_H */
