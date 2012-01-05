/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
# include "folder.h"
# include "message.h"
# include "mailer.h"


/* MailerPlugin */
/* types */
typedef struct _MailerPluginHelper
{
	Mailer * mailer;
	int (*error)(Mailer * mailer, char const * message, int ret);
} MailerPluginHelper;

typedef struct _MailerPlugin MailerPlugin;

typedef const struct _MailerPluginDefinition
{
	char const * name;
	char const * icon;
	char const * description;
	MailerPlugin * (*init)(MailerPluginHelper * helper);
	void (*destroy)(MailerPlugin * plugin);
	GtkWidget * (*get_widget)(MailerPlugin * plugin);
	void (*refresh)(MailerPlugin * plugin, Folder * folder,
			Message * message);
} MailerPluginDefinition;


/* functions */
/* XXX should be in Mailer/folder.h */
GtkListStore * folder_get_messages(Folder * folder);

#endif /* !DESKTOP_MAILER_PLUGIN_H */
