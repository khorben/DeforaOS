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



#ifndef DESKTOP_MAILER_PLUGIN_H
# define DESKTOP_MAILER_PLUGIN_H

# include <gtk/gtk.h>
# include "mailer.h"


/* MailerPlugin */
/* types */
typedef struct _MailerPluginHelper
{
	Mailer * mailer;
	int (*error)(Mailer * mailer, char const * message, int ret);
} MailerPluginHelper;

typedef struct _MailerPlugin MailerPlugin;

struct _MailerPlugin
{
	MailerPluginHelper * helper;
	char const * name;
	char const * icon;
	int (*init)(MailerPlugin * plugin);
	void (*destroy)(MailerPlugin * plugin);
	void (*set_folder)(MailerPlugin * plugin, GtkListStore * store);
	void * priv;
};

#endif /* !DESKTOP_MAILER_PLUGIN_H */
