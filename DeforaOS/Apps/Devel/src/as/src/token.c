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



#include <assert.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include "token.h"


/* AToken */
/* variables */
#ifdef DEBUG
char * sATokenCode[ATC_NULL] = {
	"ATC_COLON",
	"ATC_COMMA",
	"ATC_DOT",
	"ATC_EOF",
	"ATC_IMMEDIATE",
	"ATC_NEWLINE",
	"ATC_NUMBER",
	"ATC_REGISTER",
	"ATC_SPACE",
	"ATC_TAB",
	"ATC_WORD"
};
#endif
ATokenCode TS_FUNCTION[] = { ATC_WORD, ATC_NULL };
ATokenCode TS_INSTRUCTION[] = { ATC_WORD, ATC_NULL };
ATokenCode TS_INSTRUCTION_LIST[] = { ATC_WORD, ATC_NEWLINE, ATC_SPACE, ATC_NULL };
ATokenCode TS_NEWLINE[] = { ATC_SPACE, ATC_TAB, ATC_NEWLINE, ATC_NULL };
ATokenCode TS_NEWLINE_LIST[] = { ATC_SPACE, ATC_TAB, ATC_NEWLINE, ATC_NULL };
ATokenCode TS_OPERAND_LIST[] = { ATC_WORD, ATC_NUMBER, ATC_IMMEDIATE, ATC_REGISTER, ATC_NULL };
ATokenCode TS_OPERATOR[] = { ATC_WORD, ATC_NULL };
ATokenCode TS_SECTION[] = { ATC_DOT, ATC_NULL };
ATokenCode TS_SECTION_LIST[] = { ATC_DOT, ATC_NULL };
ATokenCode TS_SPACE[] = { ATC_SPACE, ATC_TAB, ATC_NULL };


/* functions */
/* atoken_new */
AToken * atoken_new(ATokenCode code, char * string)
{
	AToken * t;

	if((t = malloc(sizeof(*t))) == NULL)
		return NULL;
	t->code = code;
	t->string = NULL;
	if(string != NULL)
		if((t->string = strdup(string)) == NULL)
		{
			free(t);
			return NULL;
		}
#ifdef DEBUG
	fprintf(stderr, "%s%s%s%s%s", "atoken_new(", sATokenCode[code],
			", \"", string == NULL ? "NULL" : string, "\")\n");
#endif
	return t;
}

void atoken_delete(AToken * t)
{
	assert(t != NULL);
	free(t->string);
	free(t);
}


int atoken_in_set(AToken * t, ATokenSet ts)
{
	if(t == NULL)
		return 0;
	for(; *ts != ATC_NULL && t->code != *ts; ts++);
	return *ts != ATC_NULL;
}
