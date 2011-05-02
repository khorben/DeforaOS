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



#include <Devel/Asm.h>
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
typedef enum _AsmOption
{
	ASO_ARCH	= 0,
	ASO_FORMAT
} AsmOption;
#define ASO_LAST	ASO_FORMAT
#define ASO_COUNT	(ASO_LAST + 1)

typedef struct _AsmArch
{
	char const * name;
	int (*function_begin)(char const * name);
	int (*function_call)(char const * name);
	int (*function_end)(void);
} AsmArch;


/* variables */
static Asm * _asm_as;
static int _asm_optlevel;

static C99Option _asm_options[ASO_COUNT + 1] =
{
	{ "arch",	NULL	},
	{ "format",	NULL	},
	{ NULL,		NULL	}
};

/* platforms */
/* amd64 */
static int _asm_arch_amd64_function_begin(char const * name);
static int _asm_arch_amd64_function_call(char const * name);
static int _asm_arch_amd64_function_end(void);
static AsmArch _asm_arch_amd64 =
{
	"amd64",
	_asm_arch_amd64_function_begin,
	_asm_arch_amd64_function_call,
	_asm_arch_amd64_function_end
};

static AsmArch * _asm_arch[] =
{
	&_asm_arch_amd64
};


/* protected */
/* prototypes */
static int _asm_init(char const * outfile, int optlevel);
static int _asm_exit(void);
static int _asm_section(char const * name);


/* public */
/* variables */
TargetPlugin target_plugin =
{
	NULL,
	_asm_options,
	_asm_init,
	_asm_exit,
	NULL,
	_asm_section,
	NULL,
	NULL,
	NULL,
	NULL				/* FIXME implement label_set */
};


/* protected */
/* functions */
/* asm_init */
static int _init_arch(char const * arch);
static int _init_defines(char const * format);

static int _asm_init(char const * outfile, int optlevel)
{
	char const * arch = _asm_options[ASO_ARCH].value;
	char const * format = _asm_options[ASO_FORMAT].value;

	_asm_optlevel = optlevel;
	if((_asm_as = asm_new(arch, format)) == NULL)
		return 1;
	if(arch == NULL)
		asm_guess_arch(_asm_as);
	if(format == NULL)
		asm_guess_format(_asm_as);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s: architecture \"%s\", format \"%s\"\n",
			PACKAGE, asm_get_arch(_asm_as), asm_get_format(_asm_as));
#endif
	if(_init_arch(asm_get_arch(_asm_as)) != 0
			|| _init_defines(asm_get_format(_asm_as)) != 0
			|| asm_open_assemble(_asm_as, outfile) != 0
			|| _asm_section(".text") != 0)
	{
		asm_delete(_asm_as);
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() => 0\n", __func__);
#endif
	return 0;
}

static int _init_arch(char const * arch)
{
	AsmArch * aarch = NULL;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, arch);
#endif
	for(i = 0; i < sizeof(_asm_arch) / sizeof(*_asm_arch); i++)
		if(strcmp(_asm_arch[i]->name, arch) == 0)
		{
			aarch = _asm_arch[i];
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


/* asm_exit */
static int _asm_exit(void)
{
	int ret;

	ret = asm_close(_asm_as);
	asm_delete(_asm_as);
	return ret;
}


/* asm_section */
static int _asm_section(char const * name)
{
	return asm_set_section(_asm_as, name, -1, -1);
}


/* platforms */
/* amd64 */
static int _asm_arch_amd64_function_begin(char const * name)
{
	ArchOperand arg1;
	ArchOperand arg2;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(asm_set_function(_asm_as, name, -1, -1) != 0)
		return -1;
	/* FIXME give real arguments */
	memset(&arg1, 0, sizeof(arg1));
	arg1.definition = AO_IMMEDIATE(0, 0, 32);
	arg1.value.immediate.value = 0;
	memset(&arg2, 0, sizeof(arg2));
	arg2.definition = AO_IMMEDIATE(0, 0, 32);
	arg2.value.immediate.value = 0;
	return asm_instruction(_asm_as, "enter", 2, &arg1, &arg2);
}


static int _asm_arch_amd64_function_call(char const * name)
{
	ArchOperand arg;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME give a real argument */
	memset(&arg, 0, sizeof(arg));
	arg.definition = AO_IMMEDIATE(0, 0, 32);
	arg.value.immediate.value = 0;
	return asm_instruction(_asm_as, "call", 1, &arg);
}


static int _asm_arch_amd64_function_end(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return asm_instruction(_asm_as, "leave", 0);
}
