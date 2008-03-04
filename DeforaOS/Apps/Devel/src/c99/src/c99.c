/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tokenset.h"
#include "c99.h"


/* private */
/* types */
struct _C99
{
	int flags;			/* FIXME get rid of it */
	FILE * outfp;
	Cpp * cpp;
	char * outfile;
	int optlevel;
};

typedef struct _C99Operator
{
	C99Code code;
	char const * string;
} C99Operator;


/* variables */
static C99Operator _operators[] =
{
	{ C99_CODE_KEYWORD_AUTO,	"auto"		},
	{ C99_CODE_KEYWORD_BREAK,	"break"		},
	{ C99_CODE_KEYWORD_CASE,	"case"		},
	{ C99_CODE_KEYWORD_CHAR,	"char"		},
	{ C99_CODE_KEYWORD_CONST,	"const"		},
	{ C99_CODE_KEYWORD_CONTINUE,	"continue"	},
	{ C99_CODE_KEYWORD_DEFAULT,	"default"	},
	{ C99_CODE_KEYWORD_DO,		"do"		},
	{ C99_CODE_KEYWORD_DOUBLE,	"double"	},
	{ C99_CODE_KEYWORD_ELSE,	"else"		},
	{ C99_CODE_KEYWORD_ENUM,	"enum"		},
	{ C99_CODE_KEYWORD_EXTERN,	"extern"	},
	{ C99_CODE_KEYWORD_FLOAT,	"float"		},
	{ C99_CODE_KEYWORD_FOR,		"for"		},
	{ C99_CODE_KEYWORD_GOTO,	"goto"		},
	{ C99_CODE_KEYWORD_IF,		"if"		},
	{ C99_CODE_KEYWORD_INLINE,	"inline"	},
	{ C99_CODE_KEYWORD_INT,		"int"		},
	{ C99_CODE_KEYWORD_LONG,	"long"		},
	{ C99_CODE_KEYWORD_REGISTER,	"register"	},
	{ C99_CODE_KEYWORD_RESTRICT,	"restrict"	},
	{ C99_CODE_KEYWORD_RETURN,	"return"	},
	{ C99_CODE_KEYWORD_SHORT,	"short"		},
	{ C99_CODE_KEYWORD_SIGNED,	"signed"	},
	{ C99_CODE_KEYWORD_SIZEOF,	"sizeof"	},
	{ C99_CODE_KEYWORD_STATIC,	"static"	},
	{ C99_CODE_KEYWORD_STRUCT,	"struct"	},
	{ C99_CODE_KEYWORD_SWITCH,	"switch"	},
	{ C99_CODE_KEYWORD_TYPEDEF,	"typedef"	},
	{ C99_CODE_KEYWORD_UNION,	"union"		},
	{ C99_CODE_KEYWORD_UNSIGNED,	"unsigned"	},
	{ C99_CODE_KEYWORD_VOID,	"void"		},
	{ C99_CODE_KEYWORD_VOLATILE,	"volatile"	},
	{ C99_CODE_KEYWORD_WHILE,	"while"		},
	{ C99_CODE_KEYWORD__BOOL,	"_Bool"		},
	{ C99_CODE_KEYWORD__COMPLEX,	"_Complex"	},
	{ C99_CODE_KEYWORD__IMAGINARY,	"_Imaginary"	}
};
static size_t _operators_cnt = sizeof(_operators) / sizeof(*_operators);


/* public */
/* functions */
/* c99_new */
static char * _new_outfile(int flags, char const * outfile,
		char const * pathname);

