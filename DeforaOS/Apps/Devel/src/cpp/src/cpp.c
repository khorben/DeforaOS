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
/* FIXME:
 * - comments are not handled in directives
 * - fix includes (system vs regular, inclusion order)
 * - potential memory leak with tokens' data
 * - add a filter for the "%" operator
 * - add a way to tokenize input from a string (and handle "#" and "##") */



#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>
#include "common.h"

#ifdef DEBUG
# define DEBUG_CALLBACK() fprintf(stderr, "DEBUG: %s('%c' 0x%x)\n", __func__, \
		c, c);
#else
# define DEBUG_CALLBACK()
#endif


/* Cpp */
/* private */
/* types */
typedef struct _CppOperator
{
	CppCode code;
	char const * string;
} CppOperator;

/* FIXME use CPP_CODE_META_* in a structure with strings and pointers to
 *       functions instead? */
typedef enum _CppDirective
{
	CPP_DIRECTIVE_DEFINE = 0,
	CPP_DIRECTIVE_ELIF,
	CPP_DIRECTIVE_ELSE,
	CPP_DIRECTIVE_ENDIF,
	CPP_DIRECTIVE_ERROR,
	CPP_DIRECTIVE_IF,
	CPP_DIRECTIVE_IFDEF,
	CPP_DIRECTIVE_IFNDEF,
	CPP_DIRECTIVE_INCLUDE,
	CPP_DIRECTIVE_LINE,
	CPP_DIRECTIVE_PRAGMA,
	CPP_DIRECTIVE_UNDEF,
	CPP_DIRECTIVE_WARNING
} CppDirective;
#define CPP_DIRECTIVE_LAST CPP_DIRECTIVE_WARNING
#define CPP_DIRECTIVE_COUNT (CPP_DIRECTIVE_LAST + 1)


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
static const size_t _cpp_directives_cnt = CPP_DIRECTIVE_COUNT;
static const char * _cpp_directives[CPP_DIRECTIVE_COUNT] =
{
	"define", "elif", "else", "endif", "error", "if", "ifdef", "ifndef",
	"include", "line", "pragma", "undef", "warning"
};


/* prototypes */
/* useful */
static int _cpp_isword(int c);
static char * _cpp_parse_line(Parser * parser, int c);
static char * _cpp_parse_word(Parser * parser, int c);

/* filters */
static int _cpp_filter_newlines(int * c, void * data);
static int _cpp_filter_trigraphs(int * c, void * data);

/* callbacks */
static int _cpp_callback_directive(Parser * parser, Token * token, int c,
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
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data);
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data);


/* Cpp */
/* private */
/* cpp_isword */
static int _cpp_isword(int c)
{
	return isalnum(c) || c == '_' || c == '$';
}


/* cpp_parse_line */
static char * _cpp_parse_line(Parser * parser, int c)
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
	while((c = parser_scan_filter(parser)) != EOF && c != '\n');
	str[len] = '\0';
	return str;
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


/* filters */
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


/* callbacks */
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


/* cpp_callback_newline */
static int _cpp_callback_newline(Parser * parser, Token * token, int c,
		void * data)
{
	if(c != '\n')
		return 1;
	DEBUG_CALLBACK();
	parser_scan_filter(parser);
	token_set_code(token, CPP_CODE_NEWLINE);
	token_set_string(token, "\n");
	return 0;
}


