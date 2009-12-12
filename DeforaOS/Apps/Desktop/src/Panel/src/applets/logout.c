/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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
#include "Panel.h"


/* Logout */
/* private */
/* prototypes */
static GtkWidget * _logout_init(PanelApplet * applet);

/* callbacks */
static void _on_clicked(gpointer data);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_logout_init,
	NULL,
	PANEL_APPLET_POSITION_START,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* logout_init */
static GtkWidget * _logout_init(PanelApplet * applet)
{
	GtkWidget * ret;
	GtkWidget * image;

	if(applet->helper->logout_dialog == NULL)
	{
		error_set_code(0, "%s", "logout: Logging out is disabled");
		return NULL;
	}
	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("gnome-logout",
			applet->helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, "Logout");
#endif
	g_signal_connect_swapped(G_OBJECT(ret), "clicked", G_CALLBACK(
				_on_clicked), applet);
	return ret;
}


/* callbacks */
/* on_clicked */
static void _on_clicked(gpointer data)
{
	PanelApplet * applet = data;

	applet->helper->logout_dialog();
}
