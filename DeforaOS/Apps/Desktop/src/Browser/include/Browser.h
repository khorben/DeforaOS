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



#ifndef DESKTOP_BROWSER_H
# define DESKTOP_BROWSER_H

# include <gtk/gtk.h>


/* Browser */
/* public */
/* types */
typedef struct _Browser Browser;

typedef struct _BrowserPlugin BrowserPlugin;

typedef struct _BrowserPluginHelper
{
	Browser * browser;
} BrowserPluginHelper;

struct _BrowserPlugin
{
	BrowserPluginHelper * helper;
	char const * name;
	GtkWidget * (*init)(BrowserPlugin * plugin);
	void (*destroy)(BrowserPlugin * plugin);
	void * priv;
};

#endif /* !DESKTOP_BROWSER_H */
