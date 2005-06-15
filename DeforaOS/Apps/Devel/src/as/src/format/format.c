/* format/format.c */



#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "../as.h"
#include "format.h"


/* Format */
Format * format_new(char * format)
{
	Format * f;
	void * handle;
	Format * plugin;

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
	if((f = malloc(sizeof(Format))) == NULL)
	{
		as_error("malloc", 0);
		as_plugin_delete(handle);
		return NULL;
	}
	f->format_init = plugin->format_init;
	f->format_exit = plugin->format_exit;
	f->plugin = handle;
	return f;
}


/* format_delete */
void format_delete(Format * format)
{
	as_plugin_delete(format->plugin);
	free(format);
}


/* useful */
/* format_init */
int format_init(Format * format, FILE * fp)
{
	return format->format_init(fp);
}
