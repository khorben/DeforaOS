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



#include <string.h>
#include <ctype.h>
#include "common.h"
#include "c99.h"


/* private */
/* types */
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
/* useful */
/* c99_scan */
int c99_scan(C99 * c99)
	/* FIXME handle numbers too */
{
	int ret;
	int code;
	char const * string;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(c99->token != NULL)
		token_delete(c99->token);
	/* skip white-space and meta tokens */
	while((ret = cpp_scan(c99->cpp, &c99->token)) == 0)
	{
		if((code = token_get_code(c99->token)) == C99_CODE_WHITESPACE
				|| (code >= C99_CODE_META_FIRST
					&& code <= C99_CODE_META_LAST))
			continue;
		break;
	}
	if(ret != 0)
		return ret;
	if(c99->token == NULL)
		return 0;
	if(token_get_code(c99->token) != C99_CODE_WORD)
		return 0;
	if((string = token_get_string(c99->token)) == NULL)
		return 0;
	for(i = 0; i < _operators_cnt; i++)
		if(strcmp(_operators[i].string, string) == 0)
		{
			token_set_code(c99->token, _operators[i].code);
			return 0;
		}
	/* FIXME complete farther distinctions */
	if(isalpha(string[0]))
		token_set_code(c99->token, C99_CODE_IDENTIFIER);
	return 0;
}