C99 * c99_new(C99Prefs * prefs, char const * pathname)
{
	C99 * c99;
	size_t i;
	size_t j;
	size_t k;

	if((c99 = object_new(sizeof(*c99))) == NULL)
		return NULL;
	c99->flags = prefs->flags;
	if((c99->cpp = cpp_new(pathname, CPP_FILTER_TRIGRAPH)) == NULL)
	{
		object_delete(c99);
		return NULL;
	}
	for(i = 0; i < prefs->paths_cnt; i++)
		if(cpp_path_add(c99->cpp, prefs->paths[i]) != 0)
			break;
	for(j = 0; j < prefs->defines_cnt; j++)
		if(cpp_define_add(c99->cpp, prefs->defines[j], NULL) != 0)
			break;
	for(k = 0; k < prefs->undefines_cnt; k++)
		if(cpp_define_remove(c99->cpp, prefs->undefines[k]) != 0)
			break;
	c99->outfile = _new_outfile(prefs->flags, prefs->outfile, pathname);
	if(c99->outfile != NULL)
		c99->outfp = (c99->outfile[0] == '\0') ? stdout
			: fopen(c99->outfile, "w");
	else
		c99->outfp = NULL;
	c99->optlevel = prefs->optlevel;
	if(c99->outfile == NULL
			|| c99->outfp == NULL
			|| i != prefs->paths_cnt
			|| j != prefs->defines_cnt
			|| k != prefs->undefines_cnt)
	{
		c99_delete(c99);
		return NULL;
	}
	return c99;
}

static char * _new_outfile(int flags, char const * outfile,
		char const * pathname)
{
	char * ret;
	size_t len;

	if(flags & C99PREFS_c && pathname != NULL)
	{
		if((len = strlen(pathname)) < 3 || pathname[len - 2] != '.'
				|| pathname[len - 1] != 'c')
		{
			error_set_code(1, "%s", strerror(EINVAL));
			return NULL;
		}
		if((ret = strdup(pathname)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			return NULL;
		}
		ret[len - 1] = 'o';
		return ret;
	}
	if(flags & C99PREFS_E && outfile == NULL)
		outfile = "";
	if((ret = strdup(outfile != NULL ? outfile : "a.out")) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	return ret;
}


/* c99_delete */
int c99_delete(C99 * c99)
{
	int ret = 0;

	cpp_delete(c99->cpp);
	if(c99->outfp != NULL && fclose(c99->outfp) != 0)
		ret = error_set_code(1, "%s: %s", c99->outfile,
				strerror(errno));
	free(c99->outfile);
	object_delete(c99);
	return ret;
}


/* useful */
/* c99_parse */
static int _parse_E(C99 * c99);

int c99_parse(C99 * c99)
{
	if(c99->flags & C99PREFS_E)
		return _parse_E(c99);
	/* FIXME implement */
	return error_set_code(1, "%s", strerror(ENOSYS));
}

static int _parse_E(C99 * c99)
{
	int ret;
	Token * token;
	int code;

	while((ret = cpp_scan(c99->cpp, &token)) == 0
			&& token != NULL)
	{
		if((code = token_get_code(token)) == CPP_CODE_META_ERROR
				|| code == CPP_CODE_META_WARNING)
			fprintf(stderr, "%s%s%s%s%u%s%s\n",
					code == CPP_CODE_META_ERROR
					? "Error" : "Warning", " in ",
					token_get_filename(token), ":",
					token_get_line(token), ": ",
					token_get_string(token));
		else if(code >= CPP_CODE_META_FIRST
				&& code <= CPP_CODE_META_LAST)
			fprintf(c99->outfp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), c99->outfp);
		token_delete(token);
	}
	return ret;
}


/* c99_scan */
int c99_scan(C99 * c99, Token ** token)
{
	int ret;
	char const * string;
	size_t i;

	if((ret = cpp_scan(c99->cpp, token)) != 0)
		return ret;
	if(token == NULL)
		return 0;
	if(token_get_code(*token) != C99_CODE_WORD)
		return 0;
	if((string = token_get_string(*token)) == NULL)
		return 0;
	for(i = 0; i < _operators_cnt; i++)
		if(strcmp(_operators[i].string, string) == 0)
		{
			token_set_code(*token, _operators[i].code);
			return 0;
		}
	return 0;
}
