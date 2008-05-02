/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
/* c99 is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * c99 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with c99; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <as.h>
#include <stdlib.h>
#include "target.h"


/* sparc64 */
/* private */
/* variables */
As * as;


/* protected */
/* prototypes */
static int _sparc64_init(char const * arch, char const * format);
static int _sparc64_exit(void);


/* public */
/* variables */
TargetPlugin target_plugin =
{
	_sparc64_init,
	_sparc64_exit
};


/* protected */
/* functions */
/* sparc64_init */
static int _sparc64_init(char const * arch, char const * format)
{
	if((as = as_new(arch, format)) == NULL)
		return 1;
	return 0;
}


/* sparc64_exit */
static int _sparc64_exit(void)
{
	as_delete(as);
	return 0;
}
