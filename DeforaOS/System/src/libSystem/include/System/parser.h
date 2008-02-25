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



#ifndef LIBSYSTEM_PARSER_H
# define LIBSYSTEM_PARSER_H

# include "token.h"


/* Parser */
/* types */
typedef struct _Parser Parser;
typedef int (*ParserFilter)(int * c, void * data);
typedef int (*ParserCallback)(Parser * parser, Token * token, int c,
		void * data);


/* functions */
Parser * parser_new(char const * pathname);
int parser_delete(Parser * parser);

/* accessors */
char const * parser_get_filename(Parser * parser);
int parser_get_token(Parser * parser, Token ** token);

/* useful */
int parser_add_callback(Parser * parser, ParserCallback callback,
		void * data);
int parser_delete_callback(Parser * parser, ParserCallback callback);

int parser_add_filter(Parser * parser, ParserFilter filter, void * data);
int parser_delete_filter(Parser * parser, ParserFilter filter);

int parser_scan(Parser * parser);
int parser_scan_filter(Parser * parser);

#endif /* !LIBSYSTEM_PARSER_H */
