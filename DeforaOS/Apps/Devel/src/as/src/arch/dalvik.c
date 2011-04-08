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
/* types */
#define REG(name, size, id) AO_ ## name = ((id << 2) | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__),
enum
{
#include "common.reg"
#include "dalvik.reg"
};


/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
static ArchRegister _dalvik_regs[] =
{
#include "dalvik.reg"
	{ NULL,		0, 0 }
};

static ArchInstruction _dalvik_set[] =
{
	{ "mov",	0x0100,	AO_v0|AO_v0_,	2, 0, 0, 0 },
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
