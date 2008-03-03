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
/* FIXME
 * - comments are not handled in directives
 * - fix includes (system vs regular, inclusion order)
 * - implement define and undef
 * - implement ifdef and ifndef */



#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>
#include "cpp.h"


/* Cpp */
/* private */
/* types */
typedef struct _CppOperator
{
	CppCode code;
	char const * string;
} CppOperator;

struct _Cpp
{
	int filters;
	Parser * parser;
	/* for cpp_filter_newlines */
	int newlines_last;
	int newlines_last_cnt;
	/* for cpp_filter_trigraphs */
	int trigraphs_last;
	int trigraphs_last_cnt;
	/* for cpp_callback_directive */
	int directive_newline;
	/* for include directives */
	Cpp * subparser;
	char ** paths;
	size_t paths_cnt;
	/* substitutions */
	char ** defines; /* FIXME also store the value, directly in tokens? */
	size_t defines_cnt;
};

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
	CPP_DIRECTIVE_LINE,
	CPP_DIRECTIVE_IFNDEF,
	CPP_DIRECTIVE_INCLUDE,
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
	{ CPP_CODE_OPERATOR_DIVIDE,	"/"	},
	{ CPP_CODE_OPERATOR_DOTDOTDOT,	"..."	},
	{ CPP_CODE_OPERATOR_DOT,	"."	},
	{ CPP_CODE_OPERATOR_DEQUALS,	"=="	},
	{ CPP_CODE_OPERATOR_EQUALS,	"="	},
	{ CPP_CODE_OPERATOR_DGEQUALS,	">>="	},
	{ CPP_CODE_OPERATOR_DGREATER,	">>"	},
	{ CPP_CODE_OPERATOR_GREATER,	">"	},
	{ CPP_CODE_OPERATOR_DHASH,	"##"	},
	{ CPP_CODE_OPERATOR_HASH,	"#"	},
	{ CPP_CODE_OPERATOR_INVERSE,	"~"	},
	{ CPP_CODE_OPERATOR_LBRACE,	"{"	},
	{ CPP_CODE_OPERATOR_LBRACKET,	"["	},
	{ CPP_CODE_OPERATOR_DLEQUALS,	"<<="	},
	{ CPP_CODE_OPERATOR_DLESS,	"<<"	},
	{ CPP_CODE_OPERATOR_LBRACKET,	"<:"	},
	{ CPP_CODE_OPERATOR_LBRACE,	"<%"	},
	{ CPP_CODE_OPERATOR_LESS,	"<"	},
	{ CPP_CODE_OPERATOR_LPAREN,	"("	},
	{ CPP_CODE_OPERATOR_MGREATER,	"->"	},
	{ CPP_CODE_OPERATOR_DMINUS,	"--"	},
	{ CPP_CODE_OPERATOR_MEQUALS,	"-="	},
	{ CPP_CODE_OPERATOR_MINUS,	"-"	},
	{ CPP_CODE_OPERATOR_RBRACE,	"%>"	},
	{ CPP_CODE_OPERATOR_DHASH,	"%:%:"	},
	{ CPP_CODE_OPERATOR_HASH,	"%:"	},
	{ CPP_CODE_OPERATOR_MODULO,	"%"	},
	{ CPP_CODE_OPERATOR_NOT,	"!"	},
	{ CPP_CODE_OPERATOR_DPLUS,	"++"	},
	{ CPP_CODE_OPERATOR_PEQUALS,	"+="	},
	{ CPP_CODE_OPERATOR_PLUS,	"+"	},
	{ CPP_CODE_OPERATOR_QUESTION,	"?"	},
	{ CPP_CODE_OPERATOR_RBRACE,	"}"	},
	{ CPP_CODE_OPERATOR_RBRACKET,	"]"	},
	{ CPP_CODE_OPERATOR_RPAREN,	")"	},
	{ CPP_CODE_OPERATOR_SEMICOLON,	";"	},
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
	return isalnum(c) || c == '_';
}


