/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef AS_TOKEN_H
# define AS_TOKEN_H


/* types */
typedef enum _ATokenCode
{
	ATC_COLON = 0,
	ATC_COMMA,
	ATC_DOT,
	ATC_EOF,
	ATC_IMMEDIATE,
	ATC_NEWLINE,
	ATC_NUMBER,
	ATC_REGISTER,
	ATC_SPACE,
	ATC_TAB,
	ATC_WORD,
	ATC_NULL
} ATokenCode;
#ifdef DEBUG
extern char * sATokenCode[ATC_NULL];
#endif

typedef ATokenCode * ATokenSet;
extern ATokenCode TS_FUNCTION[];
extern ATokenCode TS_INSTRUCTION[];
extern ATokenCode TS_INSTRUCTION_LIST[];
extern ATokenCode TS_NEWLINE[];
extern ATokenCode TS_OPERAND[];
extern ATokenCode TS_OPERAND_LIST[];
extern ATokenCode TS_OPERATOR[];
extern ATokenCode TS_SECTION[];
extern ATokenCode TS_SECTION_LIST[];
extern ATokenCode TS_SPACE[];

typedef struct _AToken
{
	char * string;
	ATokenCode code;
} AToken;


/* AToken */
AToken * atoken_new(ATokenCode code, char * string);
void atoken_delete(AToken * token);

int atoken_in_set(AToken * token, ATokenSet set);

#endif /* !AS_TOKEN_H */
