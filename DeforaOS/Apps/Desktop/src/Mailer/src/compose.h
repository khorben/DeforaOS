/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
# include <glib.h>
# include <System.h>


/* types */
typedef struct _Compose Compose;

/* methods */
Compose * compose_new(Config * config);
Compose * compose_new_copy(Compose * compose);
void compose_delete(Compose * compose);

/* accessors */
void compose_set_font(Compose * compose, char const * font);
void compose_set_standalone(Compose * compose, gboolean standalone);
void compose_set_subject(Compose * compose, char const * subject);

/* useful */
void compose_add_field(Compose * compose, char const * field,
		char const * value);

void compose_attach_dialog(Compose * compose);

void compose_copy(Compose * compose);
void compose_cut(Compose * compose);
void compose_paste(Compose * compose);

int compose_error(Compose * compose, char const * message, int ret);

int compose_save(Compose * compose);

void compose_select_all(Compose * compose);

void compose_send(Compose * compose);
void compose_send_cancel(Compose * compose);

void compose_show_about(Compose * compose, gboolean show);

#endif /* !MAILER_COMPOSE_H */
