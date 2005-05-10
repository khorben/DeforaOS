/* arch/arch.c */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "../as.h"
#include "arch.h"


/* Arch */
Arch * arch_new(char * arch)
{
	Arch * a;
	char * filename;
	void * handle;
	Arch * plugin;

	/* FIXME if(arch == NULL) then guess... */
	arch = "x86";
#ifndef PREFIX
# define PREFIX "."
#endif
	if((filename = malloc(strlen(PREFIX "/arch/") + strlen(arch)
					+ strlen(".so") + 1)) == NULL)
	{
		as_error("malloc", 0);
		return NULL;
	}
	sprintf(filename, "%s%s%s", PREFIX "/arch/", arch, ".so");
	if((handle = dlopen(filename, RTLD_NOW)) == NULL
			|| (plugin = dlsym(handle, "arch_plugin")) == NULL)
	{
		as_error(filename, 0);
		free(filename);
		return NULL;
	}
	if((a = malloc(sizeof(Arch))) == NULL)
	{
		as_error("malloc", 0);
		free(filename);
		dlclose(handle);
		return NULL;
	}
	a->instructions = plugin->instructions;
	a->registers = plugin->registers;
	a->plugin = handle;
	free(filename);
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	dlclose(arch->plugin);
	free(arch);
}
