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
static int _code_target_function_begin(Code * code, char const * name);
static int _code_target_function_call(Code * code, char const * name);
static int _code_target_function_end(Code * code);


/* protected */
/* types */
struct _Code
{
	/* types */
	CodeType * types;
	size_t types_cnt;
	CodeVariable * variables;
	size_t variables_cnt;
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


/* code_target_function_begin */
static int _code_target_function_begin(Code * code, char const * name)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if(code->target->function_begin == NULL)
		return 0;
	return code->target->function_begin(name);
}


/* code_target_function_call */
static int _code_target_function_call(Code * code, char const * name)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if(code->target->function_call == NULL)
		return 0;
	return code->target->function_call(name);
}


/* code_target_function_end */
static int _code_target_function_end(Code * code)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->target->function_end == NULL)
		return 0;
	return code->target->function_end();
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
	fprintf(stderr, "DEBUG: %s(%s, %zu)\n", __func__, target ? target
			: "NULL", options_cnt);
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
		fprintf(stderr, "DEBUG: %s() option \"%s\"\n", __func__,
				p->name);
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
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(code->plugin != NULL)
	{
		if(code->target != NULL)
			ret = _code_target_exit(code);
		plugin_delete(code->plugin);
	}
	for(i = 0; i < code->variables_cnt; i++)
		free(code->variables[i].name);
	free(code->variables);
	for(i = 0; i < code->types_cnt; i++)
		free(code->types[i].name);
	free(code->types);
	object_delete(code);
	return ret;
}


/* useful */
/* functions */
/* code_function_begin */
int code_function_begin(Code * code, char const * name)
{
	int ret;

	if((ret = code_variable_add(code, name)) != 0)
		return ret;
	return _code_target_function_begin(code, name);
}


/* code_function_call */
int code_function_call(Code * code, char const * name)
{
#if 0 /* FIXME disabled for now */
	int ret;

	if((ret = _variable_get(code, name)) < 0)
		return -ret;
#endif
	return _code_target_function_call(code, name);
}


/* code_function_end */
int code_function_end(Code * code)
{
	return _code_target_function_end(code);
}


/* types */
/* code_type_add */
int code_type_add(Code * code, char const * name)
{
	CodeType * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, name);
#endif
	if(name == NULL || name[0] == '\0')
		return error_set_code(1, "%s", "Invalid name for a type");
	if((p = realloc(code->types, sizeof(*p) * (code->types_cnt + 1)))
			== NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->types = p;
	if((code->types[code->types_cnt].name = strdup(name)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->types_cnt++;
	return 0;
}


/* code_type_get */
int code_type_get(Code * code, char const * name)
{
	size_t i;

	/* XXX use a hash table if it gets too slow */
	for(i = 0; i < code->types_cnt; i++)
		if(strcmp(code->types[i].name, name) == 0)
			return i;
	return -error_set_code(1, "%s%s", "Unknown type ", name);
}


/* variables */
/* code_variable_add */
int code_variable_add(Code * code, char const * name)
{
	CodeVariable * p;

	if(name == NULL || name[0] == '\0')
		return error_set_code(1, "%s", "Invalid name for a variable");
	if((p = realloc(code->variables, sizeof(*p)
					* (code->variables_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->variables = p;
	p = &code->variables[code->variables_cnt];
	p->name = strdup(name);
	p->type = NULL; /* FIXME implement */
	if(p->name == NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->variables_cnt++;
	return 0;
}


/* code_variable_get */
int code_variable_get(Code * code, char const * name)
{
	size_t i;

	for(i = 0; i < code->variables_cnt; i++)
		if(strcmp(code->variables[i].name, name) == 0)
			return i;
	return -error_set_code(1, "%s%s", "Unknown variable ", name);
}
