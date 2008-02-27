/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <System.h>
#include "cpp.h"


/* Cpp */
/* private */
/* types */
typedef struct _CppOperator
{
	CppCode code;
	char const * string;
} CppOperator;

typedef struct _CppInclude
{
	char * filename;
	FILE * fp;
} CppInclude;

struct _Cpp
{
	int filters;
	Parser * parser;
	/* for cpp_filter_includes */
	CppInclude * includes;
	size_t includes_cnt;
	/* for cpp_filter_newlines */
	int newlines_last;
	int newlines_last_cnt;
	/* for cpp_filter_trigraphs */
	int trigraphs_last;
	int trigraphs_last_cnt;
	/* for cpp_callback_directive */
	int directive_newline;
};


/* variables */
static const CppOperator _cpp_operators[] =
{
	{ CPP_CODE_OPERATOR_AEQUALS,	"&="	},
	{ CPP_CODE_OPERATOR_DAMPERSAND,	"&&"	},
	{ CPP_CODE_OPERATOR_AMPERSAND,	"&"	},
	{ CPP_CODE_OPERATOR_COLON,	":"	},
	{ CPP_CODE_OPERATOR_BEQUALS,	"|="	},
	{ CPP_CODE_OPERATOR_DBAR,	"||"	},
	{ CPP_CODE_OPERATOR_BAR,	"|"	},
	{ CPP_CODE_OPERATOR_DIVIDE,	"/"	},
	{ CPP_CODE_OPERATOR_DOT,	"."	},
	{ CPP_CODE_OPERATOR_DEQUALS,	"=="	},
	{ CPP_CODE_OPERATOR_EQUALS,	"="	},
	{ CPP_CODE_OPERATOR_DGREATER,	">>"	},
	{ CPP_CODE_OPERATOR_GREATER,	">"	},
	{ CPP_CODE_OPERATOR_INVERSE,	"~"	},
	{ CPP_CODE_OPERATOR_LBRACE,	"{"	},
	{ CPP_CODE_OPERATOR_LBRACKET,	"["	},
	{ CPP_CODE_OPERATOR_DLESS,	"<<"	},
	{ CPP_CODE_OPERATOR_LESS,	"<"	},
	{ CPP_CODE_OPERATOR_LPAREN,	"("	},
	{ CPP_CODE_OPERATOR_MGREATER,	"->"	},
	{ CPP_CODE_OPERATOR_DMINUS,	"--"	},
	{ CPP_CODE_OPERATOR_MEQUALS,	"-="	},
	{ CPP_CODE_OPERATOR_MINUS,	"-"	},
	{ CPP_CODE_OPERATOR_MODULO,	"%"	},
	{ CPP_CODE_OPERATOR_MULTIPLY,	"*"	},
	{ CPP_CODE_OPERATOR_NOT,	"!"	},
	{ CPP_CODE_OPERATOR_DPLUS,	"++"	},
	{ CPP_CODE_OPERATOR_PEQUALS,	"+="	},
	{ CPP_CODE_OPERATOR_PLUS,	"+"	},
	{ CPP_CODE_OPERATOR_RBRACE,	"}"	},
	{ CPP_CODE_OPERATOR_RBRACKET,	"]"	},
	{ CPP_CODE_OPERATOR_RPAREN,	")"	},
	{ CPP_CODE_OPERATOR_SEMICOLON,	";"	},
	{ CPP_CODE_OPERATOR_XOR,	"^"	}
};
static const size_t _cpp_operators_cnt = sizeof(_cpp_operators)
	/ sizeof(*_cpp_operators);


/* prototypes */
/* useful */
/* filters */
static int _cpp_filter_includes(int * c, void * data);
static int _cpp_filter_newlines(int * c, void * data);
static int _cpp_filter_trigraphs(int * c, void * data);

