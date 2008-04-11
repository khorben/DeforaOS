/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
/* libSystem is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * libSystem is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with libSystem; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "System.h"


/* Plugin */
/* private */
/* constants */
#define PLUGIN_EXTENSION ".so"


/* public */
/* functions */
/* plugin_new */
Plugin * plugin_new(char const * libdir, char const * package,
		char const * type, char const * name)
{
	Plugin * plugin;
	size_t len;
	char * filename;

	len = strlen(libdir) + 1 + strlen(package) + 1 + strlen(type) + 1
		+ strlen(name) + strlen(PLUGIN_EXTENSION) + 1;
	if((filename = malloc(len)) == NULL)
	{
		error_set_code(1, "%s", strerror(errno));
		return NULL;
	}
	snprintf(filename, len, "%s/%s/%s/%s%s", libdir, package, type, name,
			PLUGIN_EXTENSION);
	if((plugin = dlopen(filename, RTLD_LAZY)) == NULL)
		error_set_code(1, "%s: %s", filename, dlerror());
	free(filename);
	return plugin;
}


/* plugin_delete */
void plugin_delete(Plugin * plugin)
{
	dlclose(plugin);
}
