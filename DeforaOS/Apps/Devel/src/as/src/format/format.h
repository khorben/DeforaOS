/* format/format.h */



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

#endif /* !__FORMAT_FORMAT_H */
