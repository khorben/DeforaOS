/* $Id$ */
/* Copyright (c) 2009-2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#ifndef LIBDESKTOP_DESKTOP_TOOLBAR_H
# define LIBDESKTOP_DESKTOP_TOOLBAR_H


/* Toolbar */
/* types */
typedef struct _DesktopToolbar
{
	const char * name;
	GCallback callback;
	const char * stock;
	GdkModifierType modifier;
	unsigned int accel;
	GtkToolItem * widget;
} DesktopToolbar;


/* functions */
GtkWidget * desktop_toolbar_create(DesktopToolbar * toolbar,
		gpointer data, GtkAccelGroup * accel);

#endif /* !LIBDESKTOP_DESKTOP_TOOLBAR_H */
