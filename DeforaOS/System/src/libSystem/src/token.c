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



#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include "System.h"
#include "token.h"


/* Token */
/* private */
/* types */
struct _Token
{
	TokenCode code;
	String * string;
	String * filename;
	unsigned int line;
	unsigned int col;
	void * data;
};


/* protected */
/* functions */
/* token_new
 * PRE	filename must be non-NULL */
Token * token_new(String const * filename, unsigned int line, unsigned int col)
{
	Token * token;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, %u)\n", __func__, line, col);
#endif
	if((token = object_new(sizeof(*token))) == NULL)
		return NULL;
	token->code = 0;
	token->string = NULL;
	token->filename = string_new(filename);
	token->line = line;
	token->col = col;
	token->data = NULL;
	if(token->filename == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		object_delete(token);
		return NULL;
	}
	return token;
}


/* public */
/* functions */
void token_delete(Token * token)
{
	string_delete(token->filename);
	string_delete(token->string);
	object_delete(token);
}


/* accessors */
/* token_get_code */
TokenCode token_get_code(Token * token)
{
	return token->code;
}


/* token_get_col */
unsigned int token_get_col(Token * token)
{
	return token->col;
}


/* token_get_data */
void * token_get_data(Token * token)
{
	return token->data;
}


/* token_get_filename */
String const * token_get_filename(Token * token)
{
	return token->filename;
}


/* token_get_line */
unsigned int token_get_line(Token * token)
{
	return token->line;
}


/* token_get_string */
String const * token_get_string(Token * token)
{
	return token->string;
}


/* token_set_code */
void token_set_code(Token * token, int code)
{
	token->code = code;
}


/* token_set_col */
void token_set_col(Token * token, unsigned int col)
{
	token->col = col;
}


/* token_set_data */
void token_set_data(Token * token, void * data)
{
	token->data = data;
}


/* token_set_filename */
int token_set_filename(Token * token, String const * filename)
{
	string_delete(token->filename);
	if((token->filename = string_new(filename)) == NULL)
		return 1;
	return 0;
}


/* token_set_line */
void token_set_line(Token * token, unsigned int line)
{
	token->line = line;
}


/* token_set_string */
int token_set_string(Token * token, String const * string)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, token, string);
#endif
	string_delete(token->string);
	if((token->string = string_new(string)) == NULL)
		return 1;
	return 0;
}


/* useful */
int token_in_set(Token * token, TokenSet set)
{
	TokenCode * code;

	for(code = set; *code != TC_NULL; code++)
		if(token->code == *code)
			return 1;
	return 0;
}
