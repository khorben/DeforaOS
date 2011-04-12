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
	{ "add-double",	0xab,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "add-double/2addr",
			0xcb,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "add-float",	0xa6,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "add-float/2addr",
			0xc6,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "add-int",	0x90,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "add-int/2addr",0xb0,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "add-int/lit8",0xd8,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "add-int/lit16",0xd0,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "add-long",	0x9b,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "add-long/2addr",0xbb,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "aget",	0x44,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-boolean",0x47,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-byte",	0x48,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-char",	0x49,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-object",0x46,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-short",	0x4a,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aget-wide",	0x45,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "and-int",	0x95,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "and-int/2addr",0xb5,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "and-int/lit8",0xdd,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "and-int/lit16",0xd5,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "and-long",	0xa0,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "and-long/2addr",0xc0,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "aput",	0x4b,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-boolean",0x4e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-byte",	0x4f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-char",	0x50,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-object",0x4d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-short",	0x51,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "aput-wide",	0x4c,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "array-length",0x21,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "check-cast",	0x1f,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
#endif
	{ "cmp-long",	0x31,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "cmpg-double",0x30,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "cmpg-float",	0x2e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "cmpl-double",0x2f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "cmpl-float",	0x2d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "const",	0x14,	AO_REG|AO_IMM_,			1, 1, 4, 0 },
#if 1	/* XXX really implement */
	{ "const/4",	0x12,	AO_v0|AO_IMM_,			1, 0, 1, 0 },
#endif
	{ "const/16",	0x13,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const/high16",0x15,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-class",0x1c,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-string",0x1a,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-wide",	0x18,	AO_REG|AO_IMM_,			1, 1, 8, 0 },
	{ "const-wide/16",0x16,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "const-wide/32",0x17,	AO_REG|AO_IMM_,			1, 1, 4, 0 },
	{ "div-double",	0xae,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "div-double/2addr",
			0xce,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "div-float",	0xa9,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "div-float/2addr",
			0xc9,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "div-int",	0x93,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "div-int/2addr",0xb3,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "div-int/lit8",0xdb,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "div-int/lit16",0xd3,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "div-long",	0x9e,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "div-long/2addr",0xbe,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "double-to-float",0x8c,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "double-to-int",0x8a,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "double-to-long",0x8b,AO_v0|AO_REG_,			1, 0, 1, 0 },
#if 1	/* XXX really implement */
	{ "fill-array-data",0x26,AO_REG|AO_IMM_,		1, 1, 4, 0 },
	{ "filled-new-array",0x24,AO_REG|AO_IMM_,		1, 1, 4, 0 },
	{ "filled-new-array-range",
			0x25,	AO_REG|AO_IMM_,			1, 1, 4, 0 },
#endif
	{ "float-to-double",0x89,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "float-to-int",0x8a,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "float-to-long",0x88,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "goto",	0x28,	AO_IMM,				1, 1, 0, 0 },
	{ "goto/16",	0x2900,	AO_IMM,				2, 2, 0, 0 },
	{ "if-eq",	0x32,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-eqz",	0x38,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-ge",	0x35,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-gez",	0x3b,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-gt",	0x36,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-gtz",	0x3c,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-le",	0x37,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-lez",	0x3d,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-lt",	0x34,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-ltz",	0x3a,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "if-ne",	0x33,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "if-nez",	0x39,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
#if 1	/* XXX really implement */
	{ "iget",	0x52,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-boolean",0x55,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-byte",	0x56,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-char",	0x57,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-object",0x54,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-short",	0x58,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iget-wide",	0x53,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "int-to-byte",0x8d,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "int-to-char",0x8e,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "int-to-float",0x82,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "int-to-long",0x81,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "int-to-short",0x8f,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "invoke-direct",0x70,	AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-direct/range",
			0x76,	AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-interface",0x72,AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-interface-range",
			0x78,AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-static",0x71,AO_IMM|AO_IMM_|AO_IMM__,		1, 1, 2, 2 },
	{ "invoke-static/range",
			0x77,	AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-super",0x6f,AO_IMM|AO_IMM_|AO_IMM__,		1, 1, 2, 2 },
	{ "invoke-super/range",
			0x75,	AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-virtual",0x6e,AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "invoke-virtual/range",
			0x74,	AO_IMM|AO_IMM_|AO_IMM__,	1, 1, 2, 2 },
	{ "iput",	0x59,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-boolean",0x5c,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-byte",	0x5d,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-char",	0x5e,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-object",0x5b,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-short",	0x5f,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "iput-wide",	0x5a,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "long-to-double",0x86,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "long-to-float",0x85,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "long-to-int",0x84,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "monitor-enter",0x1d,	AO_REG,				1, 1, 0, 0 },
	{ "monitor-exit",0x1e,	AO_REG,				1, 1, 0, 0 },
#if 1 /* XXX really implement */
	{ "move",	0x01,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "move/16",	0x03,	AO_REG|AO_REG_,			1, 2, 1, 0 },
	{ "move/from16",0x02,	AO_REG|AO_REG_,			1, 1, 2, 0 },
	{ "move-exception",0x0d,AO_REG,				1, 1, 0, 0 },
#if 1 /* XXX really implement */
	{ "move-object",0x07,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "move-object/16",0x09,AO_v0|AO_REG_,			1, 2, 1, 0 },
	{ "move-object/from16",0x08,AO_v0|AO_REG_,		1, 1, 2, 0 },
#endif
	{ "move-result",0x0a,	AO_REG,				1, 1, 0, 0 },
	{ "move-result-object",0x0c,AO_REG,			1, 1, 0, 0 },
	{ "move-result-wide",0x0b,AO_REG,			1, 1, 0, 0 },
#if 1 /* XXX really implement */
	{ "move-wide",0x04,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "move-wide/16",0x06,	AO_REG|AO_REG_,			1, 2, 1, 0 },
	{ "move-wide/from16",0x05,AO_REG|AO_REG_,		1, 1, 2, 0 },
	{ "mul-double",	0xad,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "mul-double/2addr",
			0xcd,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "mul-float",	0xa8,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "mul-float/2addr",
			0xc8,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "mul-int",	0x92,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "mul-int/2addr",0xb2,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "mul-int/lit8",0xda,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "mul-int/lit16",0xd2,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "mul-long",	0x9d,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "mul-long/2addr",0xbd,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "neg-double",	0x80,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "neg-float",	0x7f,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "neg-int",	0x7b,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "neg-long",	0x7d,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "new-array",	0x23,	AO_v0|AO_REG_|AO_IMM__,		1, 0, 1, 2 },
	{ "new-instance",0x22,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
#endif
	{ "nop",	0x0000,	AO_NONE,			2, 0, 0, 0 },
#if 1	/* XXX really implement */
	{ "not-int",	0x7c,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "not-long",	0x7e,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "or-int",	0x96,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "or-int/2addr",0xb6,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "or-int/lit8",0xdb,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "or-int/lit16",0xd6,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "or-long",	0xa1,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "or-long/2addr",0xc1,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "rem-double",	0xaf,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "rem-double/2addr",
			0xcf,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "rem-float",	0xaa,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "rem-float/2addr",
			0xca,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "rem-int",	0x94,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "rem-int/2addr",0xb4,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "rem-int/lit8",0xdc,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "rem-int/lit16",0xd4,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "rem-long",	0x9f,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "rem-long/2addr",0xbf,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "return",	0x0f,	AO_REG,				1, 1, 0, 0 },
	{ "return-object",0x11,	AO_REG,				1, 1, 0, 0 },
	{ "return-void",0x0e00,	AO_NONE,			2, 0, 0, 0 },
	{ "return-void",0x0e,	AO_IMM,				1, 1, 0, 0 },
	{ "return-wide",0x10,	AO_REG,				1, 1, 0, 0 },
	{ "sget",	0x60,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-boolean",0x63,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-byte",	0x64,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-char",	0x65,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-object",0x62,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-short",	0x66,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sget-wide",	0x61,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "shl-int",	0x98,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "shl-int/2addr",0xb8,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "shl-int/lit8",0xe0,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "shl-long",	0xa3,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "shl-long/2addr",0xc3,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "shr-int",	0x99,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "shr-int/2addr",0xb9,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "shr-int/lit8",0xe1,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "shr-long",	0xa4,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "shr-long/2addr",0xc4,AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "sparse-switch",0x2c,	AO_REG|AO_IMM_,			1, 1, 4, 0 },
#endif
	{ "sput",	0x67,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-boolean",0x6a,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-byte",	0x6b,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-char",	0x6c,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-object",0x69,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-short",	0x6d,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sput-wide",	0x68,	AO_REG|AO_IMM_,			1, 1, 2 ,0 },
	{ "sub-double",	0xac,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "sub-double/2addr",
			0xcc,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "sub-float",	0xa7,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
	{ "sub-float/2addr",
			0xc7,	AO_v0|AO_REG_,			1, 0, 1, 0 },
	{ "sub-int",	0x91,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "sub-int/2addr",0xb1,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "sub-int/lit8",0xd9,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "sub-int/lit16",0xd1,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "sub-long",	0x9c,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "sub-long/2addr",0xbc,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "throw",	0x27,	AO_REG,				1, 1, 0, 0 },
	{ "ushr-int",	0x9a,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "ushr-int/2addr",0xba,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "ushr-int/lit8",0xe2,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "ushr-long",	0xa5,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "ushr-long/2addr",0xc5,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "xor-int",	0x97,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "xor-int/2addr",0xb7,	AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
	{ "xor-int/lit8",0xdf,	AO_REG|AO_REG_|AO_IMM__,	1, 1, 1, 1 },
	{ "xor-int/lit16",0xd7,	AO_REG|AO_IMM_,			1, 1, 2, 0 },
	{ "xor-long",	0xa2,	AO_REG|AO_REG_|AO_REG__,	1, 1, 1, 1 },
#if 1	/* XXX implement correctly */
	{ "xor-long/2addr",0xc2,AO_v0|AO_REG_,			1, 0, 1, 0 },
#endif
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
