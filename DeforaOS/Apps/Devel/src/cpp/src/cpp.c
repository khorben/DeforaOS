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


/* CppParser */
/* private */
/* types */
typedef struct _CppOperator
{
	CppCode code;
	char const * string;
} CppOperator;

typedef struct _CppParser
{
	Parser * parser;
	const CppOperator * operators;
	size_t operators_cnt;
} CppParser;


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
static CppParser * _cppparser_new(char const * filename, int filters,
		const CppOperator * operators, size_t operators_cnt);
static void _cppparser_delete(CppParser * cppparser);



/* Cpp */
/* private */
/* types */
struct _Cpp
{
	CppParser ** parsers;
	size_t parsers_cnt;
	int filters;
};


/* prototypes */
/* useful */
static int _cpp_filter_newlines(int * c, void * data);
static int _cpp_filter_trigraphs(int * c, void * data);

static int _cpp_callback_whitespace(Parser * parser, Token * token, int c,
		void * data);
/* FIXME handle directives */
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



/* CppParser */
/* private */
/* functions */
static CppParser * _cppparser_new(char const * filename, int filters,
		const CppOperator * operators, size_t operators_cnt)
{
	CppParser * cppparser;

	if((cppparser = object_new(sizeof(*cppparser))) == NULL)
		return NULL;
	cppparser->operators = operators;
	cppparser->operators_cnt = operators_cnt;
	if((cppparser->parser = parser_new(filename)) == NULL)
	{
		_cppparser_delete(cppparser);
		return NULL;
	}
	parser_add_filter(cppparser->parser, _cpp_filter_newlines,
			cppparser->parser);
	if(filters & CPP_FILTER_TRIGRAPH)
		parser_add_filter(cppparser->parser, _cpp_filter_trigraphs,
				cppparser->parser);
	parser_add_callback(cppparser->parser, _cpp_callback_whitespace, NULL);
	parser_add_callback(cppparser->parser, _cpp_callback_comment, NULL);
	parser_add_callback(cppparser->parser, _cpp_callback_operator,
			cppparser);
	parser_add_callback(cppparser->parser, _cpp_callback_quote, NULL);
	parser_add_callback(cppparser->parser, _cpp_callback_word, NULL);
	parser_add_callback(cppparser->parser, _cpp_callback_unknown, NULL);
	return cppparser;
}


/* cppparser_delete */
static void _cppparser_delete(CppParser * cppparser)
{
	parser_delete(cppparser->parser);
	object_delete(cppparser);
}



/* Cpp */
/* private */
/* cpp_filter_newlines */
static int _cpp_filter_newlines(int * c, void * data)
{
	Parser * parser = data;
	/* FIXME obtain from data */
	static int last;
	static int last_cnt = 0;

	if(last_cnt)
	{
		last_cnt--;
		*c = last;
		return 0;
	}
	if(*c != '\\')
		return 0;
	if((*c = parser_scan(parser)) == '\n')
	{
		*c = parser_scan(parser); /* skip the newline */
		return 0;
	}
	last = *c;
	last_cnt = 1;
	*c = '\\';
	return 1;
}


/* cpp_filter_trigraphs */
static int _trigraph_get(int last, int * c);

static int _cpp_filter_trigraphs(int * c, void * data)
{
	Parser * parser = data;
	static int last;
	static int last_cnt = 0;

	if(last_cnt == 2)
	{
		last_cnt--;
		*c = '?';
		return 0;
	}
	else if(last_cnt == 1)
	{
		last_cnt--;
		*c = last;
		return 0;
	}
	if(*c != '?')
		return 0;
	if((last = parser_scan(parser)) != '?')
	{
		last_cnt = 1;
		return 1;
	}
	last = parser_scan(parser);
	if(_trigraph_get(last, c) != 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: last=%c\n", last);
#endif
		last_cnt = 2;
		return 2;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: filtered \"??%c\" into \"%c\"\n", last, *c);
#endif
	return 0;
}

static int _trigraph_get(int last, int * c)
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
	{
		if(c == '*')
		{
			if((c = parser_scan_filter(parser)) == '/')
				break;
		}
		else
			c = parser_scan_filter(parser);
	}
	if(c == EOF)
		return -1;
	token_set_code(token, CPP_CODE_WHITESPACE);
	token_set_string(token, "");
	parser_scan_filter(parser);
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
	CppParser * cp = data;
	size_t i;
	const size_t j = sizeof(_cpp_operators) / sizeof(*_cpp_operators);
	size_t pos;

	for(i = 0; i < cp->operators_cnt; i++)
		if(cp->operators[i].string[0] == c)
			break;
	if(i == cp->operators_cnt) /* nothing found */
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: cpp_callback_operator('%c')\n", c);
#endif
	for(pos = 0; i < j;)
	{
		if(cp->operators[i].string[pos] == '\0')
			break;
		if(c == cp->operators[i].string[pos])
		{
			c = parser_scan_filter(parser);
			pos++;
		}
		else
			i++;
	}
	if(i == j) /* should not happen */
		return -1;
	token_set_code(token, cp->operators[i].code);
	token_set_string(token, cp->operators[i].string);
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
Cpp * cpp_new(void)
{
	Cpp * cpp;

	if((cpp = object_new(sizeof(*cpp))) == NULL)
		return NULL;
	cpp->parsers = NULL;
	cpp->parsers_cnt = 0;
	cpp->filters = 0;
	return cpp;
}


/* cpp_delete */
void cpp_delete(Cpp * cpp)
{
	size_t i;

	for(i = 0; i < cpp->parsers_cnt; i++)
		parser_delete(cpp->parsers[i]->parser);
	object_delete(cpp);
}


/* accessors */
/* cpp_get_filename */
char const * cpp_get_filename(Cpp * cpp)
{
	if(cpp->parsers_cnt > 0)
		return parser_get_filename(
				cpp->parsers[cpp->parsers_cnt - 1]->parser);
	return NULL;
}


/* useful */
/* cpp_filter_disable */
void cpp_filter_disable(Cpp * cpp, CppFilter filter)
{
	cpp->filters -= (cpp->filters & filter);
}


/* cpp_filter_enable */
void cpp_filter_enable(Cpp * cpp, CppFilter filter)
{
	cpp->filters |= filter;
}


/* cpp_parse */
int cpp_parse(Cpp * cpp, char const * pathname)
{
	if(cpp->parsers_cnt != 0)
		return error_set_code(1, "%s", "Already parsing");
	if((cpp->parsers = malloc(sizeof(*cpp->parsers))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	if((cpp->parsers[0] = _cppparser_new(pathname, cpp->filters,
					_cpp_operators, _cpp_operators_cnt))
			== NULL)
		return 1;
	cpp->parsers_cnt = 1;
	return 0;
}


/* cpp_scan */
int cpp_scan(Cpp * cpp, Token ** token)
{
	if(cpp->parsers_cnt == 0)
		return error_set_code(1, "%s", "No file to parse");
	return parser_get_token(cpp->parsers[cpp->parsers_cnt - 1]->parser,
			token);
}
