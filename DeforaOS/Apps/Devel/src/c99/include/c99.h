/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef C99_C99_H
# define C99_C99_H

# include <cpp.h>


/* C99 */
/* private */
/* types */
typedef struct _C99 C99;


/* public */
/* types */
typedef struct _C99Helper
{
	struct _C99 * c99;
	int (*define_add)(C99 * c99, char const * name, char const * value);
} C99Helper;

typedef struct _C99Option
{
	char const * name;
	char const * value;
} C99Option;

typedef struct _C99Prefs
{
	int flags;
	char const * outfile;
	const char ** paths;
	size_t paths_cnt;
	char ** defines;
	size_t defines_cnt;
	const char ** undefines;
	size_t undefines_cnt;
	int optlevel;
	int warn;
	char const * target;
	C99Option * options;
	size_t options_cnt;
} C99Prefs;
# define C99PREFS_c 0x1
# define C99PREFS_E 0x2
# define C99PREFS_g 0x4
# define C99PREFS_s 0x8


/* functions */
C99 * c99_new(C99Prefs const * prefs, char const * pathname);
int c99_delete(C99 * c99);

/* useful */
int c99_define_add(C99 * c99, char const * name, char const * value);

int c99_parse(C99 * c99);

#endif /* !C99_C99_H */
