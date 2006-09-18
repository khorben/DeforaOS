/* compose.h */



#ifndef MAILER_COMPOSE_H
# define MAILER_COMPOSE_H

# include "mailer.h"


/* types */
typedef struct _Compose
{
	Mailer * mailer;

	/* sending mail */
	pid_t pid;
	int fd;
	char * buf;
	size_t buf_len;
	size_t buf_pos;
	GIOChannel * channel;
	GtkWidget * snd_window;
	GtkWidget * snd_progress;

	/* widgets */
	GtkWidget * window;
	GtkWidget * from;
	GtkWidget * to;
	GtkWidget * tb_cc;
	GtkWidget * cc;
	GtkWidget * tb_bcc;
	GtkWidget * bcc;
	GtkWidget * subject;
	GtkWidget * view;
	GtkWidget * statusbar;
	gint statusbar_id;
} Compose;

/* methods */
Compose * compose_new(Mailer * mailer);
void compose_delete(Compose * compose);

/* useful */
void compose_save(Compose * compose);
void compose_send(Compose * compose);

#endif /* !MAILER_COMPOSE_H */
