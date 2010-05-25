/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#ifndef BROWSER_DESKTOPICON_H
# define BROWSER_DESKTOPICON_H

# include <gtk/gtk.h>
# include "desktop.h"


/* DesktopIcon */
/* types */
# ifndef Desktop
#  define Desktop Desktop
typedef struct _Desktop Desktop;
# endif
# ifndef DesktopIcon
#  define DesktopIcon DesktopIcon
typedef struct _DesktopIcon DesktopIcon;
# endif
typedef void (*DesktopIconCallback)(Desktop * desktop, gpointer data);


/* constants */
#define DESKTOPICON_ICON_SIZE	48
#define DESKTOPICON_MAX_HEIGHT	100
#define DESKTOPICON_MAX_WIDTH	100
#define DESKTOPICON_MIN_HEIGHT	(DESKTOPICON_ICON_SIZE << 1)
#define DESKTOPICON_MIN_WIDTH	DESKTOPICON_MAX_WIDTH


/* functions */
DesktopIcon * desktopicon_new(Desktop * desktop, char const * name,
		char const * url);
DesktopIcon * desktopicon_new_application(Desktop * desktop, char const * path);
DesktopIcon * desktopicon_new_category(Desktop * desktop, char const * name,
		char const * icon);
void desktopicon_delete(DesktopIcon * desktopicon);

/* accessors */
gboolean desktopicon_get_first(DesktopIcon * desktopicon);
gboolean desktopicon_get_immutable(DesktopIcon * desktopicon);
char const * desktopicon_get_name(DesktopIcon * desktopicon);
char const * desktopicon_get_path(DesktopIcon * desktopicon);
gboolean desktopicon_get_selected(DesktopIcon * desktopicon);
gboolean desktopicon_get_updated(DesktopIcon * desktopicon);

void desktopicon_set_callback(DesktopIcon * desktopicon,
		DesktopIconCallback callback, gpointer data);
void desktopicon_set_confirm(DesktopIcon * desktopicon, gboolean confirm);
void desktopicon_set_executable(DesktopIcon * desktopicon, gboolean executable);
void desktopicon_set_first(DesktopIcon * desktopicon, gboolean first);
void desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon);
void desktopicon_set_immutable(DesktopIcon * desktopicon, gboolean immutable);
void desktopicon_set_selected(DesktopIcon * desktopicon, gboolean selected);
void desktopicon_set_updated(DesktopIcon * desktopicon, gboolean updated);

/* useful */
void desktopicon_move(DesktopIcon * desktopicon, int x, int y);
void desktopicon_show(DesktopIcon * desktopicon);

#endif /* !BROWSER_DESKTOPICON_H */