/* cpp_callback_otherspace */
static int _cpp_callback_otherspace(Parser * parser, Token * token, int c,
		void * data)
{
	Cpp * cpp = data;
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
	Cpp * cpp = data;
	char * str = NULL;
	size_t len = 2;
	char * p;

	if(c != '/')
		return 1;
	DEBUG_CALLBACK();
	if((c = parser_scan_filter(parser)) != '*')
	{
		token_set_code(token, CPP_CODE_OPERATOR_DIVIDE);
		token_set_string(token, "/");
		return 0;
	}
	for(c = parser_scan_filter(parser); c != EOF;)
	{
		if(!(cpp->filters & CPP_FILTER_COMMENT))
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
	if(str != NULL)
	{
		str[0] = '/';
		str[1] = '*';
		str[len++] = '/';
		str[len] = '\0';
		token_set_code(token, CPP_CODE_COMMENT);
		token_set_string(token, str);
		free(str);
	}
	else
	{
		token_set_code(token, CPP_CODE_WHITESPACE);
		token_set_string(token, " ");
	}
	parser_scan_filter(parser);
	return 0;
}


/* cpp_callback_directive */
/* directives: these functions should return 0 (or -1 on errors) */
static int _directive_ifdef(Token * token, char const * str);
static int _directive_ifndef(Token * token, char const * str);
static int _directive_include(Cpp * cpp, Token * token, char const * str);
static int _directive_unknown(Token * token, char const * str);

static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data)
	/* FIXME actually parse and implement, careful with comments */
{
	int ret = 0;
	Cpp * cpp = data;
	char * str;
	char * pos;
	int tmp;
	size_t n;
	size_t i;

	if(cpp->directive_newline != 1 || c != '#')
	{
		cpp->directive_newline = 0;
		return 1;
	}
	DEBUG_CALLBACK();
	if((str = _cpp_parse_line(parser, c)) == NULL)
		return -1;
	token_set_string(token, str);
	for(pos = &str[1]; isspace((tmp = *pos)); pos++); /* skip whitespaces */
	for(n = 0; pos[n] != '\0' && _cpp_isword(pos[n]); n++);
	for(i = 0; i < _cpp_directives_cnt; i++)
		if(strncmp(pos, _cpp_directives[i], n) == 0
				&& _cpp_directives[i][n] == '\0')
			break;
	for(pos = &pos[n]; isspace((tmp = *pos)); pos++); /* skip whitespaces */
	switch(i)
	{
		case CPP_DIRECTIVE_DEFINE:
			token_set_code(token, CPP_CODE_META_DEFINE);
			break;
		case CPP_DIRECTIVE_ELIF:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_ELIF);
			break;
		case CPP_DIRECTIVE_ELSE:
			token_set_code(token, CPP_CODE_META_ELSE);
			break;
		case CPP_DIRECTIVE_ENDIF:
			token_set_code(token, CPP_CODE_META_ENDIF);
			break;
		case CPP_DIRECTIVE_ERROR:
			token_set_code(token, CPP_CODE_META_ERROR);
			break;
		case CPP_DIRECTIVE_IF:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_IF);
			break;
		case CPP_DIRECTIVE_IFDEF:
			ret = _directive_ifdef(token, pos);
			break;
		case CPP_DIRECTIVE_IFNDEF:
			ret = _directive_ifndef(token, pos);
			break;
		case CPP_DIRECTIVE_INCLUDE:
			ret = _directive_include(cpp, token, pos);
			break;
		case CPP_DIRECTIVE_LINE:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_LINE);
			break;
		case CPP_DIRECTIVE_PRAGMA:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_PRAGMA);
			break;
		case CPP_DIRECTIVE_UNDEF:
			token_set_code(token, CPP_CODE_META_UNDEF);
			break;
		case CPP_DIRECTIVE_WARNING:
			token_set_code(token, CPP_CODE_META_WARNING);
			break;
		default:
			_directive_unknown(token, str);
			break;
	}
	free(str);
	return ret;
}

static int _directive_unknown(Token * token, char const * str)
{
	char buf[256];

	token_set_code(token, CPP_CODE_META_ERROR);
	snprintf(buf, sizeof(buf), "%s%s", "Unknown directive: ", str);
	token_set_string(token, buf);
	return 0;
}

