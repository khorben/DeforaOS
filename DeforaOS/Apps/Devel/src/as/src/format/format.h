/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
Format * format_new(char const * format, char const * arch);
void format_delete(Format * format);

/* accessors */
char const * format_get_name(Format * format);

/* useful */
int format_init(Format * format, char const * filename, FILE * fp);
int format_exit(Format * format);

int format_function(Format * format, char const * function);
int format_section(Format * format, char const * section);

#endif /* !AS_FORMAT_FORMAT_H */
