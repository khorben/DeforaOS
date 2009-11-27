/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
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
int makefile(Configure * configure, String const * directory, configArray * ca,
	       	int from, int to);
		

#endif /* !CONFIGURE_MAKEFILE_H */
