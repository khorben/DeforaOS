/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
/* configure is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * configure is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with configure; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef CONFIGURE_MAKEFILE_H
# define CONFIGURE_MAKEFILE_H

# include <System.h>
# include "configure.h"


/* types */
/* FIXME should be:
ARRAY(Config *, config);
but it can't be included multiple times */
typedef Array configArray;
extern configArray * configarray_new(void);


/* functions */
int makefile(Configure * configure, String * directory, configArray * ca,
	       	int from, int to);
		

#endif /* !CONFIGURE_MAKEFILE_H */
