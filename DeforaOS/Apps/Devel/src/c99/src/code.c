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
/* TODO:
 * - scope */



#include <System.h>
#include <sys/utsname.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include "target/target.h"
#include "code.h"
#include "../config.h"


/* private */
/* types */
typedef struct _CodeType
{
	char * name;
} CodeType;

typedef struct _CodeVariable
{
	CodeType * type;
	char * name;
} CodeVariable;


/* prototypes */
static int _variable_add(Code * code, char const * name);

static int _code_add_function(Code * code, char const * name);


/* protected */
/* types */
struct _Code
{
	CodeContext context;
	/* types */
	CodeType * types;
	unsigned int types_cnt;
	CodeVariable * variables;
	unsigned int variables_cnt;
	/* target */
	Target * target;
};


/* private */
/* functions */
/* code_add_function */
static int _code_add_function(Code * code, char const * name)
{
	return _variable_add(code, name);
}


/* variable_add */
static int _variable_add(Code * code, char const * name)
{
	CodeVariable * p;

	if((p = realloc(code->variables, sizeof(*p)
					* (code->variables_cnt + 1))) == NULL)
		return 1; /* FIXME report error */
	code->variables = p;
	p = &code->variables[code->variables_cnt];
	p->name = strdup(name);
	p->type = NULL; /* FIXME implement */
	if(p->name == NULL)
		return 1; /* FIXME report error */
	code->variables_cnt++;
	return 0;
}


/* public */
/* functions */
/* code_new */
static Target * _new_target(char const * target);

Code * code_new(char const * target)
{
	Code * code;

	if((code = object_new(sizeof(*code))) == NULL)
		return NULL;
	code->context = CODE_CONTEXT_UNDEFINED;
	code->types = NULL;
	code->types_cnt = 0;
	code->variables = NULL;
	code->variables_cnt = 0;
	if((code->target = _new_target(target)) == NULL)
	{
		code_delete(code);
		return NULL;
	}
	return code;
}

static Target * _new_target(char const * target)
{
	struct utsname * uts;

	if(target == NULL)
	{
		if(uname(uts) != 0)
		{
			error_set_code(1, "%s", strerror(errno));
			return NULL;
		}
		target = uts->machine;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, target);
#endif
	}
	return plugin_new(LIBDIR, PACKAGE, "target", target);
}


/* code_delete */
void code_delete(Code * code)
{
	free(code->types);
	object_delete(code);
}


/* accessors */
/* code_set_context */
int code_set_context(Code * code, CodeContext context)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(0x%x)\n", __func__, context);
#endif
	code->context = context;
	return 0;
}


/* code_set_identifier */
int code_set_identifier(Code * code, char const * name)
{
	switch(code->context)
	{
		case CODE_CONTEXT_FUNCTION_NAME:
			return _code_add_function(code, name);
		case CODE_CONTEXT_UNDEFINED:
		default:
			break;
	}
	return 1;
}


/* useful */
/* code_is_type */
int code_is_type(Code * code, char const * name)
{
	unsigned int i;

	/* XXX use a hash table if it gets too slow */
	for(i = 0; i < code->types_cnt; i++)
		if(strcmp(code->types[i].name, name) == 0)
			return 1;
	return 0;
}
