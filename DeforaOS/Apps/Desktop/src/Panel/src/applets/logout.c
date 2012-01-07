/* $Id$ */
/* Copyright (c) 2010-2012 Pierre Pronchery <khorben@defora.org> */
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



#include <stdlib.h>
#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Logout */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
} Logout;


/* prototypes */
static Logout * _logout_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _logout_destroy(Logout * logout);

/* callbacks */
static void _on_clicked(gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Logout",
	"gnome-logout",
	NULL,
	_logout_init,
	_logout_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* logout_init */
static Logout * _logout_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
	Logout * logout;
	GtkWidget * ret;
	GtkWidget * image;

	if((logout = malloc(sizeof(*logout))) == NULL)
		return NULL;
	logout->helper = helper;
	if(helper->logout_dialog == NULL)
	{
		helper->error(NULL, _("logout: Logging out is disabled"), 1);
		return NULL;
	}
	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("gnome-logout", helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Logout"));
#endif
	g_signal_connect_swapped(G_OBJECT(ret), "clicked", G_CALLBACK(
				_on_clicked), logout);
	gtk_widget_show_all(ret);
	*widget = ret;
	return logout;
}


/* logout_destroy */
static void _logout_destroy(Logout * logout)
{
	free(logout);
}


/* callbacks */
/* on_clicked */
static void _on_clicked(gpointer data)
{
	Logout * logout = data;

	logout->helper->logout_dialog(logout->helper->panel);
}
