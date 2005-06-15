/* arch/arch.c */



#include <sys/utsname.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "../as.h"
#include "arch.h"


/* Arch */
static char * _new_guess(void);
Arch * arch_new(char * arch)
{
	Arch * a;
	void * handle;
	Arch * plugin;

	if(arch == NULL && (arch = _new_guess()) == NULL)
		return NULL;
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

static char * _new_guess(void)
{
	static struct utsname uts;

	if(uname(&uts) != 0)
	{
		as_error("architecture guess", 0);
		return NULL;
	}
	return uts.machine;
}


/* arch_delete */
void arch_delete(Arch * arch)
{
	as_plugin_delete(arch->plugin);
	free(arch);
}


/* ArchOperands */
int archoperands_count(ArchOperands operands)
{
	switch(operands)
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
