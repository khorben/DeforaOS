/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libSystem */
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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef __WIN32__
# include <windows.h>
#else
# include <dlfcn.h>
#endif
#include "System.h"


/* Plugin */
/* private */
/* constants */
#ifdef __WIN32__
# define PLUGIN_EXTENSION	".dll"
#else
# define PLUGIN_EXTENSION	".so"
#endif


/* prototypes */
static Plugin * _plugin_open(char const * filename);


/* functions */
static Plugin * _plugin_open(char const * filename)
{
#ifdef __WIN32__
	if(filename == NULL)
		return GetModuleHandle(NULL);
	return LoadLibraryEx(filename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
	Plugin * plugin;

	if((plugin = dlopen(filename, RTLD_LAZY)) == NULL)
		error_set_code(1, "%s", dlerror());
	return plugin;
#endif
}


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
	plugin = _plugin_open(filename);
	free(filename);
	return plugin;
}


/* plugin_new_self */
Plugin * plugin_new_self(void)
{
	return _plugin_open(NULL);
}


/* plugin_delete */
void plugin_delete(Plugin * plugin)
{
#ifdef __WIN32__
	FreeLibrary(plugin);
#else
	dlclose(plugin);
#endif
}


/* useful */
/* plugin_lookup */
void * plugin_lookup(Plugin * plugin, char const * symbol)
{
#ifdef __WIN32__
	return GetProcAddress(plugin, symbol);
#else
	void * ret;

	if((ret = dlsym(plugin, symbol)) == NULL)
		error_set_code(1, "%s", dlerror());
	return ret;
#endif
}
