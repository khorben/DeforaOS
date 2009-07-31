/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel cpp */
/* cpp is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * cpp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with cpp; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */


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
