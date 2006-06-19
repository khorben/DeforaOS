/* mime.h */



#ifndef BROWSER_MIME_H
# define BROWSER_MIME_H


/* Mime */
/* types */
typedef struct _MimeType
{
	char * type;
	char * glob;
	GdkPixbuf * icon;
} MimeType;

typedef struct _Mime
{
	MimeType * types;
	unsigned int types_cnt;
} Mime;


/* functions */
Mime * mime_new(void);
void mime_delete(Mime * mime);

/* useful */
char const * mime_type(Mime * mime, char const * path);
GdkPixbuf * mime_icon(Mime * mime, GtkIconTheme * theme, char const * type);

#endif /* !BROWSER_MIME_H */
