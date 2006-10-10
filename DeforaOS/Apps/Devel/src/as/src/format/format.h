/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
