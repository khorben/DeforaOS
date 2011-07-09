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



#ifndef MAILER_SRC_MESSAGE_H
# define MAILER_SRC_MESSAGE_H

# include <gtk/gtk.h>
# include "Mailer.h"


/* Message */
/* types */


/* functions */
Message * message_new(AccountMessage * message, GtkListStore * store,
		GtkTreeIter * iter);
void message_delete(Message * message);

/* accessors */
GtkTextBuffer * message_get_body(Message * message);
AccountMessage * message_get_data(Message * message);
char const * message_get_header(Message * message, char const * header);
gboolean message_get_iter(Message * message, GtkTreeIter * iter);
GtkListStore * message_get_store(Message * message);

int message_set_body(Message * message, char const * buf, size_t cnt,
		gboolean append);
int message_set_header(Message * message, char const * header);
int message_set_header_value(Message * message, char const * header,
		char const * value);
void message_set_read(Message * message, gboolean read);

#endif /* !MAILER_SRC_MAILER_H */