/* cpp_parse_line */
static char * _cpp_parse_line(Parser * parser, int c)
{
	char * str = NULL;
	size_t len = 0;
	char * p;

	do
	{
		if((p = realloc(str, len + 1)) == NULL)
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
		if((p = realloc(str, len + 1)) == NULL)
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
/* these functions should return 0 (or -1 on errors) */
static int _directive_error(Cpp * cpp, Token * token, char const * str);
static int _directive_include(Cpp * cpp, Token * token, char const * str);
static int _directive_undef(Cpp * cpp, Token * token, char const * str);

static int _cpp_callback_directive(Parser * parser, Token * token, int c,
		void * data)
	/* FIXME actually parse and implement, careful with comments */
{
	int ret = 0;
	Cpp * cpp = data;
	char * str;
	char * pos;
	size_t n;
	size_t i;

	if(cpp->directive_newline != 1 || c != '#')
	{
		cpp->directive_newline = 0;
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s", "DEBUG: cpp_callback_directive()\n");
#endif
	if((str = _cpp_parse_line(parser, c)) == NULL)
		return -1;
	token_set_string(token, str);
	for(pos = &str[1]; isspace(*pos); pos++); /* skip whitespaces */
	for(n = 0; pos[n] != '\0' && _cpp_isword(pos[n]); n++);
	for(i = 0; i < _cpp_directives_cnt; i++)
		if(strncmp(pos, _cpp_directives[i], n) == 0
				&& _cpp_directives[i][n] == '\0')
			break;
	for(pos = &pos[n]; isspace(*pos); pos++); /* skip whitespaces */
	switch(i)
	{
		case CPP_DIRECTIVE_DEFINE:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_DEFINE);
			break;
		case CPP_DIRECTIVE_ELIF:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_ELIF);
			break;
		case CPP_DIRECTIVE_ELSE:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_ELSE);
			break;
		case CPP_DIRECTIVE_ENDIF:
			/* FIXME implement */
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
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_IFDEF);
			break;
		case CPP_DIRECTIVE_IFNDEF:
			/* FIXME implement */
			token_set_code(token, CPP_CODE_META_IFNDEF);
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
			_directive_undef(cpp, token, str);
			break;
		case CPP_DIRECTIVE_WARNING:
			token_set_code(token, CPP_CODE_META_WARNING);
			break;
		default:
			_directive_error(cpp, token, str);
			break;
	}
	free(str);
	return ret;
}

static int _directive_error(Cpp * cpp, Token * token, char const * str)
	/* FIXME line and column will probably be wrong for included content
	 *       use a parser to keep track of it? */
{
	char buf[256];

	token_set_code(token, CPP_CODE_META_ERROR);
	snprintf(buf, sizeof(buf), "%s%s", "Unknown or invalid directive: ",
			str);
	token_set_string(token, buf);
	return 0;
}

static char * _include_path(Cpp * cpp, Token * token, char const * str);
static int _directive_include(Cpp * cpp, Token * token, char const * str)
{
	char * path;
	size_t i;

	if((path = _include_path(cpp, token, str)) == NULL)
		return 0;
	token_set_code(token, CPP_CODE_META_INCLUDE);
	if((cpp->subparser = cpp_new(path, cpp->filters)) == NULL)
	{
		error_set_code(-1, "%s: %s", path, strerror(errno));
		free(path);
		return -1;
	}
	for(i = 0; i < cpp->paths_cnt; i++)
		if(cpp_path_add(cpp->subparser, cpp->paths[i]) != 0)
			break;
	if(i != cpp->paths_cnt)
	{
		cpp_delete(cpp->subparser);
		cpp->subparser = NULL;
		return 1;
	}
	return 0;
}

static char * _path_lookup(Cpp * cpp, Token * token, char const * path,
		int system);
