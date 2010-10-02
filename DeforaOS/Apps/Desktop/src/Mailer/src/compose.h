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
typedef struct _Compose Compose;

/* methods */
Compose * compose_new(Mailer * mailer);
void compose_delete(Compose * compose);

/* accessors */
Mailer * compose_get_mailer(Compose * compose);

/* useful */
int compose_save(Compose * compose);
void compose_send(Compose * compose);
void compose_send_cancel(Compose * compose);

void compose_show_about(Compose * compose, gboolean show);

void compose_toggle_show_bcc(Compose * compose);
void compose_toggle_show_cc(Compose * compose);

#endif /* !MAILER_COMPOSE_H */
