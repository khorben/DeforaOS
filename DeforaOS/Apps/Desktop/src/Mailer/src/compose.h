/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#ifndef MAILER_COMPOSE_H
# define MAILER_COMPOSE_H

# include <sys/types.h>
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

/* accessors */
Mailer * compose_get_mailer(Compose * compose); /* XXX ugly */

/* useful */
void compose_save(Compose * compose);
void compose_send(Compose * compose);

#endif /* !MAILER_COMPOSE_H */
