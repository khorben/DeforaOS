/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
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



#include <stddef.h>
#include "As/arch.h"


/* Dalvik */
/* private */
/* variables */
enum
{
#include "common.reg"
};

static ArchRegister _dalvik_regs[] =
{
	{ NULL,		0, 0 }
};

static ArchInstruction _dalvik_set[] =
{
	{ "nop",	0x0000,	AO_NONE,	2, 0, 0, 0 },
	{ "ret",	0x0f00,	AO_NONE,	2, 0, 0, 0 },
#include "common.ins"
#include "null.ins"
};


/* public */
/* variables */
ArchPlugin arch_plugin =
{
	"dalvik",
	_dalvik_regs,
	_dalvik_set
};