/* callbacks */
static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_whitespace(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_comment(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_comma(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_operator(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_quote(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data);



/* Cpp */
/* private */
/* cpp_filter_includes */
static int _cpp_filter_includes(int * c, void * data)
	/* FIXME should be a wrapper around parser_scan() instead */
{
	Cpp * cpp = data;
	CppInclude * include;
	CppInclude * p;

	while(cpp->includes_cnt > 0)
	{
		include = &cpp->includes[cpp->includes_cnt - 1];
		if((*c = fgetc(include->fp)) != EOF)
			return 1;
		if(!feof(include->fp)) /* an error occured */
		{
			error_set_code(1, "%s: %s", include->filename, strerror(
						errno));
			fclose(include->fp);
			return -1;
		}
		if((p = realloc(cpp->includes, sizeof(*p)
						* cpp->includes_cnt--)) == NULL)
		{
			error_set_code(1, "%s: %s", include->filename, strerror(
						errno));
			return -1;
		}
	}
	return 0;
}


/* cpp_filter_newlines */
static int _cpp_filter_newlines(int * c, void * data)
{
	Cpp * cpp = data;

	if(cpp->newlines_last_cnt != 0)
	{
		cpp->newlines_last_cnt--;
		*c = cpp->newlines_last;
		return 0;
	}
	if(*c != '\\')
		return 0;
	if((*c = parser_scan(cpp->parser)) == '\n')
	{
		*c = parser_scan(cpp->parser); /* skip the newline */
		return 0;
	}
	cpp->newlines_last = *c;
	cpp->newlines_last_cnt = 1;
	*c = '\\';
	return 1;
}


/* cpp_filter_trigraphs */
static int _trigraphs_get(int last, int * c);

static int _cpp_filter_trigraphs(int * c, void * data)
{
	Cpp * cpp = data;

	if(cpp->trigraphs_last_cnt == 2)
	{
		cpp->trigraphs_last_cnt--;
		*c = '?';
		return 0;
	}
	else if(cpp->trigraphs_last_cnt == 1)
	{
		cpp->trigraphs_last_cnt--;
		*c = cpp->trigraphs_last;
		return 0;
	}
	if(*c != '?')
		return 0;
	if((cpp->trigraphs_last = parser_scan(cpp->parser)) != '?')
	{
		cpp->trigraphs_last_cnt = 1;
		return 1;
	}
	cpp->trigraphs_last = parser_scan(cpp->parser);
	if(_trigraphs_get(cpp->trigraphs_last, c) != 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: last=%c\n", cpp->trigraphs_last);
#endif
		cpp->trigraphs_last_cnt = 2;
		return 2;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: filtered \"??%c\" into \"%c\"\n",
			cpp->trigraphs_last, *c);
#endif
	return 0;
}

static int _trigraphs_get(int last, int * c)
{
	switch(last)
	{
		case '=':
			*c = '#';
			break;
		case '/':
			*c = '\\';
			break;
		case '\'':
			*c = '^';
			break;
		case '(':
			*c = '[';
			break;
		case ')':
			*c = ']';
			break;
		case '!':
			*c = '|';
			break;
		case '<':
			*c = '{';
			break;
		case '>':
			*c = '}';
			break;
		case '-':
			*c = '~';
			break;
		default:
			return 1;
	}
	return 0;
}


/* cpp_callback_whitespace */
static int _cpp_callback_whitespace(Parser * parser, Token * token, int c,
		void * data)
{
	Cpp * cpp = data;
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(!isspace(c))
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: cpp_callback_whitespace()\n");
#endif
	do
	{
		if(c != '\n')
			continue;
		if((p = realloc(str, len + 1)) == NULL)
		{
			free(str);
			return -1;
		}
		str = p;
		str[len++] = '\n';
	}
	while(isspace((c = parser_scan_filter(parser))));
	token_set_code(token, CPP_CODE_WHITESPACE);
	if(str != NULL)
	{
		str[len] = '\0';
		token_set_string(token, str);
		free(str);
		cpp->directive_newline = 1;
	}
	else
		token_set_string(token, " ");
	return 0;
}


/* cpp_callback_comment */
static int _cpp_callback_comment(Parser * parser, Token * token, int c,
		void * data)
{
	if(c != '/')
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: cpp_callback_comment()\n");
#endif
	if((c = parser_scan_filter(parser)) != '*')
	{
		token_set_code(token, CPP_CODE_OPERATOR_DIVIDE);
		token_set_string(token, "/");
		return 0;
	}
	for(c = parser_scan_filter(parser); c != EOF;)
		if(c == '*')
		{
			if((c = parser_scan_filter(parser)) == '/')
				break;
		}
		else
			c = parser_scan_filter(parser);
	if(c == EOF)
		return -1;
	token_set_code(token, CPP_CODE_WHITESPACE);
	token_set_string(token, " ");
	parser_scan_filter(parser);
	return 0;
}


/* cpp_callback_directive */
static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data)
	/* FIXME actually parse and implement, careful with comments */
{
	Cpp * cpp = data;
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(cpp->directive_newline != 1 || c != '#')
	{
		cpp->directive_newline = 0;
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: cpp_callback_directive()\n");
#endif
	do
	{
		if((p = realloc(str, len + 1)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			free(str);
			return -1;
		}
		str = p;
		str[len++] = c;
	}
	while((c = parser_scan_filter(parser)) != '\n');
	str[len] = '\0';
	token_set_code(token, CPP_CODE_META);
	token_set_string(token, str);
	free(str);
	return 0;
}


/* cpp_callback_comma */
static int _cpp_callback_comma(Parser * parser, Token * token, int c,
		void * data)
{
	if(c != ',')
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: cpp_callback_comma()\n");
#endif
	token_set_code(token, CPP_CODE_COMMA);
	token_set_string(token, ",");
	parser_scan_filter(parser);
	return 0;
}


/* cpp_callback_operator */
static int _cpp_callback_operator(Parser * parser, Token * token, int c,
		void * data)
{
	size_t i;
	const size_t j = sizeof(_cpp_operators) / sizeof(*_cpp_operators);
	size_t pos;

	for(i = 0; i < _cpp_operators_cnt; i++)
		if(_cpp_operators[i].string[0] == c)
			break;
	if(i == _cpp_operators_cnt) /* nothing found */
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s%c%s", "DEBUG: cpp_callback_operator('", c, "')\n");
#endif
	for(pos = 0; i < j;)
	{
		if(_cpp_operators[i].string[pos] == '\0')
			break;
		if(c == _cpp_operators[i].string[pos])
		{
			c = parser_scan_filter(parser);
			pos++;
		}
		else
			i++;
	}
	if(i == j) /* should not happen */
		return -1;
	token_set_code(token, _cpp_operators[i].code);
	token_set_string(token, _cpp_operators[i].string);
	return 0;
}


/* cpp_callback_quote */
static int _cpp_callback_quote(Parser * parser, Token * token, int c,
		void * data)
{
	int escape = 0;
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(c == '\'')
		token_set_code(token, CPP_CODE_SQUOTE);
	else if(c == '"')
		token_set_code(token, CPP_CODE_DQUOTE);
	else
		return 1;
#ifdef DEBUG
	fprintf(stderr, "%s%c%s", "DEBUG: cpp_callback_quote('", c, "')\n");
#endif
	while((p = realloc(str, len + 3)) != NULL)
	{
		str = p;
		str[len++] = c;
		if((c = parser_scan_filter(parser)) == EOF || c == '\n')
			break;
		if(escape)
		{
			escape = 0;
			continue;
		}
		if(c == str[0])
			break;
		if(c == '\\')
			escape = 1;
	}
	if(str == NULL || c != str[0])
	{
		free(str);
		return -1;
	}
	str[len++] = str[0];
	str[len] = '\0';
	token_set_string(token, str);
	free(str);
	parser_scan_filter(parser);
	return 0;
}


/* cpp_callback_word */
static int _isword(int c);

static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data)
{
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(!_isword(c))
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: cpp_callback_word('%c')\n", c);
#endif
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			free(str);
			return 0;
		}
		str = p;
		str[len++] = c;
	}
	while(_isword((c = parser_scan_filter(parser))));
	str[len] = '\0';
	token_set_code(token, CPP_CODE_WORD);
	token_set_string(token, str);
	free(str);
	return 0;
}

static int _isword(int c)
{
	return isalnum(c) || c == '_';
}


/* cpp_callback_unknown */
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data)
{
	static char buf[2] = { '\0', '\0' };

	if(c == EOF)
		return 1;
	buf[0] = c;
	parser_scan(parser);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: cpp_callback_unknown('%c' 0x%x)\n", c, c);
#endif
	token_set_code(token, CPP_CODE_UNKNOWN);
	token_set_string(token, buf);
	return 0;
}


/* public */
/* functions */
/* cpp_new */
Cpp * cpp_new(char const * filename, int filters)
{
	Cpp * cpp;

	if((cpp = object_new(sizeof(*cpp))) == NULL)
		return NULL;
	cpp->filters = 0;
	cpp->parser = parser_new(filename);
	cpp->includes = NULL;
	cpp->includes_cnt = 0;
	cpp->newlines_last_cnt = 0;
	cpp->trigraphs_last_cnt = 0;
	cpp->directive_newline = 1;
	if(cpp->parser == NULL)
	{
		cpp_delete(cpp);
		return NULL;
	}
	parser_add_filter(cpp->parser, _cpp_filter_includes, cpp);
	parser_add_filter(cpp->parser, _cpp_filter_newlines, cpp);
	if(filters & CPP_FILTER_TRIGRAPH)
		parser_add_filter(cpp->parser, _cpp_filter_trigraphs, cpp);
	parser_add_callback(cpp->parser, _cpp_callback_whitespace, cpp);
	parser_add_callback(cpp->parser, _cpp_callback_comment, NULL);
	parser_add_callback(cpp->parser, _cpp_callback_directive, cpp);
	parser_add_callback(cpp->parser, _cpp_callback_comma, NULL);
	parser_add_callback(cpp->parser, _cpp_callback_operator, NULL);
	parser_add_callback(cpp->parser, _cpp_callback_quote, NULL);
	parser_add_callback(cpp->parser, _cpp_callback_word, NULL);
	parser_add_callback(cpp->parser, _cpp_callback_unknown, NULL);
	return cpp;
}


/* cpp_delete */
void cpp_delete(Cpp * cpp)
{
	while(cpp->includes_cnt-- > 0)
	{
		fclose(cpp->includes[cpp->includes_cnt].fp);
		free(cpp->includes[cpp->includes_cnt].filename);
	}
	free(cpp->includes);
	parser_delete(cpp->parser);
	object_delete(cpp);
}


/* accessors */
/* cpp_get_filename */
char const * cpp_get_filename(Cpp * cpp)
{
	if(cpp->includes_cnt > 0)
		return cpp->includes[cpp->includes_cnt - 1].filename;
	return parser_get_filename(cpp->parser);
}


/* useful */
/* cpp_scan */
int cpp_scan(Cpp * cpp, Token ** token)
{
	return parser_get_token(cpp->parser, token);
}
