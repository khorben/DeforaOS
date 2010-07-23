/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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


/* Format */
/* public */
/* types */
typedef struct _FormatPlugin FormatPlugin;

struct _FormatPlugin
{
	char const * filename;
	FILE * fp;

	int (*init)(FormatPlugin * format, char const * arch);
	int (*exit)(FormatPlugin * format);
	int (*function)(FormatPlugin * format, char const * function);
	int (*section)(FormatPlugin * format, char const * section);

	void * priv;
};

typedef struct _Format Format;


/* helpers */
#if BYTE_ORDER == BIG_ENDIAN 
# define _htob16(a) (a) 
# define _htol16(a) (((a) & 0xff) << 8 | ((a) & 0xff00) >> 8)
# define _htob32(a) (a) 
# define _htol32(a) (((a) & 0xff) << 24 | (((a) & 0xff00) << 8) \
		| (((a) & 0xff0000) >> 8) | ((a) & 0xff000000) >> 24)
# define _htob64(a) (a)
# define _htol64(a) (((a) & 0xff) << 56) | (((a) & 0xff00) << 40) \
		| (((a) & 0xff0000) << 24) | (((a) & 0xff000000) << 8) \
		| (((a) & 0xff00000000) >> 8) \
		| (((a) & 0xff0000000000) >> 24) \
		| (((a) & 0xff000000000000) >> 40) \
		| (((a) & 0xff00000000000000) >> 56)
#else
# define _htob16(a) (((a) & 0xff) << 8 | ((a) & 0xff00) >> 8)
# define _htol16(a) (a)
# define _htob32(a) (((a) & 0xff) << 24 | (((a) & 0xff00) << 8) \
		| (((a) & 0xff0000) >> 8) | ((a) & 0xff000000) >> 24)
# define _htol32(a) (a) 
# define _htob64(a) (((a) & 0xff) << 56) | (((a) & 0xff00) << 40) \
		| (((a) & 0xff0000) << 24) | (((a) & 0xff000000) << 8) \
		| (((a) & 0xff00000000) >> 8) \
		| (((a) & 0xff0000000000) >> 24) \
		| (((a) & 0xff000000000000) >> 40) \
		| (((a) & 0xff00000000000000) >> 56)
# define _htol64(a) (a)
#endif


/* functions */
Format * format_new(char const * format, char const * arch,
		char const * filename, FILE * fp);
int format_delete(Format * format);

/* useful */
int format_function(Format * format, char const * function);
int format_section(Format * format, char const * section);

#endif /* !AS_FORMAT_FORMAT_H */
