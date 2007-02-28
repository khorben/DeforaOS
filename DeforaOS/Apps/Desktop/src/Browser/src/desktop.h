/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#ifndef BROWSER_DESKTOP_H
# define BROWSER_DESKTOP_H

# include <gtk/gtk.h>


/* DesktopIcon */
/* types */
typedef struct _DesktopIcon DesktopIcon;
typedef struct _Desktop Desktop; /* XXX forward declaration */

/* functions */
DesktopIcon * desktopicon_new(struct _Desktop * desktop, char const * name,
		char const * url);
void desktopicon_delete(DesktopIcon * desktopicon);

/* accessors */
void desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon);

/* useful */
void desktopicon_move(DesktopIcon * desktopicon, int x, int y);
void desktopicon_show(DesktopIcon * desktopicon);


/* Desktop */
/* functions */
Desktop * desktop_new(void);
void desktop_delete(Desktop * desktop);

/* useful */
int desktop_error(Desktop * desktop, char const * message, int ret);

void desktop_refresh(Desktop * desktop);

void desktop_icon_add(Desktop * desktop, DesktopIcon * icon);
void desktop_icon_remove(Desktop * desktop, DesktopIcon * icon);

void desktop_icons_align(Desktop * desktop);
void desktop_icons_sort(Desktop * desktop);

#endif /* !BROWSER_DESKTOP_H */
