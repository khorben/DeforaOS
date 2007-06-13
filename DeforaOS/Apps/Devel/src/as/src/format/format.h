/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#ifndef AS_FORMAT_FORMAT_H
# define AS_FORMAT_FORMAT_H

# include <stdio.h>


/* types */
typedef struct _FormatPlugin
{
	char const * filename;

	int (*init)(FILE * fp, char const * arch);
	int (*exit)(FILE * fp);
	int (*section)(FILE * fp, char const * section);
} FormatPlugin;

typedef struct _Format
{
	char * arch;
	FormatPlugin * plugin;
	void * handle;
} Format;


/* functions */
Format * format_new(char const * format, char const * arch,
		char const * filename);
int format_delete(Format * format, FILE * fp);

/* useful */
int format_init(Format * format, FILE * fp);
int format_exit(Format * format, FILE * fp);
int format_section(Format * format, FILE * fp, char const * section);

#endif /* !AS_FORMAT_FORMAT_H */
