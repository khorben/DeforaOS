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



#ifndef MAILER_COMMON_H
# define MAILER_COMMON_H

# include <gtk/gtk.h>


/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
	unsigned int accel;
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};

struct _toolbar
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
};


/* functions */
GtkWidget * common_new_menubar(GtkWindow * window, struct _menubar * mb,
		gpointer data);
GtkWidget * common_new_toolbar(struct _toolbar * tb, gpointer data);

#endif /* !MAILER_COMMON_H */
