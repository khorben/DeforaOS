/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
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

# include "cpp.h"


/* types */
typedef struct _CppDefine /* FIXME use a hash table */
{
	char * name;
	char * value;
} CppDefine;

typedef enum _CppScope
{
	CPP_SCOPE_NOTYET = 0,
	CPP_SCOPE_TAKING,
	CPP_SCOPE_TAKEN
} CppScope;

/* FIXME make a subtype for the actual parser instead of the "parent" hack */
struct _Cpp
{
	int filters;
	Parser * parser;
	/* for cpp_filter_newlines */
	int newlines_last;
	int newlines_last_cnt;
	/* for cpp_filter_trigraphs */
	int trigraphs_last;
	int trigraphs_last_cnt;
	/* for cpp_callback_directive */
	int directive_newline;
	/* for include directives */
	Cpp * parent;
	Cpp * subparser;
	char ** paths;
	size_t paths_cnt;
	/* for substitutions */
	CppDefine * defines;
	size_t defines_cnt;
	/* for context */
	CppScope * scopes;
	size_t scopes_cnt;
};

#endif /* !_CPP_COMMON_H */
