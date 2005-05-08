/* format/format.h */



#ifndef __FORMAT_FORMAT_H
# define __FORMAT_FORMAT_H


/* types */
typedef struct _Format {
} Format;


/* functions */
Format * format_new(char * format);
void format_delete(Format * format);

#endif /* !__FORMAT_FORMAT_H */