/* directives */
/* directive_ifdef */
static int _directive_ifdef(Token * token, char const * str)
{
	char * p;

	if((p = strdup(str)) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	token_set_code(token, CPP_CODE_META_IFDEF);
	token_set_data(token, p);
	return 0;
}

/* directive_ifndef */
static int _directive_ifndef(Token * token, char const * str)
{
	char * p;

	if((p = strdup(str)) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	token_set_code(token, CPP_CODE_META_IFNDEF);
	token_set_data(token, p);
	return 0;
}

/* directive_include */
static char * _include_path(Cpp * cpp, Token * token, char const * str);
static char * _path_lookup(Cpp * cpp, Token * token, char const * path,
		int system);
static char * _lookup_error(Token * token, char const * path, int system);

static int _directive_include(Cpp * cpp, Token * token, char const * str)
{
	char * path;

	if((path = _include_path(cpp, token, str)) == NULL
			&& (path = _include_path(cpp->parent, token, str))
			== NULL)
	{
		token_set_code(token, CPP_CODE_META_ERROR);
		token_set_string(token, error_get());
		return 0;
	}
	token_set_code(token, CPP_CODE_META_INCLUDE);
	if((cpp->subparser = cpp_new(path, cpp->filters)) == NULL)
	{
		free(path);
		return -1;
	}
	free(path);
	cpp->subparser->parent = cpp->parent;
	return 0;
}

static char * _include_path(Cpp * cpp, Token * token, char const * str)
	/* FIXME use presets for path discovery and then dirname(filename) */
{
	int d;
	size_t len;
	char * path = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %s)\n", __func__, cpp, str);
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
	len = strlen(str);
	if(len < 3 || str[len - 1] != d)
	{
		error_set("%s", "Invalid include directive");
		return NULL;
	}
	if((path = strdup(&str[1])) == NULL)
	{
		error_set("%s", strerror(errno));
		return NULL;
	}
	path[len - 2] = '\0';
	p = _path_lookup(cpp, token, path, d == '>');
	free(path);
	return p;
}

static char * _path_lookup(Cpp * cpp, Token * token, char const * path,
		int system)
{
	size_t i;
	char * buf = NULL;
	char * p;
	struct stat st;

	for(i = 0; i < cpp->paths_cnt; i++)
	{
		if((p = realloc(buf, strlen(cpp->paths[i]) + strlen(path) + 2))
				== NULL)
		{
			free(buf);
			return _lookup_error(token, path, system);
		}
		buf = p;
		sprintf(buf, "%s/%s", cpp->paths[i], path);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: stat(\"%s\", %p)\n", buf, &st);
#endif
		if(stat(buf, &st) == 0)
			return buf;
	}
	free(buf);
	return _lookup_error(token, path, system);
}

static char * _lookup_error(Token * token, char const * path, int system)
{
	char buf[256];

	token_set_code(token, CPP_CODE_META_ERROR);
	snprintf(buf, sizeof(buf), "%s%c%s%c: %s", "Cannot include ",
			system ? '<' : '"', path, system ? '>' : '"',
			strerror(errno));
	token_set_string(token, buf);
	return NULL;
}


/* cpp_callback_comma */
static int _cpp_callback_comma(Parser * parser, Token * token, int c,
		void * data)
{
	if(c != ',')
		return 1;
	DEBUG_CALLBACK();
	token_set_code(token, CPP_CODE_COMMA);
	token_set_string(token, ",");
	parser_scan_filter(parser);
	return 0;
}


/* cpp_callback_operator */
static int _cpp_callback_operator(Parser * parser, Token * token, int c,
		void * data)
	/* FIXME probably fails for ".." and similar cases */
{
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
	token_set_string(token, str);
	free(str);
	return 0;
}


/* cpp_callback_word */
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data)
{
	char * str;

	if(!_cpp_isword(c))
		return 1;
	DEBUG_CALLBACK();
	if((str = _cpp_parse_word(parser, c)) == NULL)
		return -1;
	token_set_code(token, CPP_CODE_WORD);
	token_set_string(token, str);
	free(str);
	return 0;
}


