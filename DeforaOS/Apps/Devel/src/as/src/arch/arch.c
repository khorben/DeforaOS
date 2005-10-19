/* arch/arch.c */



#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>
#include "../as.h"
#include "arch.h"


/* Arch */
static char * _new_guess(void);
Arch * arch_new(char * arch)
{
	Arch * a;
	void * handle;
	Arch * plugin;

	if((handle = as_plugin_new("arch", arch, "architecture")) == NULL)
		return NULL;
	if((plugin = dlsym(handle, "arch_plugin")) == NULL)
	{
		/* FIXME factorize dlsym() operation */
		fprintf(stderr, "%s%s%s", "as: ", arch,
				": Invalid architecture plug-in\n");
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
int archoperands_count(int operands)
{
	if(operands & _AO_OP__)
		return 3;
	if(operands & _AO_OP_)
		return 2;
	if(operands & _AO_OP)
		return 1;
	assert(operands == 0);
	return 0;
}
