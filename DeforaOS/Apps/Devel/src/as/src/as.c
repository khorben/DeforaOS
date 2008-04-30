/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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
#include <dirent.h>
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
	object_delete(as);
}


/* useful */
/* as_do */
int as_do(As * as, char const * infile, char const * outfile)
{
	FILE * infp;
	Code * code;
	int ret;

	if((infp = fopen(infile, "r")) == NULL)
		return error_set_code(1, "%s: %s", infile, strerror(errno));
	if((code = code_new(as->arch, as->format, outfile)) == NULL)
		ret = 1;
	else
	{
		ret = parser(code, infile, infp);
		code_delete(code, ret);
	}
	fclose(infp);
	return ret;
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
		if(strcmp(".so", &de->d_name[len-3]) != 0)
			continue;
		de->d_name[len - 3] = '\0';
		fprintf(stderr, " %s", de->d_name);
	}
	free(path);
	closedir(dir);
	fputc('\n', stderr);
	return 0;
}
