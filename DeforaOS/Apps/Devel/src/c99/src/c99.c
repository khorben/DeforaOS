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
/* FIXME:
 * - place main() in main.c? */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tokenset.h"
#include "c99.h"
#include "../config.h"


/* private */
/* types */
typedef struct _Prefs
{
	int flags;
	char const * outfile;
	const char ** paths;
	size_t paths_cnt;
	char ** defines;
	size_t defines_cnt;
	const char ** undefines;
	size_t undefines_cnt;
	int optlevel;
} Prefs;
#define PREFS_c 0x1
#define PREFS_E 0x2
#define PREFS_g 0x4
#define PREFS_s 0x8

typedef struct _C99Operator
{
	C99Code code;
	char const * string;
} C99Operator;


/* prototypes */
static int _c99(Prefs * prefs, int filec, char * filev[]);


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


/* functions */
/* c99 */
static int _c99_do(Prefs * prefs, FILE * outfp, char const * infile);

static int _c99(Prefs * prefs, int filec, char * filev[])
{
	FILE * fp = stdout;
	int ret = 0;
	int i;

	if(prefs->outfile != NULL && (fp = fopen(prefs->outfile, "w")) == NULL)
		return error_set_print(PACKAGE, 1, "%s: %s", prefs->outfile,
				strerror(errno));
	for(i = 0; i < filec; i++)
		ret |= _c99_do(prefs, fp, filev[i]);
	if(fp != NULL && fclose(fp) != 0)
		return error_set_print(PACKAGE, 1, "%s: %s", prefs->outfile,
				strerror(errno));
	if(ret != 0)
		error_print(PACKAGE);
	return ret;
}

static int _c99_do_c(Prefs * prefs, FILE * outfp, char const * infile);
static int _c99_do_E(Prefs * prefs, FILE * outfp, char const * infile);
static int _c99_do_o(Prefs * prefs, FILE * outfp, char const * infile,
		FILE * infp);
static int _c99_do(Prefs * prefs, FILE * outfp, char const * infile)
{
	FILE * infp;
	int ret;

	if(prefs->flags & PREFS_c)
		return _c99_do_c(prefs, outfp, infile);
	else if(prefs->flags & PREFS_E)
		return _c99_do_E(prefs, outfp, infile);
	if((infp = fopen(infile, "r")) == NULL)
		return error_set_code(1, "%s: %s", infile, strerror(errno));
	else
		ret = _c99_do_o(prefs, outfp, infile, infp);
	/* FIXME implement */
	if(fclose(infp) != 0 && ret == 0)
		return error_set_code(1, "%s: %s", infile, strerror(errno));
	return ret;
}

static int _c99_do_c(Prefs * prefs, FILE * outfp, char const * infile)
{
	int ret = 0;
	size_t len;
	char * o = NULL;

	if(prefs->outfile == NULL)
	{
		if((len = strlen(infile)) < 3 || infile[len - 2] != '.'
				|| infile[len - 1] != 'c')
			return error_set_code(1, "%s", strerror(EINVAL));
		if((o = strdup(infile)) == NULL)
			return error_set_code(1, "%s", strerror(errno));
		o[len - 1] = 'o';
		if((outfp = fopen(o, "w")) == NULL)
		{
			error_set_code(1, "%s: %s", o, strerror(errno));
			free(o);
			return 1;
		}
	}
	/* FIXME implement */
	if(o != NULL)
	{
		if(fclose(outfp) != 0)
			ret = error_set_code(1, "%s: %s", o, strerror(errno));
		free(o);
	}
	return ret;
}

