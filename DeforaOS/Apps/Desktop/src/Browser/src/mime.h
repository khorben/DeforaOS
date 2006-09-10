/* mime.h */



#ifndef BROWSER_MIME_H
# define BROWSER_MIME_H

# include <System.h>
# include <gtk/gtk.h>

# define MIME_CONFIG_FILE ".mime"


/* Mime */
/* types */
typedef struct _MimeType
{
	char * type;
	char * glob;
	GdkPixbuf * icon_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48;
#endif
} MimeType;

typedef struct _Mime
{
	MimeType * types;
	unsigned int types_cnt;
	Config * config;
} Mime;


/* functions */
Mime * mime_new(void);
void mime_delete(Mime * mime);

/* useful */
char const * mime_type(Mime * mime, char const * path);
/* FIXME return an enum with error codes? */
int mime_action(Mime * mime, char const * action, char const * path);
GdkPixbuf * mime_icons(Mime * mime, GtkIconTheme * theme, char const * type,
		GdkPixbuf ** icon_48);

#endif /* !BROWSER_MIME_H */
