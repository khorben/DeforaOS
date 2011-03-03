/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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



#ifndef BROWSER_MIME_H
# define BROWSER_MIME_H

# include <System.h>
# include <gtk/gtk.h>

# define MIME_CONFIG_FILE ".mime"


/* Mime */
/* types */
typedef struct _Mime Mime;


/* functions */
Mime * mime_new(void);
void mime_delete(Mime * mime);

/* accessors */
char const * mime_get_handler(Mime * mime, char const * type,
		char const * action);
int mime_set_handler(Mime * mime, char const * type, char const * action,
		char const * handler);

/* useful */
char const * mime_type(Mime * mime, char const * path);
/* FIXME return an enum with error codes? */
int mime_action(Mime * mime, char const * action, char const * path);
int mime_action_type(Mime * mime, char const * action, char const * path,
		char const * type);
void mime_icons(Mime * mime, GtkIconTheme * theme, char const * type, ...);

#endif /* !BROWSER_MIME_H */
