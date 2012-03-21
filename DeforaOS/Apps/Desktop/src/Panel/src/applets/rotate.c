/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Rotate Panel */
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



#include <libintl.h>
#include <System.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Rotate */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;
} Rotate;


/* prototypes */
static Rotate * _rotate_init(PanelAppletHelper * helper, GtkWidget ** widget);
static void _rotate_destroy(Rotate * rotate);

/* callbacks */
static void _on_clicked(gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Rotate",
	GTK_STOCK_REFRESH, /* XXX use a more adequate image */
	NULL,
	_rotate_init,
	_rotate_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* rotate_init */
static Rotate * _rotate_init(PanelAppletHelper * helper, GtkWidget ** widget)
{
	Rotate * rotate;
	GtkWidget * image;

	if((rotate = object_new(sizeof(*rotate))) == NULL)
		return NULL;
	rotate->helper = helper;
	*widget = gtk_button_new();
	image = gtk_image_new_from_stock(applet.icon, helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(*widget), image);
	gtk_button_set_relief(GTK_BUTTON(*widget), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(*widget, _("Rotate the screen"));
#endif
	g_signal_connect_swapped(G_OBJECT(*widget), "clicked", G_CALLBACK(
				_on_clicked), rotate);
	gtk_widget_show_all(*widget);
	return rotate;
}


/* rotate_destroy */
static void _rotate_destroy(Rotate * rotate)
{
	object_delete(rotate);
}


/* callbacks */
/* on_clicked */
static void _on_clicked(gpointer data)
{
	Rotate * rotate = data;

	rotate->helper->rotate_screen(rotate->helper->panel);
}
