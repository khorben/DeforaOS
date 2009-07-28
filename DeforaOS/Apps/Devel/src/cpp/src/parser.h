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


#ifndef _CPP_PARSER_H
# define _CPP_PARSER_H

# include "cpp.h"


/* types */
typedef struct _CppParser CppParser;


/* functions */
CppParser * cppparser_new(Cpp * cpp, CppParser * parent, char const * filename,
		int filters);
void cppparser_delete(CppParser * cppparser);


/* accessors */
char const * cppparser_get_filename(CppParser * cppparser);


/* useful */
int cppparser_scan(CppParser * cppparser, Token ** token);

#endif /* !_CPP_PARSER_H */
