/* format/format.h */
/* FIXME
 * - get rid of format_init() and format_exit(): should be handled in
 *   format_new() and format_delete() */



#ifndef __FORMAT_FORMAT_H
# define __FORMAT_FORMAT_H

# include <stdio.h>


/* types */
typedef struct _FormatPlugin
{
	int (*format_init)(FILE * fp, char * arch);
	int (*format_exit)(FILE * fp);
	int (*format_section)(FILE * fp, char * section);
} FormatPlugin;

typedef struct _Format
{
	char * arch;
	int (*format_init)(FILE * fp, char * arch);
	int (*format_exit)(FILE * fp);
	int (*format_section)(FILE * fp, char * section);
	void * plugin;
} Format;


/* functions */
Format * format_new(char * format, char * arch);
void format_delete(Format * format, FILE * fp);

/* useful */
int format_init(Format * format, FILE * fp);
int format_exit(Format * format, FILE * fp);
int format_section(Format * format, FILE * fp, char * section);

#endif /* !__FORMAT_FORMAT_H */
