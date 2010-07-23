/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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


/* Java */
/* private */
/* variables */
enum
{
#include "common.reg"
};

static ArchRegister _java_regs[] =
{
	{ NULL,		0, 0 }
};

static ArchInstruction _java_set[] =
{
	{ "aaload",	0x32,	AO_NONE,	1, 0, 0, 0 },
	{ "aastore",	0x53,	AO_NONE,	1, 0, 0, 0 },
	{ "aconst_null",0x01,	AO_NONE,	1, 0, 0, 0 },
	{ "aload",	0x19,	AO_IMM,		1, 1, 0, 0 },
	{ "aload_0",	0x2a,	AO_NONE,	1, 0, 0, 0 },
	{ "aload_1",	0x2b,	AO_NONE,	1, 0, 0, 0 },
	{ "aload_2",	0x2c,	AO_NONE,	1, 0, 0, 0 },
	{ "aload_3",	0x2d,	AO_NONE,	1, 0, 0, 0 },
	{ "areturn",	0xb0,	AO_NONE,	1, 0, 0, 0 },
	{ "arraylength",0xbe,	AO_NONE,	1, 0, 0, 0 },
	{ "astore",	0x3a,	AO_IMM,		1, 1, 0, 0 },
	{ "astore_0",	0x4b,	AO_NONE,	1, 0, 0, 0 },
	{ "astore_1",	0x4c,	AO_NONE,	1, 0, 0, 0 },
	{ "astore_2",	0x4d,	AO_NONE,	1, 0, 0, 0 },
	{ "astore_3",	0x4e,	AO_NONE,	1, 0, 0, 0 },
	{ "athrow",	0xbf,	AO_NONE,	1, 0, 0, 0 },
	{ "baload",	0x33,	AO_NONE,	1, 0, 0, 0 },
	{ "bastore",	0x54,	AO_NONE,	1, 0, 0, 0 },
	{ "bipush",	0x10,	AO_IMM,		1, 4, 0, 0 },
	{ "caload",	0x34,	AO_NONE,	1, 0, 0, 0 },
	{ "castore",	0x55,	AO_NONE,	1, 0, 0, 0 },
	{ "checkcast",	0xc0,	AO_IMM|AO_IMM_,	1, 1, 1, 0 },
	{ "d2f",	0x90,	AO_NONE,	1, 0, 0, 0 },
	{ "d2i",	0x8e,	AO_NONE,	1, 0, 0, 0 },
	{ "d2l",	0x8f,	AO_NONE,	1, 0, 0, 0 },
	{ "dadd",	0x63,	AO_NONE,	1, 0, 0, 0 },
	{ "daload",	0x31,	AO_NONE,	1, 0, 0, 0 },
	{ "dastore",	0x52,	AO_NONE,	1, 0, 0, 0 },
	{ "dcmpg",	0x98,	AO_NONE,	1, 0, 0, 0 },
	{ "dcmpl",	0x97,	AO_NONE,	1, 0, 0, 0 },
	{ "dconst_0",	0x0e,	AO_NONE,	1, 0, 0, 0 },
	{ "dconst_1",	0x0f,	AO_NONE,	1, 0, 0, 0 },
	{ "ddiv",	0x6f,	AO_NONE,	1, 0, 0, 0 },
	{ "dload",	0x18,	AO_IMM,		1, 1, 0, 0 },
	{ "dload_0",	0x26,	AO_NONE,	1, 0, 0, 0 },
	{ "dload_1",	0x27,	AO_NONE,	1, 0, 0, 0 },
	{ "dload_2",	0x28,	AO_NONE,	1, 0, 0, 0 },
	{ "dload_3",	0x29,	AO_NONE,	1, 0, 0, 0 },
	{ "dmul",	0x6b,	AO_NONE,	1, 0, 0, 0 },
	{ "dneg",	0x77,	AO_NONE,	1, 0, 0, 0 },
	{ "drem",	0x73,	AO_NONE,	1, 0, 0, 0 },
	{ "dreturn",	0xaf,	AO_NONE,	1, 0, 0, 0 },
	{ "dstore",	0x39,	AO_IMM,		1, 1, 0, 0 },
	{ "dstore_0",	0x47,	AO_NONE,	1, 0, 0, 0 },
	{ "dstore_1",	0x48,	AO_NONE,	1, 0, 0, 0 },
	{ "dstore_2",	0x49,	AO_NONE,	1, 0, 0, 0 },
	{ "dstore_3",	0x4a,	AO_NONE,	1, 0, 0, 0 },
	{ "dsub",	0x67,	AO_NONE,	1, 0, 0, 0 },
	{ "dup",	0x59,	AO_NONE,	1, 0, 0, 0 },
	{ "dup_x1",	0x5a,	AO_NONE,	1, 0, 0, 0 },
	{ "dup_x2",	0x5b,	AO_NONE,	1, 0, 0, 0 },
	{ "dup2",	0x5c,	AO_NONE,	1, 0, 0, 0 },
	{ "dup2_x1",	0x5d,	AO_NONE,	1, 0, 0, 0 },
	{ "dup2_x2",	0x5e,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_0",	0x0b,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_1",	0x0c,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_2",	0x0d,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_0",	0x22,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_1",	0x23,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_2",	0x24,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_3",	0x25,	AO_NONE,	1, 0, 0, 0 },
	{ "freturn",	0xae,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_0",	0x43,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_1",	0x44,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_2",	0x45,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_3",	0x46,	AO_NONE,	1, 0, 0, 0 },
	{ "lor",	0x81,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_0",	0x3f,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_1",	0x40,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_2",	0x41,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_3",	0x42,	AO_NONE,	1, 0, 0, 0 },
	{ "monitorenter",0xc2,	AO_NONE,	1, 0, 0, 0 },
	{ "monitorexit",0xc3,	AO_NONE,	1, 0, 0, 0 },
	{ "pop",	0x57,	AO_NONE,	1, 0, 0, 0 },
	{ "pop2",	0x58,	AO_NONE,	1, 0, 0, 0 },
	{ "return",	0xb1,	AO_NONE,	1, 0, 0, 0 },
	{ "xxxunusedxxx",0xba,	AO_NONE,	1, 0, 0, 0 },
#include "null.ins"
};


/* public */
/* variables */
ArchPlugin arch_plugin =
{
	_java_regs,
	_java_set
};
