/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
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



#ifndef _CPP_COMMON_H
# define _CPP_COMMON_H

# include "parser.h"
# include "cpp.h"


/* types */
typedef struct _CppDefine CppDefine;

typedef enum _CppScope
{
	CPP_SCOPE_NOTYET = 0,
	CPP_SCOPE_TAKING,
	CPP_SCOPE_TAKEN
} CppScope;

struct _Cpp
{
	int options;
	/* for include directives */
	CppParser * parser;
	char ** paths;
	size_t paths_cnt;
	/* for substitutions */
	CppDefine * defines;
	size_t defines_cnt;
	/* for context */
	CppScope * scopes;
	size_t scopes_cnt;
};


/* functions */
char * cpp_path_lookup(Cpp * cpp, char const * filename);

#endif /* !_CPP_COMMON_H */
