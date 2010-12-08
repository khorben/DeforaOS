/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
/* FIXME:
 * - potential memory leak with tokens' data
 * - add a filter for the "%" operator
 * - add a way to tokenize input from a string (and handle "#" and "##") */



#include <assert.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "parser.h"
#include "common.h"

#ifdef DEBUG
# define DEBUG_FILTER() fprintf(stderr, "DEBUG: %s('%c' 0x%x)\n", __func__, \
		*c, *c);
# define DEBUG_CALLBACK() fprintf(stderr, "DEBUG: %s('%c' 0x%x)\n", __func__, \
		c, c);
#else
# define DEBUG_FILTER()
# define DEBUG_CALLBACK()
#endif


/* CppParser */
/* private */
/* types */
typedef struct _CppOperator
{
	CppCode code;
	char const * string;
} CppOperator;

struct _CppParser
{
	Cpp * cpp;
	CppParser * parent;

	Parser * parser;
	int filters;

	/* for cpp_filter_inject */
	char * inject;
	int inject_first;
	/* for cpp_filter_newlines */
	int newlines_last;
	int newlines_last_cnt;
	/* for cpp_filter_trigraphs */
	int trigraphs_last;
	int trigraphs_last_cnt;
	/* for cpp_callback_directive */
	int directive_newline;
	int directive_control;
	/* to queue a token */
	int queue_ready;
	TokenCode queue_code;
	String * queue_string;

	CppParser * subparser;
};


/* variables */
/* operators */
static const CppOperator _cpp_operators[] =
{
	{ CPP_CODE_OPERATOR_AEQUALS,	"&="	},
	{ CPP_CODE_OPERATOR_DAMPERSAND,	"&&"	},
	{ CPP_CODE_OPERATOR_AMPERSAND,	"&"	},
	{ CPP_CODE_OPERATOR_RBRACKET,	":>"	},
	{ CPP_CODE_OPERATOR_COLON,	":"	},
	{ CPP_CODE_OPERATOR_BEQUALS,	"|="	},
	{ CPP_CODE_OPERATOR_DBAR,	"||"	},
	{ CPP_CODE_OPERATOR_BAR,	"|"	},
	{ CPP_CODE_OPERATOR_DIVEQUALS,	"/="	},
	{ CPP_CODE_OPERATOR_DIVIDE,	"/"	},
	{ CPP_CODE_OPERATOR_DOTDOTDOT,	"..."	},
	{ CPP_CODE_OPERATOR_DOT,	"."	},
	{ CPP_CODE_OPERATOR_DEQUALS,	"=="	},
	{ CPP_CODE_OPERATOR_EQUALS,	"="	},
	{ CPP_CODE_OPERATOR_DGEQUALS,	">>="	},
	{ CPP_CODE_OPERATOR_GEQUALS,	">="	},
	{ CPP_CODE_OPERATOR_DGREATER,	">>"	},
	{ CPP_CODE_OPERATOR_GREATER,	">"	},
	{ CPP_CODE_OPERATOR_DHASH,	"##"	},
	{ CPP_CODE_OPERATOR_HASH,	"#"	},
	{ CPP_CODE_OPERATOR_LBRACE,	"{"	},
	{ CPP_CODE_OPERATOR_LBRACKET,	"["	},
	{ CPP_CODE_OPERATOR_DLEQUALS,	"<<="	},
	{ CPP_CODE_OPERATOR_DLESS,	"<<"	},
	{ CPP_CODE_OPERATOR_LBRACKET,	"<:"	},
	{ CPP_CODE_OPERATOR_LBRACE,	"<%"	},
	{ CPP_CODE_OPERATOR_LEQUALS,	"<="	},
	{ CPP_CODE_OPERATOR_LESS,	"<"	},
	{ CPP_CODE_OPERATOR_LPAREN,	"("	},
	{ CPP_CODE_OPERATOR_MGREATER,	"->"	},
	{ CPP_CODE_OPERATOR_DMINUS,	"--"	},
	{ CPP_CODE_OPERATOR_MEQUALS,	"-="	},
	{ CPP_CODE_OPERATOR_MINUS,	"-"	},
	{ CPP_CODE_OPERATOR_RBRACE,	"%>"	},
	{ CPP_CODE_OPERATOR_DHASH,	"%:%:"	},
	{ CPP_CODE_OPERATOR_HASH,	"%:"	},
	{ CPP_CODE_OPERATOR_MODEQUALS,	"%="	},
	{ CPP_CODE_OPERATOR_MODULO,	"%"	},
	{ CPP_CODE_OPERATOR_NEQUALS,	"!="	},
	{ CPP_CODE_OPERATOR_NOT,	"!"	},
	{ CPP_CODE_OPERATOR_DPLUS,	"++"	},
	{ CPP_CODE_OPERATOR_PEQUALS,	"+="	},
	{ CPP_CODE_OPERATOR_PLUS,	"+"	},
	{ CPP_CODE_OPERATOR_QUESTION,	"?"	},
	{ CPP_CODE_OPERATOR_RBRACE,	"}"	},
	{ CPP_CODE_OPERATOR_RBRACKET,	"]"	},
	{ CPP_CODE_OPERATOR_RPAREN,	")"	},
	{ CPP_CODE_OPERATOR_SEMICOLON,	";"	},
	{ CPP_CODE_OPERATOR_TILDE,	"~"	},
	{ CPP_CODE_OPERATOR_TEQUALS,	"*="	},
	{ CPP_CODE_OPERATOR_TIMES,	"*"	},
	{ CPP_CODE_OPERATOR_XEQUALS,	"^="	},
	{ CPP_CODE_OPERATOR_XOR,	"^"	}
};
static const size_t _cpp_operators_cnt = sizeof(_cpp_operators)
	/ sizeof(*_cpp_operators);

