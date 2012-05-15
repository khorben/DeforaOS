/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel c99 */
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



#ifndef DEVEL_C99_TARGET_H
# define DEVEL_C99_TARGET_H

# include <cpp.h>
# include "c99.h"


/* C99Target */
/* public */
/* types */
typedef struct _C99TargetPlugin
{
	C99Helper * helper;
	C99Option * options;
	int (*init)(char const * outfile, int optlevel);
	int (*exit)(void);
	int (*token)(Token * token);
	int (*section)(char const * name);
	int (*function_begin)(char const * name);
	int (*function_call)(char const * name);
	int (*function_end)(void);
	int (*label_set)(char const * name);
} C99TargetPlugin;

#endif /* !DEVEL_C99_TARGET_H */