/* cpp_callback_unknown */
static int _cpp_callback_unknown(Parser * parser, Token * token, int c,
		void * data)
{
	char buf[2] = { '\0', '\0' };

	if(c == EOF)
		return 1;
	DEBUG_CALLBACK();
	buf[0] = c;
	parser_scan(parser);
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
	char * p;
	int r = 0;

	if((cpp = object_new(sizeof(*cpp))) == NULL)
		return NULL;
	memset(cpp, 0, sizeof(*cpp));
	cpp->filters = filters;
	cpp->parser = parser_new(filename);
	cpp->directive_newline = 1;
	cpp->parent = cpp;
	if((p = strdup(filename)) != NULL)
	{
		r = cpp_path_add(cpp, dirname(p)); /* FIXME inclusion order */
		free(p);
	}
	if(r != 0 || cpp->parser == NULL || cpp->paths_cnt != 1)
	{
		cpp_delete(cpp);
		return NULL;
	}
	parser_add_filter(cpp->parser, _cpp_filter_newlines, cpp);
	if(cpp->filters & CPP_FILTER_TRIGRAPH)
		parser_add_filter(cpp->parser, _cpp_filter_trigraphs, cpp);
	if(cpp->filters & CPP_FILTER_WHITESPACE)
		parser_add_callback(cpp->parser, _cpp_callback_whitespace, cpp);
	else
	{
		parser_add_callback(cpp->parser, _cpp_callback_newline, cpp);
		parser_add_callback(cpp->parser, _cpp_callback_otherspace, cpp);
	}
	parser_add_callback(cpp->parser, _cpp_callback_comment, cpp);
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
	size_t i;

	if(cpp->parent == cpp)
	{
		for(i = 0; i < cpp->defines_cnt; i++)
		{
			free(cpp->defines[i].name);
			free(cpp->defines[i].value);
		}
		free(cpp->defines);
		for(i = 0; i < cpp->paths_cnt; i++)
			free(cpp->paths[i]);
		free(cpp->paths);
	}
	else
	{
		assert(cpp->defines_cnt == 0);
		assert(cpp->paths_cnt == 1);
		free(cpp->paths[0]);
		free(cpp->paths);
		assert(cpp->scopes_cnt == 0);
	}
	if(cpp->subparser != NULL)
		cpp_delete(cpp->subparser);
	if(cpp->parser != NULL)
		parser_delete(cpp->parser);
	if(cpp->scopes != NULL)
		free(cpp->scopes);
	object_delete(cpp);
}


/* accessors */
/* cpp_get_filename */
char const * cpp_get_filename(Cpp * cpp)
{
	return parser_get_filename(cpp->parser);
}


/* cpp_is_defined */
int cpp_is_defined(Cpp * cpp, char const * name)
{
	size_t i;

	cpp = cpp->parent;
	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			return 1;
	return 0;
}


/* useful */
/* cpp_define_add */
int cpp_define_add(Cpp * cpp, char const * name, char const * value)
	/* FIXME should verify validity of name and interpret value */
{
	size_t i;
	CppDefine * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\", \"%s\")\n", __func__, name,
			value);
#endif
	cpp = cpp->parent;
	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			break;
	if(i != cpp->defines_cnt)
		return error_set_code(1, "%s is already defined", name);
	if((p = realloc(cpp->defines, sizeof(*p) * (cpp->defines_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->defines = p;
	p = &p[cpp->defines_cnt];
	p->name = strdup(name);
	p->value = (value != NULL) ? strdup(value) : NULL;
	if(p->name == NULL || (value != NULL && p->value == NULL))
	{
		free(p->name);
		free(p->value);
		return error_set_code(1, "%s", strerror(errno));
	}
	cpp->defines_cnt++;
	return 0;
}


/* cpp_define_remove */
int cpp_define_remove(Cpp * cpp, char const * name)
	/* FIXME should verify validity of name */
{
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\")\n", __func__, name);
#endif
	cpp = cpp->parent;
	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i].name, name) == 0)
			break;
	if(i == cpp->defines_cnt)
		return error_set_code(1, "%s is not defined", name);
	free(cpp->defines[i].name);
	free(cpp->defines[i].value);
	cpp->defines_cnt--;
	for(; i < cpp->defines_cnt; i++)
	{
		cpp->defines[i].name = cpp->defines[i + 1].name;
		cpp->defines[i].value = cpp->defines[i + 1].value;
	}
	return 0;
}


/* cpp_path_add */
int cpp_path_add(Cpp * cpp, char const * path)
{
	char ** p;

	cpp = cpp->parent;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(cpp, \"%s\")\n", __func__, path);
#endif
	if((p = realloc(cpp->paths, sizeof(*p) * (cpp->paths_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths = p;
	if((p[cpp->paths_cnt] = strdup(path)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths_cnt++;
	return 0;
}
