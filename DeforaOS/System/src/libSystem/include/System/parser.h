/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
Parser * parser_new_string(char const * string, size_t length);
int parser_delete(Parser * parser);

/* accessors */
char const * parser_get_filename(Parser * parser);
int parser_get_token(Parser * parser, Token ** token);

/* useful */
int parser_add_callback(Parser * parser, ParserCallback callback,
		void * data);
int parser_remove_callback(Parser * parser, ParserCallback callback);

int parser_add_filter(Parser * parser, ParserFilter filter, void * data);
int parser_remove_filter(Parser * parser, ParserFilter filter);

int parser_scan(Parser * parser);
int parser_scan_filter(Parser * parser);

#endif /* !LIBSYSTEM_PARSER_H */
