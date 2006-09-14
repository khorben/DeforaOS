/* compose.h */



#ifndef MAILER_COMPOSE_H
# define MAILER_COMPOSE_H

# include "mailer.h"


/* types */
typedef struct _Compose
{
	Mailer * mailer;

	/* widgets */
	GtkWidget * window;
	GtkWidget * statusbar;
	gint statusbar_id;
} Compose;

/* methods */
Compose * compose_new(Mailer * mailer);
void compose_delete(Compose * compose);

#endif /* !MAILER_COMPOSE_H */
