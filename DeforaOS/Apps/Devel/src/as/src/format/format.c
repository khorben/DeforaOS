/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "../as.h"
#include "format.h"


/* Format */
Format * format_new(char const * format, char const * arch,
		char const * filename)
{
	Format * f;
	void * handle;
	FormatPlugin * plugin;

	if(format == NULL)
		format = "elf";
	if((handle = as_plugin_new("format", format, "output")) == NULL)
		return NULL;
	if((plugin = dlsym(handle, "format_plugin")) == NULL)
	{
		/* FIXME factorize dlsym() operation */
		fprintf(stderr, "%s%s%s", "as: ", format,
				": Invalid format plug-in\n");
		return NULL;
	}
	if((f = malloc(sizeof(*f))) == NULL || (f->arch = strdup(arch)) == NULL)
	{
		if(f != NULL)
			free(f);
		as_error("malloc", 0);
		as_plugin_delete(handle);
		return NULL;
	}
	f->plugin = plugin;
	plugin->filename = filename;
	f->handle = handle;
	return f;
}


/* format_delete */
int format_delete(Format * format, FILE * fp)
{
	int ret = 0;

	if(format->plugin->exit != NULL)
		ret = format->plugin->exit(fp);
	as_plugin_delete(format->handle);
	free(format->arch);
	free(format);
	return ret;
}


/* useful */
/* format_init */
int format_init(Format * format, FILE * fp)
{
	if(format->plugin->init == NULL)
		return 0;
	return format->plugin->init(fp, format->arch);
}


/* format_section */
int format_section(Format * format, FILE * fp, char const * section)
{
	if(format->plugin->section == NULL)
		return 0;
	return format->plugin->section(fp, section);
}
