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
