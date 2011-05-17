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



#ifndef DESKTOP_DESKTOP_MIME_H
# define DESKTOP_DESKTOP_MIME_H

# include <gtk/gtk.h>


/* Mime */
/* types */
typedef struct _Mime Mime;

typedef void (*MimeForeachCallback)(void * data, char const * name,
		GdkPixbuf * icon_24, GdkPixbuf * icon_48, GdkPixbuf * icon_96);


/* functions */
Mime * mime_new(GtkIconTheme * theme);
void mime_delete(Mime * mime);

/* accessors */
char const * mime_get_handler(Mime * mime, char const * type,
		char const * action);
int mime_set_handler(Mime * mime, char const * type, char const * action,
		char const * handler);
void mime_set_theme(Mime * mime, GtkIconTheme * theme);

/* useful */
char const * mime_type(Mime * mime, char const * path);
/* FIXME return an enum with error codes? */
int mime_action(Mime * mime, char const * action, char const * path);
int mime_action_type(Mime * mime, char const * action, char const * path,
		char const * type);

void mime_foreach(Mime * mime, MimeForeachCallback callback, void * data);

void mime_icons(Mime * mime, char const * type, ...);

int mime_save(Mime * mime);

#endif /* !DESKTOP_DESKTOP_MIME_H */
