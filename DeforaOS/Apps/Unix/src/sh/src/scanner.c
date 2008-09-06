/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scanner.h"


/* Scanner */
/* scanner_init */
static void _prompt(Scanner * scanner);
static int _next_file_prompt(Scanner * scanner);
static int _next_file(Scanner * scanner);
static int _next_string(Scanner * scanner);

void scanner_init(Scanner * scanner, Prefs * prefs, FILE * fp,
		char const * string)
{
	if((scanner->fp = fp) == NULL)
		scanner->next = _next_string;
	else if(*prefs & PREFS_i)
		scanner->next = _next_file_prompt;
	else
		scanner->next = _next_file;
	scanner->prompt = SP_PS1;
	scanner->string = string;
}

static void _prompt(Scanner * scanner)
{
	char * prompt;
	char * env[SP_COUNT] = { "PS1", "PS2", "PS4" };
	char * dflt[SP_COUNT] = { "$ ", "> ", "+ " };

	if((prompt = getenv(env[scanner->prompt])) == NULL)
	{
		if(scanner->prompt == SP_PS1 && getuid() == 0)
			prompt = "# ";
		else
			prompt = dflt[scanner->prompt];
	}
	fputs(prompt, stderr);
}

static void _lineno(void)
{
	static unsigned int i = 1;
	char lineno[11];
	const int len = sizeof(lineno) - 1;

	if(snprintf(lineno, len, "%u", i++) >= len)
		lineno[len] = '\0';
	if(setenv("LINENO", lineno, 1) != 0)
		sh_error("setenv", 0);
}

static int _next_file_prompt(Scanner * scanner)
{
	static int c = '\n';

	if(c == '\n')
	{
		_prompt(scanner);
		_lineno();
	}
	if((c = fgetc(scanner->fp)) == EOF)
		fputc('\n', stderr);
	return c;
}

static int _next_file(Scanner * scanner)
{
	int c;

	if((c = fgetc(scanner->fp)) == '\n')
		_lineno();
	return c;
}

static int _next_string(Scanner * scanner)
{
	static unsigned int pos = 0;

	if(scanner->string[pos] == '\0')
		return EOF;
	return scanner->string[pos++];
}


/* scanner_next */
static Token * _next_operator(Scanner * scanner, int * c);
static Token * _next_blank(Scanner * scanner, int * c);
static Token * _next_comment(Scanner * scanner, int * c);
static Token * _next_newline(int * c);
static Token * _next_word(Scanner * scanner, int * c);

Token * scanner_next(Scanner * scanner)
{
	static int c = EOF;
	Token * t;

#ifdef DEBUG
	fprintf(stderr, "%s", "scanner_next()\n");
#endif
	scanner->prompt = SP_PS1;
	if(c == EOF && (c = scanner->next(scanner)) == EOF)
		return token_new(TC_EOI, NULL);
	scanner->prompt = SP_PS2;
	_next_blank(scanner, &c);
	_next_comment(scanner, &c);
	if((t = _next_operator(scanner, &c)) != NULL
			|| (t = _next_newline(&c)) != NULL
			|| (t = _next_word(scanner, &c)) != NULL)
		return t;
	return NULL;
}

static Token * _next_operator(Scanner * scanner, int * c)
{
	unsigned int i;
	unsigned int pos = 0;

	for(i = TC_OP_AND_IF; i <= TC_OP_GREAT;)
	{
		if(sTokenCode[i][pos] == '\0')
			return token_new(i, NULL);
		if(*c == sTokenCode[i][pos])
		{
			*c = scanner->next(scanner);
			pos++;
		}
		else
			i++;
	}
	return NULL;
}

static Token * _next_blank(Scanner * scanner, int * c)
{
	while(*c == '\t' || *c == ' ')
		*c = scanner->next(scanner);
	return NULL;
}

static Token * _next_comment(Scanner * scanner, int * c)
{
	if(*c != '#')
		return NULL;
	*c = scanner->next(scanner);
	while(*c != EOF && *c != '\0' && *c != '\r' && *c != '\n')
		*c = scanner->next(scanner);
	return NULL;
}

