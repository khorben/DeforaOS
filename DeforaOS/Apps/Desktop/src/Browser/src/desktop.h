/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#ifndef BROWSER_DESKTOP_H
# define BROWSER_DESKTOP_H

# include <gtk/gtk.h>
# include "mime.h"
# include "desktopicon.h"


/* Desktop */
/* types */
# ifndef DesktopIcon
#  define DesktopIcon DesktopIcon
typedef struct _DesktopIcon DesktopIcon;
# endif
# ifndef Desktop
#  define Desktop Desktop
typedef struct _Desktop Desktop;
# endif


/* functions */
Desktop * desktop_new(void);
void desktop_delete(Desktop * desktop);

/* accessors */
/* XXX most of these accessors expose internal structures somehow */
int desktop_get_drag_data(Desktop * desktop, GtkSelectionData * seldata);
GdkPixbuf * desktop_get_file(Desktop * desktop);
GdkPixbuf * desktop_get_folder(Desktop * desktop);
Mime * desktop_get_mime(Desktop * desktop);
GtkIconTheme * desktop_get_theme(Desktop * desktop);

/* useful */
int desktop_error(Desktop * desktop, char const * message, int ret);

void desktop_refresh(Desktop * desktop);

void desktop_icon_add(Desktop * desktop, DesktopIcon * icon);
void desktop_icon_remove(Desktop * desktop, DesktopIcon * icon);

void desktop_icons_align(Desktop * desktop);
void desktop_icons_sort(Desktop * desktop);

void desktop_select_all(Desktop * desktop);
void desktop_select_above(Desktop * desktop, DesktopIcon * icon);
void desktop_select_under(Desktop * desktop, DesktopIcon * icon);
void desktop_unselect_all(Desktop * desktop);

#endif /* !BROWSER_DESKTOP_H */
