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



#include <stdlib.h>
#include "Mailer/folder.h"
#include "Mailer/message.h"
#include "Mailer/plugin.h"


/* Mailing-lists */
/* private */
/* types */
typedef struct _MailerPlugin MailingLists;

struct _MailerPlugin
{
	MailerPluginHelper * helper;

	/* widgets */
	GtkWidget * vbox;
	GtkWidget * folder;
	GtkWidget * message;
	GtkWidget * name;
};


/* protected */
/* prototypes */
/* plug-in */
static MailerPlugin * _ml_init(MailerPluginHelper * helper);
static void _ml_destroy(MailingLists * ml);
static GtkWidget * _ml_get_widget(MailingLists * ml);
static void _ml_refresh(MailingLists * ml, MailerFolder * folder,
		MailerMessage * message);


/* public */
/* variables */
/* plug-in */
MailerPluginDefinition plugin =
{
	"Mailing-lists",
	NULL,
	"Mailing-lists management",
	_ml_init,
	_ml_destroy,
	_ml_get_widget,
	_ml_refresh
};


/* protected */
/* functions */
/* plug-in */
/* ml_init */
static MailerPlugin * _ml_init(MailerPluginHelper * helper)
{
	MailingLists * ml;
	PangoFontDescription * bold;

	if((ml = malloc(sizeof(*ml))) == NULL)
		return NULL;
	ml->helper = helper;
	/* widgets */
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	ml->vbox = gtk_vbox_new(FALSE, 4);
	ml->folder = gtk_label_new("");
	gtk_widget_modify_font(ml->folder, bold);
	gtk_misc_set_alignment(GTK_MISC(ml->folder), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->folder, FALSE, TRUE, 0);
	ml->message = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ml->message), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->message, FALSE, TRUE, 0);
	ml->name = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ml->name), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(ml->vbox), ml->name, FALSE, TRUE, 0);
	pango_font_description_free(bold);
	return ml;
}


/* ml_destroy */
static void _ml_destroy(MailingLists * ml)
{
	free(ml);
}


/* ml_get_widget */
static GtkWidget * _ml_get_widget(MailingLists * ml)
{
	return ml->vbox;
}


/* ml_refresh */
static void _ml_refresh(MailingLists * ml, MailerFolder * folder,
		MailerMessage * message)
{
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
	gtk_widget_hide(ml->message);
	gtk_label_set_text(GTK_LABEL(ml->name), id);
	gtk_widget_show(ml->name);
}
