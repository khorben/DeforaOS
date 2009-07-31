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



#include <System.h>
#include <sys/utsname.h>
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <errno.h>
#include "c99/target.h"
#include "code.h"
#include "../config.h"

#ifdef DEBUG
# define DEBUG_FUNC()  fprintf(stderr, "DEBUG: %s()\n", __func__);
# define DEBUG_SCOPE() fprintf(stderr, "DEBUG: %s() %zu\n", __func__, \
		code->scopes_cnt);
#else
# define DEBUG_FUNC()
# define DEBUG_SCOPE()
#endif


/* private */
/* types */
typedef enum _CodeTypeFlags
{
	CTF_NULL	= 0x0,
	CTF_SIGNED	= 0x1,
	CTF_UNSIGNED	= 0x2,
	CTF_SHORT	= 0x4,
	CTF_LONG	= 0x8,
	CTF_LONG_LONG	= 0x10
} CodeTypeFlags;

typedef struct _CodeType
{
	int flags;			/* bitmask of flags */
	char * name;
} CodeType;

typedef struct _CodeVariable
{
	CodeType * type;
	char * name;
} CodeVariable;

typedef struct _CodeScope
{
	CodeVariable * variables;
	size_t variables_cnt;
} CodeScope;

typedef struct _CodeIdentifier
{
	CodeContext context;
	CodeStorage storage;
	/* XXX consider copying the original token instead */
	char * name;
} CodeIdentifier;


/* prototypes */
/* context */
static void _code_context_flush(Code * code);
static int _code_context_queue_identifier(Code * code, char const * identifier);

/* target */
static int _code_target_init(Code * code, char const * outfile, int optlevel);
static int _code_target_exit(Code * code);
static int _code_target_token(Code * code, Token * token);
static int _code_target_function_begin(Code * code, char const * name);
static int _code_target_function_call(Code * code, char const * name);
static int _code_target_function_end(Code * code);
static int _code_target_label_set(Code * code, char const * name);


/* protected */
/* types */
struct _Code
{
	/* target */
	Plugin * plugin;
	TargetPlugin * target;
	/* types */
	CodeType * types;
	size_t types_cnt;
	/* scope */
	CodeScope * scopes;
	size_t scopes_cnt;
	/* context */
	CodeContext context;
	CodeStorage storage;
	CodeIdentifier * identifiers;
	size_t identifiers_cnt;
};


/* private */
/* functions */
/* context */
/* code_context_flush */
static void _code_context_flush(Code * code)
{
	size_t i;

	code->context = CODE_CONTEXT_NULL;
	code->storage = CODE_STORAGE_NULL;
	for(i = 0; i < code->identifiers_cnt; i++)
		free(code->identifiers[i].name);
	free(code->identifiers);
	code->identifiers = NULL;
	code->identifiers_cnt = 0;
}


