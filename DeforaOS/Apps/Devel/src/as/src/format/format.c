/* format/format.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "../as.h"
#include "format.h"


/* Format */
Format * format_new(char * format, char * arch)
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
	if((f = malloc(sizeof(Format))) == NULL || (f->arch = strdup(arch))
			== NULL)
	{
		if(f != NULL)
			free(f);
		as_error("malloc", 0);
		as_plugin_delete(handle);
		return NULL;
	}
	f->plugin = plugin;
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
int format_section(Format * format, FILE * fp, char * section)
{
	if(format->plugin->section == NULL)
		return 0;
	return format->plugin->section(fp, section);
}
