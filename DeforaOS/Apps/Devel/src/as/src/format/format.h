/* format/format.h */
/* FIXME
 * - get rid of format_init() and format_exit() (should be handled in
 *   format_new() and format_delete()
 * - format plug-ins should be told and aware of the target architecture */



#ifndef __FORMAT_FORMAT_H
# define __FORMAT_FORMAT_H

# include <stdio.h>


/* types */

typedef struct _FormatPlugin
{
	int (*format_init)(FILE * fp);
	int (*format_exit)(FILE * fp);
} FormatPlugin;

typedef struct _Format
{
	int (*format_init)(FILE * fp);
	int (*format_exit)(FILE * fp);
	void * plugin;
} Format;


/* functions */
Format * format_new(char * format);
void format_delete(Format * format);

/* useful */
int format_init(Format * format, FILE * fp);
int format_exit(Format * format, FILE * fp);

int format_section(Format * format, char * section);

#endif /* !__FORMAT_FORMAT_H */
