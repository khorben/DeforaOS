/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "as.h"
#include "../config.h"


/* as */
/* private */
/* types */
struct _As
{
	char const * arch;
	char const * format;
	Code * code;
};

typedef struct _AsPluginDescription
{
	char const * name;
	char const * description;
} AsPluginDescription;


/* constants */
#define ASPT_LAST	ASPT_FORMAT
#define ASPT_COUNT	(ASPT_LAST + 1)


/* variables */
static const AsPluginDescription _as_plugin_description[ASPT_COUNT] =
{
	{ "arch",	"architecture"	},
	{ "format",	"file format"	}
};


/* prototypes */
static char const * _as_guess_arch(void);
static char const * _as_guess_format(void);


/* functions */
/* as_guess_arch */
static char const * _as_guess_arch(void)
{
	static struct utsname uts;
	static int cached = 0;

	if(cached == 0)
	{
		if(uname(&uts) != 0)
		{
			error_set_code(1, "%s", strerror(errno));
			return NULL;
		}
		cached = 1;
	}
	return uts.machine;
}


/* as_guess_format */
static char const * _as_guess_format(void)
{
	/* XXX may be dependent from the architecture used */
	return "elf";
}


/* public */
/* functions */
/* as_new */
As * as_new(char const * arch, char const * format)
{
	As * as;

	if((as = object_new(sizeof(*as))) == NULL)
		return NULL;
	if((as->arch = arch) == NULL)
		as->arch = _as_guess_arch();
	if((as->format = format) == NULL)
		as->format = _as_guess_format();
	as->code = NULL;
	if(as->arch == NULL || as->format == NULL)
	{
		as_delete(as);
		return NULL;
	}
	return as;
}


/* as_delete */
void as_delete(As * as)
{
	as_close(as);
	object_delete(as);
}


/* accessors */
/* as_get_arch */
char const * as_get_arch(As * as)
{
	return as->arch;
}


/* as_get_format */
char const * as_get_format(As * as)
{
	return as->format;
}


/* useful */
/* as_parse */
int as_parse(As * as, char const * infile, char const * outfile)
{
	int ret;

	if(as_open(as, outfile) != 0)
		return 1;
	ret = parser(as->code, infile);
	if(ret != 0 && unlink(outfile) != 0)
		ret |= error_set_code(3, "%s: %s", outfile, strerror(errno));
	ret |= as_close(as);
	return ret;
}


/* as_open */
int as_open(As * as, char const * outfile)
{
	if(as_close(as) != 0)
		return 1;
	if((as->code = code_new(as->arch, as->format, outfile)) == NULL)
		return 1;
	return 0;
}


/* as_close */
int as_close(As * as)
{
	int ret = 0;

	if(as->code != NULL)
	{
		ret = code_delete(as->code);
		as->code = NULL;
	}
	return ret;
}


/* as_section */
int as_section(As * as, char const * name)
{
	return code_section(as->code, name);
}


/* as_function */
int as_function(As * as, char const * name)
{
	return code_function(as->code, name);
}


/* as_instruction */
int as_instruction(As * as, char const * name, unsigned int operands_cnt, ...)
{
	int ret;
	va_list ap;
	AsOperand ** ao = NULL;
	size_t i;

	if(operands_cnt != 0)
	{
		if((ao = malloc(sizeof(*ao) * operands_cnt)) == NULL)
			return error_set_code(1, "%s", strerror(errno));
		va_start(ap, operands_cnt);
		for(i = 0; i < operands_cnt; i++)
			ao[i] = va_arg(ap, AsOperand *);
		va_end(ap);
	}
	ret = code_instruction(as->code, name, ao, operands_cnt);
	free(ao);
	return ret;
}


/* as_plugin_list */
int as_plugin_list(AsPluginType type)
{
	AsPluginDescription const * aspd;
	char * path;
	DIR * dir;
	struct dirent * de;
	size_t len;

	aspd = &_as_plugin_description[type];
	fprintf(stderr, "%s%s%s", "Available ", aspd->description,
			" plug-ins:");
	len = strlen(LIBDIR) + 1 + strlen(PACKAGE) + 1 + strlen(aspd->name) + 1;
	if((path = malloc(len)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		fputc('\n', stderr);
		return 1;
	}
	snprintf(path, len, "%s/%s/%s", LIBDIR, PACKAGE, aspd->name);
	if((dir = opendir(path)) == NULL)
	{
		error_set_code(1, "%s: %s", path, strerror(errno));
		fputc('\n', stderr);
		free(path);
		return 1;
	}
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name)) < 4)
			continue;
		if(strcmp(".so", &de->d_name[len - 3]) != 0)
			continue;
		de->d_name[len - 3] = '\0';
		fprintf(stderr, " %s", de->d_name);
	}
	free(path);
	closedir(dir);
	fputc('\n', stderr);
	return 0;
}
