/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <Devel/As.h>
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

typedef struct _AsArch
{
	char const * name;
	int (*function_begin)(char const * name);
	int (*function_call)(char const * name);
	int (*function_end)(void);
} AsArch;


/* variables */
static As * _as_as;
static int _as_optlevel;

static C99Option _as_options[ASO_COUNT + 1] =
{
	{ "arch",	NULL	},
	{ "format",	NULL	},
	{ NULL,		NULL	}
};

/* platforms */
/* amd64 */
static int _as_arch_amd64_function_begin(char const * name);
static int _as_arch_amd64_function_call(char const * name);
static int _as_arch_amd64_function_end(void);
static AsArch _as_arch_amd64 =
{
	"amd64",
	_as_arch_amd64_function_begin,
	_as_arch_amd64_function_call,
	_as_arch_amd64_function_end
};

static AsArch * _as_arch[] =
{
	&_as_arch_amd64
};


/* protected */
/* prototypes */
static int _as_init(char const * outfile, int optlevel);
static int _as_exit(void);
static int _as_section(char const * name);


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
	NULL,
	NULL,
	NULL,
	NULL				/* FIXME implement label_set */
};


/* protected */
/* functions */
/* as_init */
static int _init_arch(char const * arch);
static int _init_defines(char const * format);

static int _as_init(char const * outfile, int optlevel)
{
	char const * arch = _as_options[ASO_ARCH].value;
	char const * format = _as_options[ASO_FORMAT].value;

	_as_optlevel = optlevel;
	if((_as_as = as_new(arch, format)) == NULL)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s: architecture \"%s\", format \"%s\"\n",
			PACKAGE, as_get_arch(_as_as), as_get_format(_as_as));
#endif
	if(_init_arch(as_get_arch(_as_as)) != 0
			|| _init_defines(as_get_format(_as_as)) != 0
			|| as_open(_as_as, outfile) != 0
			|| _as_section(".text") != 0)
	{
		as_delete(_as_as);
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => 0\n", __func__);
#endif
	return 0;
}

static int _init_arch(char const * arch)
{
	AsArch * aarch = NULL;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, arch);
#endif
	for(i = 0; i < sizeof(_as_arch) / sizeof(*_as_arch); i++)
		if(strcmp(_as_arch[i]->name, arch) == 0)
		{
			aarch = _as_arch[i];
			break;
		}
	if(aarch == NULL)
		return -error_set_code(1, "%s%s", "Unsupported architecture ",
				arch);
	target_plugin.function_begin = aarch->function_begin;
	target_plugin.function_call = aarch->function_call;
	target_plugin.function_end = aarch->function_end;
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


/* platforms */
/* amd64 */
static int _as_arch_amd64_function_begin(char const * name)
{
	/* FIXME give real arguments */
	AsOperand ao[2] =
	{
		{ AOT_IMMEDIATE, 0, NULL },
		{ AOT_IMMEDIATE, 0, NULL }
	};
	unsigned long a0 = 0;
	unsigned long a1 = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ao[0].value = &a0;
	ao[1].value = &a1;
	return as_instruction(_as_as, "enter", 2, &ao[0], &ao[1]);
}


static int _as_arch_amd64_function_call(char const * name)
{
	/* FIXME give a real argument */
	AsOperand ao = { AOT_IMMEDIATE, 0, NULL };
	unsigned long a0 = 0;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	ao.value = &a0;
	return as_instruction(_as_as, "call", 1, &ao);
}


static int _as_arch_amd64_function_end(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return as_instruction(_as_as, "leave", 0);
}
