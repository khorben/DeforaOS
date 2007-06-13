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



#include <stddef.h>
#include "arch.h"

#define I386_REAL


/* types */
#define REG(name, size, id) AO_ ## name = ((id << 2) | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__),
enum {
#include "common.reg"
#include "80386.reg"
};

/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
ArchRegister arch_i386_regs[] =
{
#include "80386.reg"
	{ NULL,		0, 0 }
};

ArchInstruction arch_i386_set[] =
{
#include "80386.ins"
	{ NULL,		0x0000, AO_NONE, 0, 0, 0 }
};

ArchPlugin arch_plugin =
{
	arch_i386_regs,
	arch_i386_set
};
