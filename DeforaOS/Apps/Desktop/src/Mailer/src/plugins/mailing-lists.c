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



#include <stdlib.h>
#include "Mailer/plugin.h"


/* Mailing-lists */
/* private */
/* types */
typedef struct _MailingLists
{
	GtkWidget * vbox;
	GtkWidget * folder;
	GtkWidget * message;
	GtkWidget * name;
} MailingLists;


/* protected */
/* prototypes */
/* plug-in */
static GtkWidget * _ml_init(MailerPlugin * plugin);
static void _ml_destroy(MailerPlugin * plugin);
static void _ml_refresh(MailerPlugin * plugin, Folder * folder,
		Message * message);


/* public */
/* variables */
/* plug-in */
MailerPlugin plugin =
{
	NULL,
	"Mailing-lists",
	NULL,
	_ml_init,
	_ml_destroy,
	_ml_refresh,
	NULL
};


/* protected */
/* functions */
/* plug-in */
/* ml_init */
static GtkWidget * _ml_init(MailerPlugin * plugin)
{
	MailingLists * ml;

	if((ml = malloc(sizeof(*ml))) == NULL)
		return NULL;
	plugin->priv = ml;
	ml->vbox = gtk_vbox_new(FALSE, 4);
	ml->folder = gtk_label_new("");
	/* FIXME set a bold font */
	gtk_misc_set_alignment(GTK_MISC(ml->folder), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->folder, FALSE, TRUE, 0);
	ml->message = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ml->message), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->message, FALSE, TRUE, 0);
	ml->name = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ml->name), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->name, FALSE, TRUE, 0);
	return ml->vbox;
}


/* ml_destroy */
static void _ml_destroy(MailerPlugin * plugin)
{
	MailingLists * ml = plugin->priv;

	free(ml);
}


/* ml_refresh */
static void _ml_refresh(MailerPlugin * plugin, Folder * folder,
		Message * message)
{
	MailingLists * ml = plugin->priv;
	char const * id;

	if(folder == NULL)
	{
		gtk_widget_hide(ml->folder);
		gtk_widget_hide(ml->message);
		gtk_widget_hide(ml->name);
		return;
	}
	gtk_label_set_text(GTK_LABEL(ml->folder), folder_get_name(folder));
	gtk_widget_show(ml->folder);
	if(message == NULL)
	{
		gtk_widget_hide(ml->message);
		gtk_widget_hide(ml->name);
		return;
	}
	if((id = message_get_header(message, "List-Id")) == NULL)
	{
		gtk_label_set_text(GTK_LABEL(ml->message),
				"Not a mailing-list");
		gtk_widget_show(ml->message);
		gtk_widget_hide(ml->name);
		return;
	}
	/* XXX parse and beautify the list's name */
	gtk_label_set_text(GTK_LABEL(ml->name), id);
	gtk_widget_show(ml->name);
}
