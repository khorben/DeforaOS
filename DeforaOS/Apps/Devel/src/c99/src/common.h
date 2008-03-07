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



#ifndef _C99_COMMON_H
# define _C99_COMMON_H

# include <stdio.h>
# include <cpp.h>


/* C99 */
/* protected */
/* types */
struct _C99
{
	int flags;                      /* FIXME get rid of it */
	FILE * outfp;
	Cpp * cpp;
	char * outfile;
	int optlevel;

	/* parsing */
	Token * token;
	int in_switch;
};

#endif /* !C99_C99_H */