static char * _include_path(Cpp * cpp, Token * token, char const * str)
	/* FIXME use presets for path discovery and then dirname(filename) */
{
	int d;
	size_t len;
	char * path = NULL;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: _include_path(%p, %s)\n", cpp, str);
#endif
	if(str[0] == '"')
		d = str[0];
	else if(str[0] == '<')
		d = '>';
	else
		return NULL;
	len = strlen(str);
	if(len < 3 || str[len - 1] != d)
		return NULL;
	if((path = strdup(&str[1])) == NULL)
		return NULL;
	path[len - 2] = '\0';
	p = _path_lookup(cpp, token, path, d == '>');
	free(path);
	return p;
}

static char * _lookup_error(Token * token, char const * path, int system);
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

static int _directive_undef(Cpp * cpp, Token * token, char const * str)
{
	cpp_define_remove(cpp, str); /* FIXME may not be just a word */
	token_set_code(token, CPP_CODE_META_UNDEF);
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
static int _cpp_callback_word(Parser * parser, Token * token, int c,
		void * data)
{
	char * str;

	if(!_cpp_isword(c))
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: cpp_callback_word('%c')\n", c);
#endif
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
	char * p;

	if((cpp = object_new(sizeof(*cpp))) == NULL)
		return NULL;
	cpp->filters = filters;
	cpp->parser = parser_new(filename);
	cpp->newlines_last_cnt = 0;
	cpp->trigraphs_last_cnt = 0;
	cpp->directive_newline = 1;
	cpp->subparser = NULL;
	cpp->paths = NULL;
	cpp->paths_cnt = 0;
	cpp->defines = NULL;
	cpp->defines_cnt = 0;
	if((p = strdup(filename)) != NULL)
	{
		cpp_path_add(cpp, dirname(p)); /* FIXME inclusion order */
		free(p);
	}
	if(cpp->parser == NULL || cpp->paths_cnt != 1)
	{
		cpp_delete(cpp);
		return NULL;
	}
	parser_add_filter(cpp->parser, _cpp_filter_newlines, cpp);
	if(cpp->filters & CPP_FILTER_TRIGRAPH)
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
	if(cpp->subparser != NULL)
		cpp_delete(cpp->subparser);
	if(cpp->parser != NULL)
		parser_delete(cpp->parser);
	object_delete(cpp);
}


/* accessors */
/* cpp_get_filename */
char const * cpp_get_filename(Cpp * cpp)
{
	if(cpp->subparser != NULL)
		return cpp_get_filename(cpp->subparser);
	return parser_get_filename(cpp->parser);
}


/* useful */
/* cpp_define_add */
int cpp_define_add(Cpp * cpp, char const * name, char const * value)
{
	char ** p;

	if((p = realloc(cpp->defines, sizeof(*p) * (cpp->defines_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->defines = p;
	if((p[cpp->defines_cnt] = strdup(name)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->defines_cnt++;
	return 0;
}


/* cpp_define_remove */
int cpp_define_remove(Cpp * cpp, char const * name)
{
	size_t i;

	for(i = 0; i < cpp->defines_cnt; i++)
		if(strcmp(cpp->defines[i], name) == 0)
			break;
	if(i == cpp->defines_cnt)
		return 1;
	free(cpp->defines[i]);
	cpp->defines_cnt--;
	for(; i < cpp->defines_cnt; i++)
		cpp->defines[i] = cpp->defines[i + 1];
	return 0;
}


/* cpp_path_add */
int cpp_path_add(Cpp * cpp, char const * path)
{
	char ** p;

	if((p = realloc(cpp->paths, sizeof(*p) * (cpp->paths_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths = p;
	if((p[cpp->paths_cnt] = strdup(path)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->paths_cnt++;
	return 0;
}


/* cpp_scan */
int cpp_scan(Cpp * cpp, Token ** token)
{
	if(cpp->subparser != NULL)
	{
		if(cpp_scan(cpp->subparser, token) != 0)
			return 1;
		if(*token != NULL)
			return 0;
		cpp_delete(cpp->subparser); /* end of file */
		cpp->subparser = NULL;
	}
	return parser_get_token(cpp->parser, token);
}
