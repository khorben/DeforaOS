/* scanner.c */



#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "token.h"


/* scan */
static Token * _scan_comment(FILE * fp, int * la);
static Token * _scan_eof(int * la);
static Token * _scan_newline(FILE * fp, int * la);
static Token * _scan_space(FILE * fp, int * la);
static Token * _scan_word(FILE * fp, int * la);
Token * scan(FILE * fp)
{
	Token * t;
	static int la = EOF;

	if(la == EOF)
		la = fgetc(fp);
	if((t = _scan_comment(fp, &la))
			|| (t = _scan_eof(&la))
			|| (t = _scan_newline(fp, &la))
			|| (t = _scan_space(fp, &la))
			|| (t = _scan_word(fp, &la)))
		return t;
	fprintf(stderr, "%s", "inetd: scan: Should not happen...?\n");
	return NULL;
}

static Token * _scan_comment(FILE * fp, int * la)
{
	int c;

	if(*la != '#')
		return NULL;
	for(;;)
	{
		c = fgetc(fp);
		switch(c)
		{
			case '\r':
			case '\n':
			case EOF:
				*la = c;
				return token_new(TC_SPACE, "#");
		}
	}
}

static Token * _scan_eof(int * la)
{
	if(*la == EOF)
		return token_new(TC_EOF, "EOF");
	return NULL;
}

static Token * _scan_newline(FILE * fp, int * la)
{
	if(*la == '\n' || *la == '\r') /* FIXME '\r\n' */
	{
		*la = fgetc(fp);
		return token_new(TC_NEWLINE, "\n");
	}
	return NULL;
}

static Token * _scan_space(FILE * fp, int * la)
{
	if(isspace(*la))
	{
		for(; isspace(*la); *la = fgetc(fp));
		return token_new(TC_SPACE, " ");
	}
	return NULL;
}

static Token * _scan_word(FILE * fp, int * la)
	/* FIXME */
{
	char * str = NULL;
	int len = 0;
	char * p;
	Token * t;

	if(!isalnum(*la) && !ispunct(*la))
		return NULL;
	do
	{
		if((p = realloc(str, len+2)) == NULL)
		{
			free(str);
			return NULL; /* FIXME report error */
		}
		str = p;
		str[len++] = *la;
		*la = fgetc(fp);
	}
	while(isalnum(*la) || ispunct(*la));
	str[len] = '\0';
	t = token_new(TC_WORD, str);
	free(str);
	return t;
}


Token * check(FILE * fp, TokenCode code)
{
	Token * t;

	if((t = scan(fp)) == NULL)
		return 0;
	if(t->code == code)
		return t;
	token_delete(t);
	return NULL;
}
