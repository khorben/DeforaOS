/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel as */
/* as is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * as is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with as; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#ifndef AS_AS_H
# define AS_AS_H


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


/* constants */
# define AS_FILENAME_DEFAULT "a.out"


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

#endif /* !AS_AS_H */
