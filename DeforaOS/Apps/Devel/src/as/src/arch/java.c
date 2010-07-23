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
	{ "checkcast",	0xc0,	AO_IMM,		1, 2, 0, 0 },
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
	{ "f2d",	0x8d,	AO_NONE,	1, 0, 0, 0 },
	{ "f2i",	0x8b,	AO_NONE,	1, 0, 0, 0 },
	{ "f2l",	0x8c,	AO_NONE,	1, 0, 0, 0 },
	{ "fadd",	0x62,	AO_NONE,	1, 0, 0, 0 },
	{ "faload",	0x30,	AO_NONE,	1, 0, 0, 0 },
	{ "fastore",	0x51,	AO_NONE,	1, 0, 0, 0 },
	{ "fcmpg",	0x96,	AO_NONE,	1, 0, 0, 0 },
	{ "fcmpl",	0x95,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_0",	0x0b,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_1",	0x0c,	AO_NONE,	1, 0, 0, 0 },
	{ "fconst_2",	0x0d,	AO_NONE,	1, 0, 0, 0 },
	{ "fdiv",	0x6e,	AO_NONE,	1, 0, 0, 0 },
	{ "fload",	0x17,	AO_IMM,		1, 1, 0, 0 },
	{ "fload_0",	0x22,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_1",	0x23,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_2",	0x24,	AO_NONE,	1, 0, 0, 0 },
	{ "fload_3",	0x25,	AO_NONE,	1, 0, 0, 0 },
	{ "fmul",	0x6a,	AO_NONE,	1, 0, 0, 0 },
	{ "fneg",	0x76,	AO_NONE,	1, 0, 0, 0 },
	{ "frem",	0x72,	AO_NONE,	1, 0, 0, 0 },
	{ "freturn",	0xae,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_0",	0x43,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_1",	0x44,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_2",	0x45,	AO_NONE,	1, 0, 0, 0 },
	{ "fstore_3",	0x46,	AO_NONE,	1, 0, 0, 0 },
	{ "fsub",	0x66,	AO_NONE,	1, 0, 0, 0 },
	{ "getfield",	0xb4,	AO_IMM,		1, 2, 0, 0 },
	{ "getstatic",	0xb2,	AO_IMM,		1, 2, 0, 0 },
	{ "goto",	0xa7,	AO_IMM,		1, 2, 0, 0 },
	{ "goto_w",	0xc8,	AO_IMM,		1, 4, 0, 0 },
	{ "i2b",	0x91,	AO_NONE,	1, 0, 0, 0 },
	{ "i2c",	0x92,	AO_NONE,	1, 0, 0, 0 },
	{ "i2d",	0x87,	AO_NONE,	1, 0, 0, 0 },
	{ "i2f",	0x86,	AO_NONE,	1, 0, 0, 0 },
	{ "i2l",	0x85,	AO_NONE,	1, 0, 0, 0 },
	{ "i2s",	0x93,	AO_NONE,	1, 0, 0, 0 },
	{ "iadd",	0x60,	AO_NONE,	1, 0, 0, 0 },
	{ "iaload",	0x2e,	AO_NONE,	1, 0, 0, 0 },
	{ "iand",	0x7e,	AO_NONE,	1, 0, 0, 0 },
	{ "iastore",	0x4f,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_m1",	0x02,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_0",	0x03,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_1",	0x04,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_2",	0x05,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_3",	0x06,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_4",	0x07,	AO_NONE,	1, 0, 0, 0 },
	{ "iconst_5",	0x08,	AO_NONE,	1, 0, 0, 0 },
	{ "idiv",	0x6c,	AO_NONE,	1, 0, 0, 0 },
	{ "if_acmpeq",	0xa5,	AO_IMM,		1, 2, 0, 0 },
	{ "if_acmpne",	0xa6,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmpeq",	0x9f,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmpne",	0xa0,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmplt",	0xa1,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmpge",	0xa2,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmpgt",	0xa3,	AO_IMM,		1, 2, 0, 0 },
	{ "if_icmple",	0xa4,	AO_IMM,		1, 2, 0, 0 },
	{ "ifeq",	0x99,	AO_IMM,		1, 2, 0, 0 },
	{ "ifne",	0x9a,	AO_IMM,		1, 2, 0, 0 },
	{ "iflt",	0x9b,	AO_IMM,		1, 2, 0, 0 },
	{ "ifge",	0x9c,	AO_IMM,		1, 2, 0, 0 },
	{ "ifgt",	0x9d,	AO_IMM,		1, 2, 0, 0 },
	{ "ifle",	0x9e,	AO_IMM,		1, 2, 0, 0 },
	{ "ifnonnull",	0xc7,	AO_IMM,		1, 2, 0, 0 },
	{ "ifnull",	0xc6,	AO_IMM,		1, 2, 0, 0 },
	{ "iinc",	0x84,	AO_NONE,	1, 0, 0, 0 },
	{ "iload",	0x15,	AO_IMM,		1, 1, 0, 0 },
	{ "iload_0",	0x1a,	AO_NONE,	1, 0, 0, 0 },
	{ "iload_1",	0x1b,	AO_NONE,	1, 0, 0, 0 },
	{ "iload_2",	0x1c,	AO_NONE,	1, 0, 0, 0 },
	{ "iload_3",	0x1d,	AO_NONE,	1, 0, 0, 0 },
	{ "impdep1",	0xfe,	AO_NONE,	1, 0, 0, 0 },
	{ "impdep2",	0xff,	AO_NONE,	1, 0, 0, 0 },
	{ "imul",	0x68,	AO_NONE,	1, 0, 0, 0 },
	{ "ineg",	0x74,	AO_NONE,	1, 0, 0, 0 },
	{ "instanceof",	0xc1,	AO_IMM,		1, 2, 0, 0 },
	{ "l2d",	0x8a,	AO_NONE,	1, 0, 0, 0 },
	{ "l2f",	0x89,	AO_NONE,	1, 0, 0, 0 },
	{ "l2i",	0x88,	AO_NONE,	1, 0, 0, 0 },
	{ "ladd",	0x61,	AO_NONE,	1, 0, 0, 0 },
	{ "laload",	0x2f,	AO_NONE,	1, 0, 0, 0 },
	{ "land",	0x7f,	AO_NONE,	1, 0, 0, 0 },
	{ "lastore",	0x50,	AO_NONE,	1, 0, 0, 0 },
	{ "lcmp",	0x94,	AO_NONE,	1, 0, 0, 0 },
	{ "lconst_0",	0x09,	AO_NONE,	1, 0, 0, 0 },
	{ "lconst_1",	0x0a,	AO_NONE,	1, 0, 0, 0 },
	{ "ldc",	0x12,	AO_IMM,		1, 1, 0, 0 },
	{ "ldc_w",	0x13,	AO_IMM,		1, 2, 0, 0 },
	{ "ldc2_w",	0x14,	AO_IMM,		1, 2, 0, 0 },
	{ "ldiv",	0x6d,	AO_NONE,	1, 0, 0, 0 },
	{ "lload",	0x16,	AO_IMM,		1, 1, 0, 0 },
	{ "lload_0",	0x1e,	AO_NONE,	1, 0, 0, 0 },
	{ "lload_1",	0x1f,	AO_NONE,	1, 0, 0, 0 },
	{ "lload_2",	0x20,	AO_NONE,	1, 0, 0, 0 },
	{ "lload_3",	0x21,	AO_NONE,	1, 0, 0, 0 },
	{ "lmul",	0x69,	AO_NONE,	1, 0, 0, 0 },
	{ "lneg",	0x75,	AO_NONE,	1, 0, 0, 0 },
	{ "lor",	0x81,	AO_NONE,	1, 0, 0, 0 },
	{ "lrem",	0x71,	AO_NONE,	1, 0, 0, 0 },
	{ "lreturn",	0xad,	AO_NONE,	1, 0, 0, 0 },
	{ "lshl",	0x79,	AO_NONE,	1, 0, 0, 0 },
	{ "lshr",	0x7b,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore",	0x37,	AO_IMM,		1, 1, 0, 0 },
	{ "lstore_0",	0x3f,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_1",	0x40,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_2",	0x41,	AO_NONE,	1, 0, 0, 0 },
	{ "lstore_3",	0x42,	AO_NONE,	1, 0, 0, 0 },
	{ "lsub",	0x65,	AO_NONE,	1, 0, 0, 0 },
	{ "lushr",	0x7d,	AO_NONE,	1, 0, 0, 0 },
	{ "lxor",	0x83,	AO_NONE,	1, 0, 0, 0 },
	{ "monitorenter",0xc2,	AO_NONE,	1, 0, 0, 0 },
	{ "monitorexit",0xc3,	AO_NONE,	1, 0, 0, 0 },
	{ "new",	0xbb,	AO_IMM,		1, 2, 0, 0 },
	{ "newarray",	0xbb,	AO_IMM,		1, 1, 0, 0 },
	{ "nop",	0x00,	AO_NONE,	1, 0, 0, 0 },
	{ "pop",	0x57,	AO_NONE,	1, 0, 0, 0 },
	{ "pop2",	0x58,	AO_NONE,	1, 0, 0, 0 },
	{ "putfield",	0xb5,	AO_IMM,		1, 2, 0, 0 },
	{ "putstatic",	0xb3,	AO_IMM,		1, 2, 0, 0 },
	{ "ret",	0xa9,	AO_IMM,		1, 1, 0, 0 },
	{ "return",	0xb1,	AO_NONE,	1, 0, 0, 0 },
	{ "saload",	0x35,	AO_NONE,	1, 0, 0, 0 },
	{ "sastore",	0x56,	AO_NONE,	1, 0, 0, 0 },
	{ "sipush",	0x11,	AO_IMM,		1, 2, 0, 0 },
	{ "swap",	0x5f,	AO_NONE,	1, 0, 0, 0 },
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
