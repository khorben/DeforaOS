/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
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



#ifndef DEVEL_AS_AS_H
# define DEVEL_AS_AS_H


/* As */
/* types */
typedef struct _As As;

typedef enum _AsOperandType { AOT_IMMEDIATE = 0, AOT_REGISTER } AsOperandType;

typedef struct _AsOperand
{
	AsOperandType type;
	int dereference;
	void * value;
} AsOperand;

typedef enum _AsPluginType { ASPT_ARCH = 0, ASPT_FORMAT } AsPluginType;


/* functions */
As * as_new(char const * arch, char const * format);
void as_delete(As * as);


/* accessors */
char const * as_get_arch(As * as);
char const * as_get_format(As * as);


/* useful */
int as_parse(As * as, char const * infile, char const * outfile);

int as_open(As * as, char const * outfile);
int as_close(As * as);
int as_section(As * as, char const * name);
int as_function(As * as, char const * name);
int as_instruction(As * as, char const * name, unsigned int operands_cnt, ...);


/* plugins helpers */
int as_plugin_list(AsPluginType type);

#endif /* !DEVEL_AS_AS_H */