static Token * _next_newline(int * c)
{
	if(*c != '\r' && *c != '\n')
		return NULL;
	*c = EOF;
	return token_new(TC_NEWLINE, NULL);
}

static char _word_escape(Scanner * scanner, int * c);
static char * _word_dquote(Scanner * scanner, int * c, char * str,
		unsigned int * len);
static char * _word_quote(Scanner * scanner, int * c, char * str,
		unsigned int * len);
static Token * _next_word(Scanner * scanner, int * c)
{
	char eow[] = "\t \r\n&|;<>";
	char * str = NULL;
	unsigned int len = 0;
	unsigned int i;
	char * p;

	for(; *c != EOF; *c = scanner->next(scanner))
	{
		for(i = 0; eow[i] != '\0' && *c != eow[i]; i++);
		if(*c == eow[i])
			break;
		if(*c == '"')
			p = _word_dquote(scanner, c, str, &len);
		else if(*c == '\'')
			p = _word_quote(scanner, c, str, &len);
		else
		{
			if(*c == '\\' && (*c = _word_escape(scanner, c)) == EOF)
				break;
			if((p = realloc(str, len + 2)) != NULL)
			{
				str = p;
				str[len++] = *c;
				continue;
			}
		}
		if(p == NULL)
		{
			sh_error("malloc", 0);
			free(str);
			return NULL;
		}
		str = p;
	}
	if(str == NULL)
		return NULL;
	str[len] = '\0';
	/* FIXME aliases need to be replaced now; I suggest changing the
	 * scanner->next() function for a while but I don't really like it,
	 * moreover it wouldn't parse the alias into tokens... */
	return token_new(TC_TOKEN, str);
}

static char _word_escape(Scanner * scanner, int * c)
{
	char p;

	if((p = scanner->next(scanner)) == EOF)
		return *c;
	return p;
}

static char const * _read_variable(Scanner * scanner, int * c);
static char * _word_dquote(Scanner * scanner, int * c, char * str,
		unsigned int * len)
{
	char * p;
	char const * var;

	if(str == NULL && (str = malloc(1)) == NULL)
		return NULL;
	for(*c = scanner->next(scanner); *c != EOF && *c != '"';
			*c = scanner->next(scanner))
	{
		if(*c == '\\')
			*c = _word_escape(scanner, c);
		else if(*c == '$')
		{
			if((var = _read_variable(scanner, c)) == NULL)
				return NULL;
			if((p = realloc(str, (*len)+strlen(var)+1)) == NULL)
				return NULL;
			str = p;
			strcpy(&str[(*len)], var);
			(*len)+=strlen(var);
			if(*c == '"')
				break;
		}
		if((p = realloc(str, (*len)+2)) == NULL)
			return NULL;
		str = p;
		str[(*len)++] = *c;
	}
	return str;
}

static char const * _read_variable(Scanner * scanner, int * c)
{
	char buf[80];
	unsigned long i;
	char * p;
	char delim = '\0';

	for(i = 0; i < sizeof(buf)-1; i++)
	{
		*c = scanner->next(scanner);
		if(i == 0 && *c == '{')
		{
			delim = '}';
			i--;
			continue;
		}
		if(i != 0 && delim != '\0' && *c == delim)
		{
			*c = scanner->next(scanner);
			break;
		}
		if(*c == EOF || (!isalnum(*c) && *c != '_'))
			break;
		buf[i] = *c;
	}
	if(i == sizeof(buf)-1)
		return NULL;
	if(i == 0)
		return "$";
	buf[i] = '\0';
	if((p = getenv(buf)) == NULL)
		return "";
	return p;
}

static char * _word_quote(Scanner * scanner, int * c, char * str,
		unsigned int * len)
{
	char * p;

	if(str == NULL && (str = malloc(1)) == NULL)
		return NULL;
	for(*c = scanner->next(scanner); *c != EOF && *c != '\'';
			*c = scanner->next(scanner))
	{
		if((p = realloc(str, (*len)+2)) == NULL)
			return NULL;
		str = p;
		str[(*len)++] = *c;
	}
	return str;
}
