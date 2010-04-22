/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <string.h>
#include <libintl.h>
#include <gdk/gdkx.h>
#include <X11/X.h>
#include "Panel.h"
#define _(string) gettext(string)


/* Desktop */
/* private */
/* prototypes */
static GtkWidget * _desktop_init(PanelApplet * applet);

/* callbacks */
static void _on_clicked(GtkWidget * widget);


/* public */
/* variables */
PanelApplet applet =
{
	NULL,
	_desktop_init,
	NULL,
	PANEL_APPLET_POSITION_FIRST,
	FALSE,
	TRUE,
	NULL
};


/* private */
/* functions */
/* desktop_init */
static GtkWidget * _desktop_init(PanelApplet * applet)
{
	GtkWidget * ret;
	GtkWidget * image;

	ret = gtk_button_new();
	image = gtk_image_new_from_icon_name("user-desktop",
			applet->helper->icon_size);
	gtk_button_set_image(GTK_BUTTON(ret), image);
	gtk_button_set_relief(GTK_BUTTON(ret), GTK_RELIEF_NONE);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(ret, _("Show desktop"));
#endif
	g_signal_connect(G_OBJECT(ret), "clicked", G_CALLBACK(_on_clicked),
			NULL);
	gtk_widget_show_all(ret);
	return ret;
}


/* callbacks */
/* on_clicked */
static void _on_clicked(GtkWidget * widget)
{
	GdkScreen * screen;
	GdkDisplay * display;
	GdkWindow * root;
	XEvent xev;

	screen = gtk_widget_get_screen(widget);
	display = gtk_widget_get_display(widget);
	root = gdk_screen_get_root_window(screen);
	xev.xclient.type = ClientMessage;
	xev.xclient.window = GDK_WINDOW_XWINDOW(root);
	xev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(
			display, "_NET_SHOWING_DESKTOP");
	xev.xclient.format = 32;
	memset(&xev.xclient.data, sizeof(xev.xclient.data), 0);
	xev.xclient.data.l[0] = 1;
	gdk_error_trap_push();
	XSendEvent(GDK_DISPLAY_XDISPLAY(display), GDK_WINDOW_XWINDOW(root),
			False,
			SubstructureNotifyMask | SubstructureRedirectMask,
			&xev);
	gdk_error_trap_pop();
}
