/* arch/arch.c */



#include <stdlib.h>
#include <dlfcn.h>
#include "../as.h"
#include "arch.h"


/* Arch */
Arch * arch_new(char * arch)
{
	Arch * a;
	void * handle;
	Arch * plugin;

	/* FIXME if(arch == NULL) then guess... */
	if(arch == NULL)
		arch = "x86";
	if((handle = as_plugin_new("arch", arch)) == NULL)
		return NULL;
	if((plugin = dlsym(handle, "arch_plugin")) == NULL)
	{
		as_error("dlsym", 0); /* FIXME not very explicit */
		return NULL;
	}
	if((a = malloc(sizeof(Arch))) == NULL)
	{
		as_error("malloc", 0);
		dlclose(handle);
		return NULL;
	}
	a->instructions = plugin->instructions;
	a->registers = plugin->registers;
	a->plugin = handle;
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	as_plugin_delete(arch->plugin);
	free(arch);
}


/* ArchOperands */
int archoperands_count(ArchOperands op)
{
	switch(op)
	{
		case AO_NONE:
			return 0;
		case AO_IMM:
		case AO_REG:
			return 1;
		case AO_IMM_IMM:
		case AO_IMM_REG:
		case AO_REG_IMM:
		case AO_REG_REG:
			return 2;
	}
	return -1;
}
