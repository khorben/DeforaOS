/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#ifndef LIBDESKTOP_MENUBAR_H
# define LIBDESKTOP_MENUBAR_H


/* Menubar */
/* types */
typedef struct _DesktopMenu
{
	const char * name;
	GCallback callback;
	const char * stock;
	GdkModifierType modifier;
	unsigned int accel;
} DesktopMenu;

typedef struct _DesktopMenubar
{
	const char * name;
	DesktopMenu * menu;
} DesktopMenubar;


/* functions */
GtkWidget * desktop_menubar_create(DesktopMenubar * menubar, gpointer data,
		GtkAccelGroup * accel);

#endif /* !LIBDESKTOP_MENUBAR_H */
