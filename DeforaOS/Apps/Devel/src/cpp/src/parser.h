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
int cppparser_include(CppParser * cppparser, char const * include);
int cppparser_inject(CppParser * cppparser, char const * string);
int cppparser_scan(CppParser * cppparser, Token ** token);

#endif /* !_CPP_PARSER_H */
