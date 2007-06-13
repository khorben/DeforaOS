/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
/* inetd is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * inetd is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * inetd; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef __TOKEN_H
# define __TOKEN_H


/* types */
typedef enum _TokenCode
{
	TC_EOF,
	TC_NEWLINE,
	TC_NUMBER,
	TC_SPACE,
	TC_TAB,
	TC_WORD,
	TC_NULL
} TokenCode;
#ifdef DEBUG
extern char * sTokenCode[TC_NULL];
#endif

typedef TokenCode * TokenSet;
extern TokenCode TS_NEWLINE[];
extern TokenCode TS_PROGRAM_ARGUMENT[];
extern TokenCode TS_SERVICE[];
extern TokenCode TS_SPACE[];

typedef struct _Token
{
	char * string;
	TokenCode code;
} Token;


/* functions */
Token * token_new(TokenCode code, char * string);
void token_delete(Token * token);

int token_in_set(Token * token, TokenSet set);

#endif /* !__TOKEN_H */