/* directives */
static const char * _cpp_directives[] =
{
	"", "define", "elif", "else", "endif", "error", "if", "ifdef",
	"ifndef", "include", "pragma", "undef", "warning", NULL
};


/* prototypes */
/* useful */
static int _cpp_isword(int c);
static char * _cpp_parse_word(Parser * parser, int c);
static int _cpp_token_set(CppParser * cp, Token * token, TokenCode code,
		char const * string);

/* filters */
static int _cpp_filter_inject(int * c, void * data);
static int _cpp_filter_newlines(int * c, void * data);
static int _cpp_filter_trigraphs(int * c, void * data);

/* callbacks */
static int _cpp_callback_inject(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_dequeue(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_header(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_control(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_whitespace(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_newline(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_otherspace(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_comment(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_comma(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_operator(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_quote(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data);


/* CppParser */
/* private */
/* cpp_isword */
static int _cpp_isword(int c)
{
	return isalnum(c) || c == '_' || c == '$';
}


/* cpp_parse_word */
static char * _cpp_parse_word(Parser * parser, int c)
{
	char * str = NULL;
	size_t len = 0;
	char * p;

	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			free(str);
			return NULL;
		}
		str = p;
		str[len++] = c;
	}
	while(_cpp_isword((c = parser_scan_filter(parser))));
	str[len] = '\0';
	return str;
}


/* cpp_token_set */
static int _cpp_token_set(CppParser * cp, Token * token, TokenCode code,
		char const * string)
{
	if(token_set_string(token, string) != 0)
		return -1;
	if(cp->queue_code == CPP_CODE_NULL)
	{
		token_set_code(token, code);
		return 0;
	}
	/* we are parsing a directive */
	token_set_code(token, CPP_CODE_META_DATA);
	if(code == CPP_CODE_COMMENT)
		/* comments within directives are whitespaces */
		string = " ";
	if(cp->queue_string == NULL)
	{
		if((cp->queue_string = string_new(string)) == NULL)
			return -1;
	}
	else if(string_append(&cp->queue_string, string) != 0)
		return -1;
	return 0;
}


/* filters */
/* cpp_filter_inject */
static int _cpp_filter_inject(int * c, void * data)
{
	CppParser * cp = data;
	size_t len;
	int d;

	if(cp->inject == NULL)
		return 0;
	DEBUG_FILTER();
	if((len = strlen(cp->inject)) > 0)
	{
		d = *c;
		*c = cp->inject[0];
		memmove(cp->inject, &cp->inject[1], len--);
		if(cp->inject_first && d != EOF && d != '\0')
		{
			cp->inject[len++] = d;
			cp->inject[len] = '\0';
			cp->inject_first = 0;
		}
	}
	if(len > 0)
		return 1;
	free(cp->inject);
	cp->inject = NULL;
	return 0;
}


/* cpp_filter_newlines */
static int _cpp_filter_newlines(int * c, void * data)
{
	CppParser * cpp = data;

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
	CppParser * cpp = data;

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


/* callbacks */
/* cpp_callback_whitespace */
static int _cpp_callback_whitespace(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cpp = data;
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(!isspace(c))
		return 1;
	DEBUG_CALLBACK();
	do
	{
		if(c != '\n')
			continue;
		if((p = realloc(str, len + 2)) == NULL)
		{
			free(str);
			return -1;
		}
		str = p;
		str[len++] = c;
	}
	while(isspace((c = parser_scan_filter(parser))));
	token_set_code(token, CPP_CODE_WHITESPACE);
	if(str != NULL) /* some newlines were encountered */
	{
		str[len] = '\0';
		token_set_string(token, str);
		free(str);
		cpp->directive_newline = 1;
		cpp->queue_ready = 1;
		return 0;
	}
	token_set_string(token, " ");
	if(cpp->queue_code != CPP_CODE_NULL && cpp->queue_string != NULL)
		string_append(&cpp->queue_string, " ");
	return 0;
}


/* cpp_callback_newline */
static int _cpp_callback_newline(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cpp = data;

	if(c != '\n')
		return 1;
	DEBUG_CALLBACK();
	cpp->directive_newline = 1;
	cpp->queue_ready = 1;
	parser_scan_filter(parser);
	token_set_code(token, CPP_CODE_NEWLINE);
	token_set_string(token, "\n");
	return 0;
}


/* cpp_callback_otherspace */
static int _cpp_callback_otherspace(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cppparser = data;
	char * str = NULL;
	size_t len = 0;
	char * p;

	assert(c != '\n');
	if(!isspace(c))
		return 1;
	DEBUG_CALLBACK();
	do
	{
		if((p = realloc(str, len + 2)) == NULL)
		{
			free(str);
			return -1;
		}
		str = p;
		str[len++] = c;
	}
	while(isspace((c = parser_scan_filter(parser))) && c != '\n');
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
	CppParser * cp = data;
	char * str = NULL;
	size_t len = 2;
	char * p;

	if(c != '/')
		return 1;
	DEBUG_CALLBACK();
	if((c = parser_scan_filter(parser)) != '*')
		return _cpp_token_set(cp, token, CPP_CODE_OPERATOR_DIVIDE, "/");
	for(c = parser_scan_filter(parser); c != EOF;)
	{
		if(!(cp->filters & CPP_FILTER_COMMENT))
		{
			if((p = realloc(str, len + 3)) == NULL)
				return -error_set_code(1, "%s", strerror(
							errno));
			str = p;
			str[len++] = c;
		}
		if(c == '*')
		{
			if((c = parser_scan_filter(parser)) == '/')
				break;
		}
		else
			c = parser_scan_filter(parser);
	}
	if(c == EOF)
		return -error_set_code(1, "%s", "End of file within a comment");
	parser_scan_filter(parser);
	if(str == NULL)
		return _cpp_token_set(cp, token, CPP_CODE_WHITESPACE, " ");
	str[0] = '/';
	str[1] = '*';
	str[len++] = '/';
	str[len] = '\0';
	_cpp_token_set(cp, token, CPP_CODE_COMMENT, str); /* XXX may fail */
	free(str);
	return 0;
}


/* cpp_callback_inject */
static int _cpp_callback_inject(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cp = data;
	char buf[2] = "\0";

	if(cp->inject_first == 0)
		return 1;
	DEBUG_CALLBACK();
	if(c == EOF)
		return 1;
	/* the current character actually goes after the substitution */
	buf[0] = c;
	if(string_append(&cp->inject, buf) != 0)
		return -1;
	parser_scan_filter(parser);
	return 1;
}


/* cpp_callback_dequeue */
static int _cpp_callback_dequeue(Parser * parser, Token * token, int c,
		void * data)
{
	int ret = 0;
	CppParser * cpp = data;

	if(cpp->queue_ready == 0)
		return 1;
	cpp->queue_ready = 0;
	if(cpp->queue_code == CPP_CODE_NULL && cpp->queue_string == NULL)
		return 1;
	DEBUG_CALLBACK();
	token_set_code(token, cpp->queue_code);
	switch(cpp->queue_code)
	{
		case CPP_CODE_META_DEFINE:
		case CPP_CODE_META_ELIF:
		case CPP_CODE_META_IF:
		case CPP_CODE_META_IFDEF:
		case CPP_CODE_META_IFNDEF:
		case CPP_CODE_META_INCLUDE:
		case CPP_CODE_META_UNDEF:
			token_set_string(token, "");
			token_set_data(token, cpp->queue_string);
			cpp->queue_string = NULL;
			break;
		case CPP_CODE_META_ERROR:
		case CPP_CODE_META_WARNING:
			token_set_string(token, (cpp->queue_string != NULL)
					? cpp->queue_string : "");
			break;
		default:
			token_set_string(token, "");
			break;
	}
	cpp->queue_code = CPP_CODE_NULL;
	string_delete(cpp->queue_string);
	cpp->queue_string = NULL;
	cpp->directive_newline = 1;
	cpp->directive_control = 0;
	return ret;
}


/* cpp_callback_header */
static int _cpp_callback_header(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cp = data;
	char end;
	char * str = NULL;
	size_t len = 0;
	char * p;

	if(cp->directive_control != 1 || cp->queue_code != CPP_CODE_META_INCLUDE
			|| (c != '<' && c != '"'))
		return 1;
	DEBUG_CALLBACK();
	end = (c == '<') ? '>' : '"';
	while((p = realloc(str, len + 3)) != NULL)
	{
		str = p;
		str[len++] = c;
		if((c = parser_scan_filter(parser)) == EOF || c == '\n')
			break;
		else if(c == end)
			break;
	}
	if(p == NULL) /* there was an error with realloc() */
	{
		error_set_code(1, "%s", strerror(errno));
		free(str);
		return -1;
	}
	else if(c == end) /* the header name is properly closed */
	{
		str[len++] = c;
		parser_scan_filter(parser);
	}
	str[len] = '\0';
	token_set_code(token, CPP_CODE_META_DATA);
	token_set_string(token, str);
	if(cp->queue_string == NULL)
		cp->queue_string = str;
	else
	{
		free(str);
		cp->queue_code = CPP_CODE_META_ERROR;
		free(cp->queue_string);
		/* XXX may be followed by junk */
		cp->queue_string = strdup("Syntax error");
	}
	return 0;
}


/* cpp_callback_control */
static int _cpp_callback_control(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cpp = data;

	if(cpp->directive_newline != 1 || c != '#')
	{
		cpp->directive_newline = 0;
		return 1;
	}
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	token_set_code(token, CPP_CODE_META_DATA);
	token_set_string(token, "#");
	cpp->directive_newline = 0;
	cpp->directive_control = 1;
	cpp->queue_code = CPP_CODE_NULL;
	return 0;
}


/* cpp_callback_comma */
static int _cpp_callback_comma(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cp = data;

	if(c != ',')
		return 1;
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	return _cpp_token_set(cp, token, CPP_CODE_COMMA, ",");
}


/* cpp_callback_operator */
static int _cpp_callback_operator(Parser * parser, Token * token, int c,
		void * data)
	/* FIXME probably fails for ".." and similar cases */
{
	CppParser * cp = data;
	size_t i;
	const size_t j = sizeof(_cpp_operators) / sizeof(*_cpp_operators);
	size_t pos;

	for(i = 0; i < _cpp_operators_cnt; i++)
		if(_cpp_operators[i].string[0] == c)
			break;
	if(i == _cpp_operators_cnt) /* nothing found */
		return 1;
	DEBUG_CALLBACK();
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
	return _cpp_token_set(cp, token, _cpp_operators[i].code,
			_cpp_operators[i].string);
}


/* cpp_callback_quote */
static int _cpp_callback_quote(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cp = data;
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
	DEBUG_CALLBACK();
	while((p = realloc(str, len + 3)) != NULL)
	{
		str = p;
		str[len++] = c;
		if((c = parser_scan_filter(parser)) == EOF || c == '\n')
			break;
		if(escape)
			escape = 0;
		else if(c == str[0])
			break;
		else if(c == '\\')
			escape = 1;
	}
	if(p == NULL) /* there was an error with realloc() */
	{
		error_set_code(1, "%s", strerror(errno));
		free(str);
		return -1;
	}
	else if(c == str[0]) /* the quoted string is properly closed */
	{
		str[len++] = str[0];
		parser_scan_filter(parser);
	} /* XXX else we should probably issue a warning */
	str[len] = '\0';
	/* XXX keep code earlier, may fail */
	_cpp_token_set(cp, token, token_get_code(token), str);
	free(str);
	return 0;
}


/* cpp_callback_directive */
static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cpp = data;
	char * str;
	size_t i;

	if(cpp->directive_control != 1 || cpp->queue_code != CPP_CODE_NULL
			|| !_cpp_isword(c))
		return 1;
	DEBUG_CALLBACK();
	if((str = _cpp_parse_word(parser, c)) == NULL)
		return -1;
	for(i = 0; _cpp_directives[i] != NULL; i++)
		if(strcmp(str, _cpp_directives[i]) == 0)
			break;
	if(_cpp_directives[i] != NULL)
	{
		cpp->queue_code = CPP_CODE_META_FIRST + i;
		cpp->queue_string = NULL;
	}
	else
	{
		cpp->queue_code = CPP_CODE_META_ERROR;
		cpp->queue_string = string_new_append("Invalid directive: #",
				str, ":", NULL); /* XXX check for errors */
	}
	token_set_code(token, CPP_CODE_META_DATA);
	token_set_string(token, str);
	free(str);
	return 0;
}


/* cpp_callback_word */
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data)
{
	int ret;
	CppParser * cp = data;
	char * str;

	if(!_cpp_isword(c))
		return 1;
	DEBUG_CALLBACK();
	if((str = _cpp_parse_word(parser, c)) == NULL)
		return -1;
	ret = _cpp_token_set(cp, token, CPP_CODE_WORD, str);
	free(str);
	return ret;
}


/* cpp_callback_unknown */
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data)
{
	CppParser * cp = data;
	char buf[2] = "\0";

	if(c == EOF)
		return 1;
	DEBUG_CALLBACK();
	buf[0] = c;
	parser_scan_filter(parser);
	return _cpp_token_set(cp, token, CPP_CODE_UNKNOWN, buf);
}


/* public */
/* functions */
/* cppparser_new */
CppParser * cppparser_new(Cpp * cpp, CppParser * parent, char const * filename,
		int filters)
{
	CppParser * cp;

	if((cp = object_new(sizeof(*cp))) == NULL)
		return NULL;
	cp->cpp = cpp;
	cp->parent = parent;
	cp->parser = parser_new(filename);
	cp->filters = filters;
	cp->inject = NULL;
	cp->inject_first = 0;
	cp->newlines_last = 0;
	cp->newlines_last_cnt = 0;
	cp->trigraphs_last = 0;
	cp->trigraphs_last_cnt = 0;
	cp->directive_newline = 1;
	cp->directive_control = 0;
	cp->queue_ready = 0;
	cp->queue_code = CPP_CODE_NULL;
	cp->queue_string = NULL;
	cp->subparser = NULL;
	if(cp->parser == NULL)
	{
		cppparser_delete(cp);
		return NULL;
	}
	parser_add_filter(cp->parser, _cpp_filter_inject, cp);
	parser_add_filter(cp->parser, _cpp_filter_newlines, cp);
	if(cp->filters & CPP_FILTER_TRIGRAPH)
		parser_add_filter(cp->parser, _cpp_filter_trigraphs, cp);
	parser_add_callback(cp->parser, _cpp_callback_inject, cp);
	parser_add_callback(cp->parser, _cpp_callback_dequeue, cp);
	if(cp->filters & CPP_FILTER_WHITESPACE)
		parser_add_callback(cp->parser, _cpp_callback_whitespace, cp);
	else
	{
		parser_add_callback(cp->parser, _cpp_callback_newline, cp);
		parser_add_callback(cp->parser, _cpp_callback_otherspace, cp);
	}
	parser_add_callback(cp->parser, _cpp_callback_comment, cp);
	parser_add_callback(cp->parser, _cpp_callback_header, cp);
	parser_add_callback(cp->parser, _cpp_callback_control, cp);
	parser_add_callback(cp->parser, _cpp_callback_comma, cp);
	parser_add_callback(cp->parser, _cpp_callback_operator, cp);
	parser_add_callback(cp->parser, _cpp_callback_quote, cp);
	parser_add_callback(cp->parser, _cpp_callback_directive, cp);
	parser_add_callback(cp->parser, _cpp_callback_word, cp);
	parser_add_callback(cp->parser, _cpp_callback_unknown, cp);
	return cp;
}


/* cppparser_delete */
void cppparser_delete(CppParser * cp)
{
	string_delete(cp->queue_string);
	if(cp->subparser != NULL)
		cppparser_delete(cp->subparser);
	if(cp->parser != NULL)
		parser_delete(cp->parser);
	string_delete(cp->inject);
	object_delete(cp);
}


/* accessors */
/* cppparser_get_filename */
char const * cppparser_get_filename(CppParser * cpp)
{
	return parser_get_filename(cpp->parser);
}


/* useful */
/* cppparser_include */
static char * _include_path(CppParser * cpp, char const * str);
static char * _path_lookup(CppParser * cp, char const * path, int system);

int cppparser_include(CppParser * cp, char const * include)
{
	char * path;

	if((path = _include_path(cp, include)) == NULL)
		return -1;
	for(; cp->subparser != NULL; cp = cp->subparser);
	cp->subparser = cppparser_new(cp->cpp, cp, path, cp->filters);
	free(path);
	return (cp->subparser != NULL) ? 0 : -1;
}

static char * _include_path(CppParser * cpp, char const * str)
{
	int d;
	size_t i;
	char * path = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, cpp, str);
#endif
	if(str[0] == '"')
		d = str[0];
	else if(str[0] == '<')
		d = '>';
	else
	{
		error_set("%s", "Invalid include directive");
		return NULL;
	}
	for(i = 1; str[i] != '\0' && str[i] != d; i++);
	/* FIXME also check what's behind the directive */
	if(i == 1 || str[i] != d)
	{
		error_set("%s", "Invalid include directive");
		return NULL;
	}
	if((path = strdup(&str[1])) == NULL)
	{
		error_set("%s", strerror(errno));
		return NULL;
	}
	path[i - 1] = '\0';
	p = _path_lookup(cpp, path, d == '>');
	free(path);
	return p;
}

static char * _path_lookup(CppParser * cp, char const * path, int system)
{
	Cpp * cpp = cp->cpp;
	char const * filename;
	char * p;
	char * q;
	char * r;
	struct stat st;

	if(system != 0)
		return cpp_path_lookup(cp->cpp, path);
	for(; cp != NULL; cp = cp->parent)
	{
		filename = parser_get_filename(cp->parser);
		if((p = string_new(filename)) == NULL)
			return NULL;
		q = dirname(p);
		if((r = string_new(q)) == NULL || string_append(&r, "/") != 0
				|| string_append(&r, path) != 0)
		{
			string_delete(r);
			string_delete(p);
			return NULL;
		}
		string_delete(p);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: stat(\"%s\", %p)\n", r, &st);
#endif
		if(stat(r, &st) == 0)
			return r;
		error_set("%s: %s", r, strerror(errno));
		string_delete(r);
	}
	return cpp_path_lookup(cpp, path); /* XXX errors change "" into <> */
}


/* cppparser_inject */
/* FIXME should take a buffer as input? */
int cppparser_inject(CppParser * cp, char const * string)
{
	if(string == NULL || string[0] == '\0')
		return 0; /* don't bother */
	for(; cp->subparser != NULL; cp = cp->subparser);
	if(cp->inject == NULL)
	{
		if((cp->inject = string_new(string)) == NULL)
			return 1;
	}
	else if(string_append(&cp->inject, string) != 0)
		return 1;
	cp->inject_first = 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\") => \"%s\"\n", __func__, cp,
			string, cp->inject);
#endif
	return 0;
}


/* cppparser_scan */
int cppparser_scan(CppParser * cp, Token ** token)
{
	if(cp->subparser != NULL)
	{
		if(cppparser_scan(cp->subparser, token) != 0)
			return 1;
		if(*token != NULL)
			return 0;
		cppparser_delete(cp->subparser); /* end of file */
		cp->subparser = NULL;
	}
	return parser_get_token(cp->parser, token);
}