static int _code_context_queue_identifier(Code * code, char const * identifier)
{
	CodeIdentifier * p;

	if((p = realloc(code->identifiers, sizeof(*p) * (code->identifiers_cnt
						+ 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->identifiers = p;
	/* initialize identifier */
	p = &code->identifiers[code->identifiers_cnt];
	p->context = code->context;
	p->storage = code->storage;
	if((p->name = strdup(identifier)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	code->identifiers_cnt++;
	return 0;
}


/* target */
/* code_target_init */
static int _code_target_init(Code * code, char const * outfile, int optlevel)
{
	DEBUG_FUNC();
	if(code->target->init == NULL)
		return 0;
	return code->target->init(outfile, optlevel);
}


/* code_target_exit */
static int _code_target_exit(Code * code)
{
	DEBUG_FUNC();
	if(code->target->exit == NULL)
		return 0;
	return code->target->exit();
}


/* code_target_token */
static int _code_target_token(Code * code, Token * token)
{
	DEBUG_FUNC();
	if(code->target->token == NULL)
		return 0;
	return code->target->token(token);
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
	DEBUG_FUNC();
	if(code->target->function_end == NULL)
		return 0;
	return code->target->function_end();
}


/* code_target_label_set */
static int _code_target_label_set(Code * code, char const * name)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if(code->target->label_set == NULL)
		return 0;
	return code->target->label_set(name);
}


/* public */
/* functions */
/* code_new */
static int _new_target(Code * code, char const * target,
		C99Option const * options, size_t options_cnt);

Code * code_new(C99Prefs const * prefs, char const * outfile)
{
	Code * code;
	C99Prefs const * p = prefs;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, prefs, outfile);
#endif
	if((code = object_new(sizeof(*code))) == NULL)
		return NULL;
	memset(code, 0, sizeof(*code));
	if(_new_target(code, p->target, p->options, p->options_cnt) != 0
			|| _code_target_init(code, outfile, p->optlevel) != 0
			|| code_scope_push(code) != 0)
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
	CodeScope * scope;
	size_t i;

	DEBUG_FUNC();
	if(code->plugin != NULL)
	{
		if(code->target != NULL)
			ret = _code_target_exit(code);
		plugin_delete(code->plugin);
	}
	/* free the context */
	_code_context_flush(code);
	/* free the scopes */
	/* do it ourselves as code_scope_pop() stops at the global */
	for(; code->scopes_cnt > 0; code->scopes_cnt--)
	{
		scope = &code->scopes[code->scopes_cnt - 1];
		for(i = 0; i < scope->variables_cnt; i++)
			free(scope->variables[i].name);
		free(scope->variables);
	}
	free(code->scopes);
	/* free the types */
	for(i = 0; i < code->types_cnt; i++)
		free(code->types[i].name);
	free(code->types);
	object_delete(code);
	return ret;
}


/* useful */
/* parsing */
int code_token(Code * code, Token * token)
{
	return _code_target_token(code, token);
}


/* context */
/* code_context_get */
CodeContext code_context_get(Code * code)
{
	return code->context;
}


/* code_context_set */
int code_context_set(Code * code, CodeContext context)
	/* XXX use assertions? */
{
	int ret = 0;
	size_t i;
	CodeIdentifier * ci;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() 0x%x\n", __func__, context);
#endif
	code->context = context;
	switch(context)
	{
		case CODE_CONTEXT_ASSIGNMENT:
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_DECLARATION:
			/* handle any DECLARATION_OR_FUNCTION identifier */
			for(i = 0; i < code->identifiers_cnt; i++)
			{
				ci = &code->identifiers[i];
				/* XXX ugly hack */
				if(ci->context != CODE_CONTEXT_PARAMETERS_TYPE)
					code->context = context;
				else
					code->context = CODE_CONTEXT_DECLARATION_PARAMETERS;
				code->storage = ci->storage;
				ret |= code_context_set_identifier(code,
						ci->name);
			}
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_DECLARATION_END:
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_FUNCTION:
			/* handle any DECLARATION_OR_FUNCTION identifier */
			/* XXX ugly hack */
			for(i = 0; i < code->identifiers_cnt; i++)
				if(code->identifiers[i].context ==
						CODE_CONTEXT_TYPEDEF_NAME)
					continue;
				else
				{
					ret |= code_context_set_identifier(code,
							code->identifiers[i]
							.name);
					break;
				}
			code->context = CODE_CONTEXT_FUNCTION_PARAMETERS;
			for(i++; i < code->identifiers_cnt; i++)
				ret |= code_context_set_identifier(code,
						code->identifiers[i].name);
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_FUNCTION_END:
			ret |= code_function_end(code);
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_FUNCTION_PARAMETERS:
			/* XXX unchecked but consider it was a function call */
			if(code->identifiers_cnt == 1)
			{
				code->context = CODE_CONTEXT_FUNCTION_CALL;
				ret |= code_context_set_identifier(code,
						code->identifiers[0].name);
			}
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_LABEL:
			if(code->identifiers_cnt == 1)
			{
				code->context = CODE_CONTEXT_LABEL;
				ret |= code_context_set_identifier(code,
						code->identifiers[0].name);
			}
			_code_context_flush(code);
			break;
		case CODE_CONTEXT_STATEMENT:
			_code_context_flush(code);
			break;
		default:
			break;
	}
	return ret;
}


/* code_context_set_class */
int code_context_set_class(Code * code, CodeClass cclass)
{
	/* FIXME implement */
	return 0;
}


/* code_context_set_identifier */
int code_context_set_identifier(Code * code, char const * identifier)
{
#ifdef DEBUG
	char const * str[CODE_CONTEXT_COUNT] =
	{
		"NULL",
		"assignment",
		"declaration",
		"declaration begin",
		"declaration end",
		"declaration or function",
		"declaration parameters",
		"enumeration constant",
		"enumeration name",
		"enumeration value",
		"function",
		"function begin",
		"function call",
		"function end",
		"function parameters",
		"label",
		"parameters",
		"parameters type",
		"primary expr",
		"statement",
		"struct",
		"typedef name",
		"union"
	};

	fprintf(stderr, "DEBUG: %s(\"%s\") %s\n", __func__, identifier,
			str[code->context]);
#endif
	switch(code->context)
	{
		case CODE_CONTEXT_DECLARATION:
			if(code->storage & CODE_STORAGE_TYPEDEF)
				/* FIXME get filename and line */
				return code_type_add(code, identifier);
			return code_variable_add(code, identifier);
		case CODE_CONTEXT_DECLARATION_OR_FUNCTION:
			return _code_context_queue_identifier(code, identifier);
		case CODE_CONTEXT_ENUMERATION_CONSTANT:
			return code_variable_add(code, identifier);
		case CODE_CONTEXT_FUNCTION:
			return code_function_begin(code, identifier);
		case CODE_CONTEXT_FUNCTION_CALL:
			return code_function_call(code, identifier);
		case CODE_CONTEXT_LABEL:
			return code_label_set(code, identifier);
		case CODE_CONTEXT_PARAMETERS:
			return _code_context_queue_identifier(code, identifier);
		case CODE_CONTEXT_PARAMETERS_TYPE:
			return _code_context_queue_identifier(code, identifier);
		case CODE_CONTEXT_PRIMARY_EXPR:
			return _code_context_queue_identifier(code, identifier);
		default:
			break;
	}
	return 0;
}


/* code_context_set_storage */
int code_context_set_storage(Code * code, CodeStorage storage)
{
	struct
	{
		CodeStorage storage;
		char const * name;
	} sn[CODE_STORAGE_COUNT] =
	{
		{ CODE_STORAGE_NULL,	"NULL"		},
		{ CODE_STORAGE_TYPEDEF,	"typedef"	},
		{ CODE_STORAGE_EXTERN,	"extern"	},
		{ CODE_STORAGE_STATIC,	"static"	},
		{ CODE_STORAGE_AUTO,	"auto"		},
		{ CODE_STORAGE_REGISTER,"register"	}
	};
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(0x%x)\n", __func__, storage);
#endif
	if(!(code->storage & storage))
	{
		code->storage |= storage;
		return 0;
	}
	for(i = 0; i < CODE_STORAGE_COUNT; i++)
		if(sn[i].storage == storage)
			return error_set_code(1, "%s\"%s\"", "Duplicate ",
					sn[i].name);
	return error_set_code(1, "%s", "Duplicate storage class specifier");
}


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


/* code_label_set */
int code_label_set(Code * code, char const * name)
{
	return _code_target_label_set(code, name);
}


/* scope */
/* code_scope_push
 * PRE
 * POST	the current scope is increased one level
 * 	>= 0	the current scope value
 * 	else	an error happened */
int code_scope_push(Code * code)
{
	CodeScope * p;

	DEBUG_SCOPE();
	/* resize the scope array */
	if((p = realloc(code->scopes, sizeof(*p) * (code->scopes_cnt + 1)))
			== NULL)
		return -error_set_code(1, "%s", strerror(errno));
	code->scopes = p;
	/* initialize the new scope */
	p = &code->scopes[code->scopes_cnt];
	p->variables = NULL;
	p->variables_cnt = 0;
	return code->scopes_cnt++;
}


/* code_scope_pop
 * PRE
 * POST	the current scope is decreased one level and relevant variables removed
 * 	>= 0	the current scope value
 * 	else	the scope was already global (0) */
int code_scope_pop(Code * code)
{
	CodeScope * p;
	size_t i;

	DEBUG_SCOPE();
	if(code->scopes_cnt <= 1)
		return -error_set_code(1, "%s", "Already in global scope");
	/* free all variables in the current scope */
	p = &code->scopes[code->scopes_cnt - 1];
	for(i = 0; i < p->variables_cnt; i++)
		free(p->variables[i].name);
	free(p->variables);
	/* resize the scope array */
	if((p = realloc(code->scopes, sizeof(*p) * (code->scopes_cnt - 1)))
			!= NULL) /* ignoring the error is fine */
		code->scopes = p;
	return --(code->scopes_cnt);
}


/* types */
/* code_type_add */
int code_type_add(Code * code, char const * name)
	/* FIXME consider using a Token instead of the name */
{
	size_t i;
	CodeType * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if(name == NULL || name[0] == '\0')
		return -error_set_code(1, "%s", "Invalid name for a type");
	for(i = 0; i < code->types_cnt; i++)
	{
		p = &code->types[i];
		if(strcmp(p->name, name) != 0)
			continue;
		return -error_set_code(1, "%s%s%s:%u", name,
				" is already defined");
	}
	if((p = realloc(code->types, sizeof(*p) * (i + 1))) == NULL)
		return -error_set_code(1, "%s", strerror(errno));
	code->types = p;
	code->types[i].name = strdup(name);
	if(code->types[i].name == NULL)
	{
		free(code->types[i].name);
		return -error_set_code(1, "%s", strerror(errno));
	}
	return code->types_cnt++;
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
	CodeScope * scope;
	CodeVariable * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	/* check if the name is valid */
	if(name == NULL || name[0] == '\0')
		return error_set_code(1, "%s", "Invalid name for a variable");
	/* resize the current scope */
	scope = &code->scopes[code->scopes_cnt - 1];
	if((p = realloc(scope->variables, sizeof(*p)
					* (scope->variables_cnt + 1))) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	scope->variables = p;
	/* assign the variable */
	p = &scope->variables[scope->variables_cnt];
	p->name = strdup(name);
	p->type = NULL; /* FIXME implement */
	if(p->name == NULL)
		return error_set_code(1, "%s", strerror(errno));
	scope->variables_cnt++;
	return 0;
}