static int _c99_do_E(Prefs * prefs, FILE * outfp, char const * infile)
{
	int ret;
	Cpp * cpp;
	size_t i;
	size_t j;
	size_t k;
	Token * token;
	int code;

	if((cpp = cpp_new(infile, CPP_FILTER_TRIGRAPH)) == NULL)
		return 1;
	for(i = 0; i < prefs->paths_cnt; i++)
		if(cpp_path_add(cpp, prefs->paths[i]) != 0)
			break;
	for(j = 0; j < prefs->defines_cnt; j++)
		if(cpp_define_add(cpp, prefs->defines[j], NULL) != 0)
			break;
	for(k = 0; k < prefs->undefines_cnt; k++)
		if(cpp_define_remove(cpp, prefs->undefines[k]) != 0)
			break;
	if(i != prefs->paths_cnt || j != prefs->defines_cnt
			|| k != prefs->undefines_cnt)
	{
		cpp_delete(cpp);
		return 1;
	}
	while((ret = cpp_scan(cpp, &token)) == 0
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
		else if(code >= CPP_CODE_META_FIRST && code <= CPP_CODE_META_LAST)
			fprintf(outfp, "%s\n", token_get_string(token));
		else
			fputs(token_get_string(token), outfp);
	}
	cpp_delete(cpp);
	return ret;
}

static int _c99_do_o(Prefs * prefs, FILE * outfp, char const * infile,
		FILE * infp)
{
	/* FIXME implement */
	return error_set_code(1, "%s", strerror(ENOSYS));
}


/* useful */
/* c99_scan */
int c99_scan(C99 * c99, Token ** token)
{
	int ret;
	char const * string;
	size_t i;

	if((ret = cpp_scan(c99, token)) != 0)
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


/* usage */
static int _usage(void)
{
	fputs("Usage: c99 [-c][-D name[=value]]...[-E][-g][-I directory]"
"[-L directory][-o outfile][-Ooptlevel][-s][-U name]... operand ...\n", stderr);
	return 1;
}


/* public */
/* main */
static int _main_add_define(Prefs * prefs, char * define);
static int _main_add_path(Prefs * prefs, char const * path);
static int _main_add_undefine(Prefs * prefs, char const * undefine);

int main(int argc, char * argv[])
{
	Prefs prefs;
	int o;

	memset(&prefs, 0, sizeof(prefs));
	while((o = getopt(argc, argv, "cD:EgI:L:o:O123sU:")) != -1)
		switch(o)
		{
			case 'c':
				prefs.flags |= PREFS_c;
				break;
			case 'D':
				if(_main_add_define(&prefs, optarg) != 0)
					return 2;
				break;
			case 'E':
				prefs.flags |= PREFS_E;
				break;
			case 'I':
				if(_main_add_path(&prefs, optarg) != 0)
					return 2;
				break;
			case 'g':
				prefs.flags |= PREFS_g;
				break;
			case 'o':
				prefs.outfile = optarg;
				break;
			case 'O':
				prefs.optlevel++;
				break;
			case 's':
				prefs.flags |= PREFS_s;
				break;
			case 'U':
				if(_main_add_undefine(&prefs, optarg) != 0)
					return 2;
			case '1':
			case '2':
			case '3':
				prefs.optlevel = o - '0';
				break;
			default:
				return _usage();
		}
	if(optind == argc)
		return _usage();
	if(prefs.flags & PREFS_c && prefs.outfile != NULL && optind + 1 != argc)
		return _usage();
	return _c99(&prefs, argc - optind, &argv[optind]) == 0 ? 0 : 2;
}

static int _main_add_define(Prefs * prefs, char * define)
{
	char ** p;
	char * value;

	if(strlen(define) == 0)
		return 1;
	value = strtok(define, "=");
	if((p = realloc(prefs->defines, sizeof(*p) * (prefs->defines_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->defines = p;
	prefs->defines[prefs->defines_cnt++] = define;
	return 0;
}

static int _main_add_path(Prefs * prefs, char const * path)
{
	const char ** p;

	if((p = realloc(prefs->paths, sizeof(*p) * (prefs->paths_cnt + 1)))
			== NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->paths = p;
	prefs->paths[prefs->paths_cnt++] = path;
	return 0;
}

static int _main_add_undefine(Prefs * prefs, char const * undefine)
{
	const char ** p;

	if(strlen(undefine) == 0)
		return 1;
	if((p = realloc(prefs->undefines, sizeof(*p)
					* (prefs->undefines_cnt + 1))) == NULL)
		return error_set_print(PACKAGE, 1, "%s", strerror(errno));
	prefs->undefines = p;
	prefs->undefines[prefs->undefines_cnt++] = undefine;
	return 0;
}
