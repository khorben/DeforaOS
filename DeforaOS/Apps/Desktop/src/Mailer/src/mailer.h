/* mailer.h */



#ifndef MAILER_MAILER_H
# define MAILER_MAILER_H

# include <gtk/gtk.h>


/* Mailer */
/* types */
typedef struct _Mailer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * from;
	GtkWidget * to;
	GtkWidget * subject;
	GtkWidget * date;
	GtkWidget * view_body;
	GtkWidget * statusbar;
	gint statusbar_id;
} Mailer;


/* functions */
Mailer * mailer_new(void);
void mailer_delete(Mailer * mailer);

/* useful */
int mailer_error(Mailer * mailer, char const * message, int ret);

#endif
