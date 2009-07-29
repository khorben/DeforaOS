/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
 * - potential memory leak with tokens' data */



#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "common.h"

#ifdef DEBUG
# define DEBUG_SCOPE() fprintf(stderr, "DEBUG: %s(%p) %zu scope=%d\n", \
		__func__, cpp, _cpp_scope_get_count(cpp), _cpp_scope_get(cpp))
#else
# define DEBUG_SCOPE()
#endif


/* private */
/* prototypes */
/* scope */
static int _cpp_scope_push(Cpp * cpp, CppScope scope);
static CppScope _cpp_scope_get(Cpp * cpp);
static size_t _cpp_scope_get_count(Cpp * cpp);
static void _cpp_scope_set(Cpp * cpp, CppScope scope);
static int _cpp_scope_pop(Cpp * cpp);


/* functions */
/* scope */
/* cpp_scope_push */
static int _cpp_scope_push(Cpp * cpp, CppScope scope)
{
	CppScope * p;

	if(_cpp_scope_get(cpp) != CPP_SCOPE_TAKING)
		scope = CPP_SCOPE_TAKEN;
	if((p = realloc(cpp->scopes, sizeof(*p) * (cpp->scopes_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->scopes = p;
	p[cpp->scopes_cnt++] = scope;
	return 0;
}


/* cpp_scope_get */
static CppScope _cpp_scope_get(Cpp * cpp)
{
	return (cpp->scopes_cnt == 0) ? CPP_SCOPE_TAKING
		: cpp->scopes[cpp->scopes_cnt - 1];
}


/* cpp_scope_get_count */
static size_t _cpp_scope_get_count(Cpp * cpp)
{
	return cpp->scopes_cnt;
}


/* cpp_scope_set */
static void _cpp_scope_set(Cpp * cpp, CppScope scope)
{
	assert(cpp->scopes_cnt > 0);
	cpp->scopes[cpp->scopes_cnt - 1] = scope;
}


/* cpp_scope_pop */
static int _cpp_scope_pop(Cpp * cpp)
{
	CppScope * p;

	assert(cpp->scopes_cnt > 0);
	if(cpp->scopes_cnt == 1)
	{
		free(cpp->scopes);
		p = NULL;
	}
	else if((p = realloc(cpp->scopes, sizeof(*p) * (cpp->scopes_cnt - 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	cpp->scopes = p;
	cpp->scopes_cnt--;
	return 0;
}


/* public */
/* cpp_scan */
static void _scan_meta_line(Token * token);
static int _scan_ifdef(Cpp * cpp, Token ** token);
static int _scan_ifndef(Cpp * cpp, Token ** token);
static int _scan_if(Cpp * cpp, Token ** token);
static int _scan_elif(Cpp * cpp, Token ** token);
static int _scan_else(Cpp * cpp, Token ** token);
static int _scan_endif(Cpp * cpp, Token ** token);
static int _scan_define(Cpp * cpp, Token ** token);
static int _scan_undef(Cpp * cpp, Token ** token);

int cpp_scan(Cpp * cpp, Token ** token)
{
	int ret;
	TokenCode code;
	char const * str;

	for(; (ret = cppparser_scan(cpp->parser, token)) == 0;
			token_delete(*token))
	{
		if(*token == NULL) /* end of file */
			break;
		if((code = token_get_code(*token)) >= CPP_CODE_META_FIRST
				&& code <= CPP_CODE_META_LAST)
			_scan_meta_line(*token);
		switch(code)
		{
			case CPP_CODE_META_IFDEF:
				return _scan_ifdef(cpp, token);
			case CPP_CODE_META_IFNDEF:
				return _scan_ifndef(cpp, token);
			case CPP_CODE_META_IF:
				return _scan_if(cpp, token);
			case CPP_CODE_META_ELIF:
				return _scan_elif(cpp, token);
			case CPP_CODE_META_ELSE:
				return _scan_else(cpp, token);
			case CPP_CODE_META_ENDIF:
				return _scan_endif(cpp, token);
			default:
				break;
		}
		if(_cpp_scope_get(cpp) != CPP_SCOPE_TAKING) /* not in scope */
			continue;
		switch(code)
		{
			case CPP_CODE_META_DEFINE:
				return _scan_define(cpp, token);
			case CPP_CODE_META_UNDEF:
				return _scan_undef(cpp, token);
			case CPP_CODE_WORD:
				str = token_get_string(*token);
				if((str = cpp_define_get(cpp, str)) != NULL)
				{
					if(cppparser_inject(cpp->parser, str)
							!= 0)
						return -1;
					continue;
				}
				return 0;
			default:
				return 0;
		}
	}
	return ret;
}

static void _scan_meta_line(Token * token)
{
	/* decrement the line number, not so elegant but works */
	token_set_line(token, token_get_line(token) - 1);
}

static int _scan_ifdef(Cpp * cpp, Token ** token)
{
	char * name;
	int take;

	DEBUG_SCOPE();
	name = token_get_data(*token);
	take = (cpp_define_get(cpp, name) != NULL) ? 1 : 0;
	token_set_data(*token, NULL);
	free(name);
	_cpp_scope_push(cpp, take ? CPP_SCOPE_TAKING : CPP_SCOPE_NOTYET);
	return 0;
}

static int _scan_ifndef(Cpp * cpp, Token ** token)
{
	char * name;
	int take;

	DEBUG_SCOPE();
	name = token_get_data(*token);
	take = (cpp_define_get(cpp, name) == NULL) ? 1 : 0;
	token_set_data(*token, NULL);
	free(name);
	_cpp_scope_push(cpp, take ? CPP_SCOPE_TAKING : CPP_SCOPE_NOTYET);
	return 0;
}

static int _scan_if(Cpp * cpp, Token ** token)
{
	DEBUG_SCOPE();
	/* FIXME check the condition */
	_cpp_scope_push(cpp, CPP_SCOPE_TAKING);
	return 0;
}

static int _scan_elif(Cpp * cpp, Token ** token)
{
	CppScope scope;

	DEBUG_SCOPE();
	if(_cpp_scope_get_count(cpp) == 0)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, "#elif without #if or #ifdef"
				" or #ifndef");
		return 0;
	}
	scope = _cpp_scope_get(cpp);
	if(scope == CPP_SCOPE_TAKING)
		_cpp_scope_set(cpp, CPP_SCOPE_TAKEN);
	else if(scope == CPP_SCOPE_NOTYET)
		/* FIXME check the condition */
		_cpp_scope_set(cpp, CPP_SCOPE_TAKING);
	return 0;
}

static int _scan_else(Cpp * cpp, Token ** token)
{
	CppScope scope;

	DEBUG_SCOPE();
	if(_cpp_scope_get_count(cpp) == 0)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, "#else without #if or #ifdef"
				" or #ifndef");
		return 0;
	}
	scope = _cpp_scope_get(cpp);
	if(scope == CPP_SCOPE_TAKING)
		_cpp_scope_set(cpp, CPP_SCOPE_TAKEN);
	else if(scope == CPP_SCOPE_NOTYET)
		_cpp_scope_set(cpp, CPP_SCOPE_TAKING);
	return 0;
}

static int _scan_endif(Cpp * cpp, Token ** token)
{
	DEBUG_SCOPE();
	if(_cpp_scope_get_count(cpp) == 0)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, "#endif without #if or #ifdef"
				" or #ifndef");
	}
	_cpp_scope_pop(cpp);
	return 0;
}

static int _scan_define(Cpp * cpp, Token ** token)
{
	char * str;
	int tmp;
	size_t i;
	size_t j;
	size_t k = 0;
	char * var;
	char const * val;

	str = token_get_data(*token);
	/* fetch variable name */
	for(i = 1; (tmp = str[i]) != '\0' && !isspace(tmp); i++)
	{
		/* FIXME actually implement macros */
		if(str[i] != '(')
			continue;
		for(k = i; str[i] != '\0' && str[i] != ')'; i++);
		if(str[i] == ')')
			i++;
		break;
	}
	/* skip white-spaces and fetch value */
	for(j = i; (tmp = str[j]) != '\0' && isspace(tmp); j++);
	val = (str[j] != '\0') ? &str[j] : NULL;
	if((var = strdup(str)) == NULL)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, strerror(errno));
		token_set_data(*token, NULL);
		free(str);
		return 0;
	}
	var[k != 0 ? k : i] = '\0';
	if(cpp_define_add(cpp, var, val) != 0)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, error_get());
	}
	free(var);
	token_set_data(*token, NULL);
	free(str);
	return 0;
}

static int _scan_undef(Cpp * cpp, Token ** token)
	/* FIXME ignores what's after the spaces after the variable name */
{
	char * str;
	int tmp;
	size_t i;
	char * var;

	str = token_get_data(*token);
	/* fetch variable name */
	for(i = 1; (tmp = str[i]) != '\0' && !isspace(tmp); i++);
	if((var = strdup(str)) == NULL)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, strerror(errno));
		free(str);
		return 0;
	}
	var[i] = '\0';
	if(cpp_define_remove(cpp, var) != 0)
	{
		token_set_code(*token, CPP_CODE_META_ERROR);
		token_set_string(*token, error_get());
	}
	free(var);
	free(str);
	return 0;
}
