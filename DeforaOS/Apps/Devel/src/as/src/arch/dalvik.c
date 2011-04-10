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
	{ "add-int",	0x90,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "add-long",	0x9b,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget",	0x44,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-boolean",0x47,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-byte",	0x48,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-char",	0x49,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-object",0x46,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-short",	0x4a,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-wide",	0x45,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "and-int",	0x95,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput",	0x4b,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-boolean",0x4e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-byte",	0x4f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-char",	0x50,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-object",0x4d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-short",	0x51,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-wide",	0x4c,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "cmp-long",	0x31,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 0 },
	{ "cmpg-double",0x30,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 0 },
	{ "cmpg-float",	0x2e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 0 },
	{ "cmpl-double",0x2f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 0 },
	{ "cmpl-float",	0x2d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 0 },
	{ "const",	0x14,	AO_REG|AO_IMM_,			1, 1, 5, 0 },
	{ "const/16",	0x13,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-string",0x1a,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-wide",	0x18,	AO_REG|AO_IMM_,			1, 1, 8, 0 },
	{ "const-wide/16",0x16,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-wide/32",0x17,	AO_REG|AO_IMM_,			1, 1, 4, 0 },
	{ "div-int",	0x93,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "div-long",	0x9e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "goto",	0x28,	AO_IMM,				1, 1, 0, 0 },
	{ "goto/16",	0x2900,	AO_IMM,				2, 2, 0, 0 },
	{ "if-eqz",	0x38,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-gez",	0x3b,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-gtz",	0x3c,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-lez",	0x3d,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-ltz",	0x3a,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-nez",	0x39,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "monitor-enter",0x1d,	AO_REG,				1, 1, 0, 0 },
	{ "monitor-exit",0x1e,	AO_REG,				1, 1, 0, 0 },
	{ "move",	0x0100,	AO_v0|AO_v0_,			2, 0, 0, 0 },
	{ "move-exception",0x0d,AO_REG,				1, 1, 0, 0 },
	{ "move-result",0x0a,	AO_REG,				1, 1, 0, 0 },
	{ "move-result-wide",0x0b,AO_REG,			1, 1, 0, 0 },
	{ "mul-int",	0x92,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "mul-long",	0x9d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "nop",	0x0000,	AO_NONE,			2, 0, 0, 0 },
	{ "or-int",	0x96,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "rem-int",	0x94,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "rem-long",	0x9f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "return",	0x0f,	AO_REG,				1, 1, 0, 0 },
	{ "return-object",0x11,	AO_REG,				1, 1, 0, 0 },
	{ "return-void",0x0e00,	AO_NONE,			2, 0, 0, 0 },
	{ "shl-int",	0x98,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "shr-int",	0x99,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "sub-int",	0x91,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "sub-long",	0x9c,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "throw",	0x27,	AO_REG,				1, 1, 0, 0 },
	{ "ushr-int",	0x9a,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "xor-int",	0x97,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
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
