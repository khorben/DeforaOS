/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
#include "arch.h"


/* types */
#define REG(name, size, id) AO_ ## name = ((id << 2) | AO_REG), \
				AO_ ## name ## _ = ((id << 10) | AO_REG_), \
				AO_ ## name ## __ = ((id << 18) | AO_REG__), \
				AO_D_ ## name = ((id << 2) | AO_DREG), \
				AO_D_ ## name ## _ = ((id << 10) | AO_DREG_), \
				AO_D_ ## name ## __ = ((id << 18) | AO_DREG__),
enum
{
#include "common.reg"
#include "80686.reg"
};


/* variables */
#undef REG
#define REG(name, size, id) { "" # name, size, id },
static ArchRegister _i686_regs[] =
{
#include "80686.reg"
	{ NULL,		0, 0 }
};

static ArchInstruction _i686_set[] =
{
#include "80686.ins"
#include "common.ins"
#include "null.ins"
};

ArchPlugin arch_plugin =
{
	_i686_regs,
	_i686_set
};
