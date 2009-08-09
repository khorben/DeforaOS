/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#include <as.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "c99/target.h"
#ifdef DEBUG
# include "../../config.h"
#endif


/* as */
/* private */
/* types */
typedef enum _AsOption
{
	ASO_ARCH	= 0,
	ASO_FORMAT
} AsOption;
#define ASO_LAST	ASO_FORMAT
#define ASO_COUNT	(ASO_LAST + 1)


/* variables */
static As * _as_as;
static int _as_optlevel;

static C99Option _as_options[ASO_COUNT + 1] =
{
	{ "arch",	NULL	},
	{ "format",	NULL	},
	{ NULL,		NULL	}
};


/* protected */
/* prototypes */
static int _as_init(char const * outfile, int optlevel);
static int _as_exit(void);
static int _as_section(char const * name);
static int _as_function_begin(char const * name);
static int _as_function_call(char const * name);
static int _as_function_end(void);


/* public */
/* variables */
TargetPlugin target_plugin =
{
	NULL,
	_as_options,
	_as_init,
	_as_exit,
	NULL,
	_as_section,
	_as_function_begin,
	_as_function_call,
	_as_function_end,
	NULL				/* FIXME implement label_set */
};


/* protected */
/* functions */
/* as_init */
static int _init_defines(char const * format);

static int _as_init(char const * outfile, int optlevel)
{
	char const * arch = _as_options[ASO_ARCH].value;
	char const * format = _as_options[ASO_FORMAT].value;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	_as_optlevel = optlevel;
	/* FIXME verify if we know how to handle to architecture */
	if((_as_as = as_new(arch, format)) == NULL)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s: architecture \"%s\", format \"%s\"\n",
			PACKAGE, as_get_arch(_as_as), as_get_format(_as_as));
#endif
	if(_init_defines(as_get_format(_as_as)) != 0
			|| as_open(_as_as, outfile) != 0
			|| _as_section(".text") != 0)
	{
		as_delete(_as_as);
		return 1;
	}
	return 0;
}

static int _init_defines(char const * format)
{
	C99 * c99;
	size_t len;
	char * p;
	size_t i;
	int c;

	c99 = target_plugin.helper->c99;
	len = strlen(format) + 5;
	if((p = malloc(len)) == NULL)
		return 1;
	snprintf(p, len, "%s%s%s", "__", format, "__");
	for(i = 2; i < len - 2; i++)
	{
		c = p[i];
		p[i] = toupper(c);
	}
	target_plugin.helper->define_add(c99, p, NULL);
	free(p);
	return 0;
}


/* as_exit */
static int _as_exit(void)
{
	int ret;

	ret = as_close(_as_as);
	as_delete(_as_as);
	return ret;
}


/* as_section */
static int _as_section(char const * name)
{
	return as_section(_as_as, name);
}


/* as_function */
static int _as_function_begin(char const * name)
{
	return as_function(_as_as, name);
}


/* as_function_call */
static int _as_function_call(char const * name)
{
	/* FIXME implement */
	return 0;
}


/* as_function_end */
static int _as_function_end(void)
{
	/* FIXME implement */
	return 0;
}
