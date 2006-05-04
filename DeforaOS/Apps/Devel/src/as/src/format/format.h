/* format/format.h */
/* FIXME
 * - get rid of format_init() and format_exit(): should be handled in
 *   format_new() and format_delete() */



#ifndef AS_FORMAT_FORMAT_H
# define AS_FORMAT_FORMAT_H

# include <stdio.h>


/* types */
typedef struct _FormatPlugin
{
	int (*init)(FILE * fp, char * arch);
	int (*exit)(FILE * fp);
	int (*section)(FILE * fp, char * section);
} FormatPlugin;

typedef struct _Format
{
	char * arch;
	FormatPlugin * plugin;
	void * handle;
} Format;


/* functions */
Format * format_new(char * format, char * arch);
int format_delete(Format * format, FILE * fp);

/* useful */
int format_init(Format * format, FILE * fp);
int format_exit(Format * format, FILE * fp);
int format_section(Format * format, FILE * fp, char * section);

#endif /* !AS_FORMAT_FORMAT_H */
