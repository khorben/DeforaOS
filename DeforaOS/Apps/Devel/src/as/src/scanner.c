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



#include <System.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "as.h"
#include "token.h"


/* scan */
static AToken * _scan_colon(FILE * fp, int * la);
static AToken * _scan_comma(FILE * fp, int * la);
static AToken * _scan_comment(FILE * fp, int * la);
static AToken * _scan_dot(FILE * fp, int * la);
static AToken * _scan_eof(int * la);
static AToken * _scan_immediate(FILE * fp, int * la);
static AToken * _scan_newline(FILE * fp, int * la);
static AToken * _scan_number(FILE * fp, int * la);
static AToken * _scan_register(FILE * fp, int * la);
static AToken * _scan_space(FILE * fp, int * la);
static AToken * _scan_word(FILE * fp, int * la);
AToken * scan(FILE * fp)
{
	AToken * t;
	static int la = EOF;

	if(la == EOF)
		la = fgetc(fp);
	if((t = _scan_colon(fp, &la))
			|| (t = _scan_comma(fp, &la))
			|| (t = _scan_comment(fp, &la))
			|| (t = _scan_dot(fp, &la))
			|| (t = _scan_eof(&la))
			|| (t = _scan_immediate(fp, &la))
			|| (t = _scan_newline(fp, &la))
			|| (t = _scan_number(fp, &la))
			|| (t = _scan_register(fp, &la))
			|| (t = _scan_space(fp, &la))
			|| (t = _scan_word(fp, &la)))
		return t;
	return NULL;
}

static AToken * _scan_colon(FILE * fp, int * la)
{
	if(*la == ':')
	{
		*la = fgetc(fp);
		return atoken_new(ATC_COLON, ":");
	}
	return NULL;
}

static AToken * _scan_comma(FILE * fp, int * la)
{
	if(*la == ',')
	{
		*la = fgetc(fp);
		return atoken_new(ATC_COMMA, ",");
	}
	return NULL;
}

static AToken * _scan_comment(FILE * fp, int * la)
{
	int c;

	if(*la != '/')
		return NULL;
	if((c = fgetc(fp)) != '/')
	{
		fseek(fp, -1, SEEK_CUR);
		return NULL;
	}
	for(;;)
	{
		c = fgetc(fp);
		switch(c)
		{
			case '\r':
			case '\n':
			case EOF:
				*la = c;
				return atoken_new(ATC_SPACE, "//");
		}
	}
}

static AToken * _scan_dot(FILE * fp, int * la)
{
	if(*la == '.')
	{
		*la = fgetc(fp);
		return atoken_new(ATC_DOT, ".");
	}
	return NULL;
}

static AToken * _scan_eof(int * la)
{
	if(*la == EOF)
		return atoken_new(ATC_EOF, "EOF");
	return NULL;
}

static AToken * _scan_immediate(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 1;
	char * p;
	AToken * t;
	int hex = 0;

	if(*la != '$' || !isdigit(*la = fgetc(fp)) || (str = malloc(1)) == NULL)
		return NULL;
	str[0] = '$';
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
		if(len == 2 && str[1] == '0' && *la == 'x')
			hex = 1;
	}
	while(isdigit(*la) || (len == 2 && hex) || (hex && (tolower(*la) >= 'a'
					&& tolower(*la) <= 'f')));
	str[len] = '\0';
	t = atoken_new(ATC_IMMEDIATE, str);
	free(str);
	return t;
}

static AToken * _scan_newline(FILE * fp, int * la)
{
	if(*la == '\n' || *la == '\r') /* FIXME '\r\n' */
	{
		*la = fgetc(fp);
		return atoken_new(ATC_NEWLINE, "\n");
	}
	return NULL;
}

static AToken * _scan_number(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 0;
	char * p;
	AToken * t;

	if(!isdigit(*la))
		return NULL;
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(isdigit(*la));
	str[len] = '\0';
	t = atoken_new(ATC_NUMBER, str);
	free(str);
	return t;
}

static AToken * _scan_register(FILE * fp, int * la)
{
	char * str = NULL;
	int len = 1;
	char * p;
	AToken * t;

	if(*la != '%' || !islower(*la = fgetc(fp)) || (str = malloc(1)) == NULL)
		return NULL;
	str[0] = '%';
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			free(str);
			return NULL; /* FIXME report error */
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(islower(*la));
	str[len] = '\0';
	t = atoken_new(ATC_REGISTER, str);
	free(str);
	return t;
}

static AToken * _scan_space(FILE * fp, int * la)
{
	if(isspace(*la))
	{
		for(; isspace(*la); *la = fgetc(fp));
		return atoken_new(ATC_SPACE, " ");
	}
	return NULL;
}

static AToken * _scan_word(FILE * fp, int * la)
	/* FIXME */
{
	char * str = NULL;
	int len = 0;
	char * p;
	AToken * t;

	if(!islower(*la))
		return NULL;
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			free(str);
			return NULL; /* FIXME report error */
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(islower(*la) || isdigit(*la));
	str[len] = '\0';
	t = atoken_new(ATC_WORD, str);
	free(str);
	return t;
}


AToken * check(FILE * fp, ATokenCode code)
{
	AToken * t;

	if((t = scan(fp)) == NULL)
		return 0;
	if(t->code == code)
		return t;
	atoken_delete(t);
	return NULL;
}
