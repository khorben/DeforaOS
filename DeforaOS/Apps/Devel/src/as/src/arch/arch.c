/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>
#include "../as.h"
#include "arch.h"


/* Arch */
Arch * arch_new(char const * arch)
{
	Arch * a;
	void * handle;
	Arch * plugin;

	if((handle = as_plugin_new("arch", arch, "architecture")) == NULL)
		return NULL;
	if((plugin = dlsym(handle, "arch_plugin")) == NULL)
	{
		/* FIXME factorize dlsym() operation */
		dlclose(handle);
		fprintf(stderr, "%s%s%s", "as: ", arch,
				": Invalid architecture plug-in\n");
		return NULL;
	}
	/* if((handle = as_plugin_new("arch", arch, "architecture", &plugin)) == NULL)
	 * 	... */
	if((a = malloc(sizeof(Arch))) == NULL)
	{
		as_error("malloc", 0);
		dlclose(handle);
		return NULL;
	}
	a->instructions = plugin->instructions;
	a->registers = plugin->registers;
	a->handle = handle;
	return a;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	as_plugin_delete(arch->handle);
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
