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
static int _code_target_init(Code * code, char const * outfile, int optlevel);
static int _code_target_exit(Code * code);
static int _code_target_function(Code * code, char const * name);

static int _variable_add(Code * code, char const * name);

static int _code_function(Code * code, char const * name);


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
	Plugin * plugin;
	TargetPlugin * target;
};


/* private */
/* functions */
/* code_target_init */
static int _code_target_init(Code * code, char const * outfile, int optlevel)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->target->init == NULL)
		return 0;
	return code->target->init(outfile, optlevel);
}


/* code_target_exit */
static int _code_target_exit(Code * code)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->target->exit == NULL)
		return 0;
	return code->target->exit();
}


/* code_target_function */
static int _code_target_function(Code * code, char const * name)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->target->function == NULL)
		return 0;
	return code->target->function(name);
}


/* code_function */
static int _code_function(Code * code, char const * name)
{
	int ret;

	if((ret = _variable_add(code, name)) != 0)
		return ret;
	return _code_target_function(code, name);
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
static int _new_target(Code * code, char const * target,
		C99Option const * options, size_t options_cnt);

Code * code_new(C99Prefs const * prefs, char const * outfile)
{
	Code * code;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, prefs, outfile);
#endif
	if((code = object_new(sizeof(*code))) == NULL)
		return NULL;
	memset(code, 0, sizeof(*code));
	code->context = CODE_CONTEXT_UNDEFINED;
	if(_new_target(code, prefs->target, prefs->options, prefs->options_cnt)
			!= 0
			|| _code_target_init(code, outfile, prefs->optlevel)
			!= 0)
	{
		code_delete(code);
		return NULL;
	}
	return code;
}

static int _new_target(Code * code, char const * target,
		C99Option const * options, size_t options_cnt)
{
	C99Option const * p;
	size_t i;
	size_t j;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%zu)\n", __func__, options_cnt);
#endif
	if(target == NULL)
		target = "as";
	if((code->plugin = plugin_new(LIBDIR, PACKAGE, "target", target))
			== NULL
			|| (code->target = plugin_lookup(code->plugin,
					"target_plugin")) == NULL)
		return 1;
	if(code->target->options == NULL)
	{
		if(options_cnt == 0)
			return 0;
		return error_set_code(1, "%s", "Target supports no options");
	}
	for(i = 0; i < options_cnt; i++)
	{
		p = &options[i];
#ifdef DEBUG
		fprintf(stderr, "DEBUG: option \"%s\"\n", p->name);
#endif
		for(j = 0; code->target->options[j].name != NULL; j++)
			if(strcmp(p->name, code->target->options[j].name) == 0)
				break;
		if(code->target->options[j].name == NULL)
			break;
		code->target->options[j].value = p->value;
	}
	if(i == options_cnt)
		return 0;
	code->target = NULL;
	return error_set_code(1, "%s: %s%s%s", target, "Unknown option \"",
			p->name, "\" for target");
}


/* code_delete */
int code_delete(Code * code)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->plugin != NULL)
	{
		if(code->target != NULL)
			ret = _code_target_exit(code);
		plugin_delete(code->plugin);
	}
	free(code->variables);
	free(code->types);
	object_delete(code);
	return ret;
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
			return _code_function(code, name);
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
