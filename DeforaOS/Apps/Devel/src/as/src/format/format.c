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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "as.h"
#include "format.h"
#include "../../config.h"


/* Format */
/* private */
/* types */
struct _Format
{
	char * arch;
	FormatPlugin * plugin;
	Plugin * handle;
};


/* public */
/* functions */
/* format_new */
Format * format_new(char const * format, char const * arch,
		char const * filename)
{
	Format * f;
	Plugin * handle;
	FormatPlugin * plugin;

	if(format == NULL)
		format = "elf";
	if((handle = plugin_new(LIBDIR, PACKAGE, "format", format)) == NULL)
		return NULL;
	if((plugin = plugin_lookup(handle, "format_plugin")) == NULL)
	{
		plugin_delete(handle);
		return NULL;
	}
	if((f = malloc(sizeof(*f))) == NULL || (f->arch = strdup(arch)) == NULL)
	{
		if(f != NULL)
			free(f);
		error_set_code(1, "%s", strerror(errno));
		plugin_delete(handle);
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
	plugin_delete(format->handle);
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
