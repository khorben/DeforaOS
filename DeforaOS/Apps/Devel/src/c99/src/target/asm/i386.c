/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel C99 */
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



/* i386 */
/* prototypes */
static int _asm_arch_i386_function_begin(char const * name);
static int _asm_arch_i386_function_call(char const * name);
static int _asm_arch_i386_function_end(void);


/* variables */
static AsmTargetArch _asm_arch_i386 =
{
	"i386",
	_asm_arch_i386_function_begin,
	_asm_arch_i386_function_call,
	_asm_arch_i386_function_end
};


/* functions */
/* asm_arch_i386_function_begin */
static int _asm_arch_i386_function_begin(char const * name)
{
	AsmArchOperand arg1;
	AsmArchOperand arg2;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(asm_set_function(_asm_as, name, -1, -1) != 0)
		return -1;
	/* FIXME give real arguments */
	memset(&arg1, 0, sizeof(arg1));
	arg1.definition = AO_IMMEDIATE(0, 0, 32);
	arg1.value.immediate.value = 0;
	memset(&arg2, 0, sizeof(arg2));
	arg2.definition = AO_IMMEDIATE(0, 0, 32);
	arg2.value.immediate.value = 0;
	return asm_instruction(_asm_as, "enter", 2, &arg1, &arg2);
}


/* asm_arch_i386_function_call */
static int _asm_arch_i386_function_call(char const * name)
{
	AsmArchOperand arg;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	/* FIXME give a real argument */
	memset(&arg, 0, sizeof(arg));
	arg.definition = AO_IMMEDIATE(0, 0, 32);
	arg.value.immediate.value = 0;
	return asm_instruction(_asm_as, "call", 1, &arg);
}


/* asm_arch_i386_function_end */
static int _asm_arch_i386_function_end(void)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	return asm_instruction(_asm_as, "leave", 0);
}
