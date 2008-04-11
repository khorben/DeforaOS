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



#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "parser.h"
#include "as.h"
#include "../config.h"

#ifndef PACKAGE
# define PACKAGE "as"
#endif

#ifndef PREFIX
# define PREFIX		"."
#endif

#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib/" PACKAGE
#endif


/* as */
static char * _as_guess(void);
static int as(char const * arch, char const * format,
		char * infile, char * outfile)
{
	FILE * infp;
	Code * code;
	int ret;

	if(arch == NULL && (arch = _as_guess()) == NULL)
		return 2;
	if((infp = fopen(infile, "r")) == NULL)
		return as_error(infile, 2);
	if((code = code_new(arch, format, outfile)) == NULL)
		ret = 2;
	else
	{
		ret = parser(code, infile, infp);
		code_delete(code, ret);
	}
	fclose(infp);
	return ret;
}

static char * _as_guess(void)
{
	static struct utsname uts;

	if(uname(&uts) != 0)
	{
		as_error("architecture guess", 0);
		return NULL;
	}
	return uts.machine;
}


/* useful */
/* as_error */
int as_error(char const * msg, int ret)
{
	fputs("as: ", stderr);
	perror(msg);
	return ret;
}


/* plugins helpers */
void * as_plugin_new(char const * type, char const * name,
		char const * description)
{
	char * filename;
	void * handle;

	if((filename = malloc(strlen(LIBDIR) + 1 + strlen(type) + 1
					+ strlen(name) + strlen(".so") + 1))
				== NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	sprintf(filename, "%s/%s/%s%s", LIBDIR, type, name, ".so");
	if((handle = dlopen(filename, RTLD_LAZY)) == NULL)
		fprintf(stderr, "%s%s%s%s%s", "as: ", name, ": No such ",
				description, " plug-in\n");
	free(filename);
	return handle;
}


/* as_plugin_delete */
void as_plugin_delete(void * handle)
{
	dlclose(handle);
}


/* as_plugin_list */
void as_plugin_list(char const * type, char const * description)
{
	char * path;
	DIR * dir;
	struct dirent * de;
	unsigned int len;

	fprintf(stderr, "%s%s%s", "Available ", description, " plug-ins:");
	if((path = malloc(strlen(LIBDIR) + 1 + strlen(type) + 1)) == NULL)
	{
		fputc('\n', stderr);
		as_error("malloc", 0);
		return;
	}
	sprintf(path, "%s/%s", LIBDIR, type);
	if((dir = opendir(path)) == NULL)
	{
		fputc('\n', stderr);
		as_error(path, 0);
		return;
	}
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name)) < 4)
			continue;
		if(strcmp(".so", &de->d_name[len-3]) != 0)
			continue;
		de->d_name[len-3] = '\0';
		fprintf(stderr, " %s", de->d_name);
	}
	free(path);
	closedir(dir);
	fputc('\n', stderr);
}


/* usage */
static unsigned int _usage(void)
{
	char * arch = _as_guess();

	fprintf(stderr, "%s%s%s",
"Usage: as [-a arch][-f format][-o file] file\n"
"       as -l\n"
"  -a	target architecture (default: ", arch != NULL ? arch : "guessed", ")\n"
"  -f	target file format (default: elf)\n"
"  -o	filename to use for output (default: \"" AS_FILENAME_DEFAULT "\")\n"
"  -l	list available architectures and formats\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char * outfile = AS_FILENAME_DEFAULT;
	char * arch = NULL;
	char * format = NULL;

	while((o = getopt(argc, argv, "a:f:o:l")) != -1)
	{
		switch(o)
		{
			case 'a':
				arch = optarg;
				break;
			case 'f':
				format = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'l':
				as_plugin_list("arch", "architecture");
				as_plugin_list("format", "file format");
				return 0;
			default:
				return _usage();
		}
	}
	if(argc - optind != 1)
		return _usage();
	return as(arch, format, argv[optind], outfile) ? 2 : 0;
}
