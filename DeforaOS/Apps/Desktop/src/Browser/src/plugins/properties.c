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



#include <System.h>
#include "Browser.h"


/* Properties */
/* private */
/* types */
typedef struct _Properties
{
	GtkWidget * view;
} Properties;


/* prototypes */
static GtkWidget * _properties_init(BrowserPlugin * plugin);
static void _properties_destroy(BrowserPlugin * plugin);


/* public */
/* variables */
BrowserPlugin plugin =
{
	NULL,
	"Properties",
	_properties_init,
	_properties_destroy,
	NULL
};


/* private */
/* functions */
/* properties_init */
static GtkWidget * _properties_init(BrowserPlugin * plugin)
{
	Properties * properties;
	GtkWidget * widget;

	if((properties = object_new(sizeof(*properties))) == NULL)
		return NULL;
	plugin->priv = properties;
	properties->view = gtk_vbox_new(FALSE, 4);
	/* FIXME really implement */
	widget = gtk_label_new("Folder");
	gtk_box_pack_start(GTK_BOX(properties->view), widget, FALSE, TRUE, 0);
	gtk_widget_show_all(properties->view);
	return properties->view;
}


/* properties_destroy */
static void _properties_destroy(BrowserPlugin * plugin)
{
	Properties * properties = plugin->priv;

	gtk_widget_destroy(properties->view);
	object_delete(properties);
}
