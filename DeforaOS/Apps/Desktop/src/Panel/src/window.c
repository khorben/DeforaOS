/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "common.h"
#include "window.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* PanelWindow */
/* private */
/* types */
struct _PanelWindow
{
	PanelAppletHelper * helper;
	gint height;

	GtkWidget * window;
	GtkWidget * hbox;
};


/* public */
/* functions */
/* panel_window_new */
static void _new_strut(PanelWindow * panel, PanelPosition position,
		GdkRectangle * root);
static gboolean _on_closex(void);

PanelWindow * panel_window_new(PanelPosition position,
		PanelAppletHelper * helper, GdkRectangle * root)
{
	PanelWindow * panel;
	int icon_width;
	int icon_height;

	if(gtk_icon_size_lookup(helper->icon_size, &icon_width, &icon_height)
			!= TRUE)
	{
		error_set_code(1, _("Invalid panel size"));
		return NULL;
	}
	if((panel = object_new(sizeof(*panel))) == NULL)
		return NULL;
	panel->helper = helper;
	panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	panel->height = icon_height + (PANEL_BORDER_WIDTH * 4);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() height=%d\n", __func__, panel->height);
#endif
	gtk_window_resize(GTK_WINDOW(panel->window), root->width,
			panel->height);
	gtk_window_set_accept_focus(GTK_WINDOW(panel->window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(panel->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	if(position == PANEL_POSITION_TOP)
		gtk_window_move(GTK_WINDOW(panel->window), root->x, 0);
	else
		gtk_window_move(GTK_WINDOW(panel->window), root->x,
				root->y + root->height - panel->height);
	gtk_window_stick(GTK_WINDOW(panel->window));
	g_signal_connect(G_OBJECT(panel->window), "delete-event", G_CALLBACK(
				_on_closex), panel);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(panel->window), panel->hbox);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_widget_show_all(panel->window);
	_new_strut(panel, position, root);
	return panel;
}

static void _new_strut(PanelWindow * panel, PanelPosition position,
		GdkRectangle * root)
{
	GdkWindow * window;
	GdkAtom atom;
	GdkAtom cardinal;
	unsigned long strut[12];

#if GTK_CHECK_VERSION(2, 14, 0)
	window = gtk_widget_get_window(panel->window);
#else
	window = panel->window->window;
#endif
	cardinal = gdk_atom_intern("CARDINAL", FALSE);
	memset(strut, 0, sizeof(strut));
	switch(position)
	{
		case PANEL_POSITION_TOP:
			strut[2] = panel->height;
			strut[8] = root->x;
			strut[9] = root->x + root->width;
			break;
		case PANEL_POSITION_BOTTOM:
			strut[3] = panel->height;
			strut[10] = root->x;
			strut[11] = root->x + root->width;
			break;
	}
	atom = gdk_atom_intern("_NET_WM_STRUT", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 4);
	atom = gdk_atom_intern("_NET_WM_STRUT_PARTIAL", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 12);
}

static gboolean _on_closex(void)
{
	/* ignore delete events */
	return TRUE;
}


/* panel_window_delete */
void panel_window_delete(PanelWindow * panel)
{
	gtk_widget_destroy(panel->window);
	object_delete(panel);
}


/* accessors */
int panel_window_get_height(PanelWindow * panel)
{
	return panel->height;
}


/* useful */
void panel_window_append(PanelWindow * panel, GtkWidget * widget,
		gboolean expand, gboolean fill, PanelAppletPosition position)
{
	switch(position)
	{
		case PANEL_APPLET_POSITION_END:
			gtk_box_pack_end(GTK_BOX(panel->hbox), widget, expand,
					fill, 0);
			break;
		case PANEL_APPLET_POSITION_FIRST:
			gtk_box_pack_start(GTK_BOX(panel->hbox), widget, expand,
					fill, 0);
			gtk_box_reorder_child(GTK_BOX(panel->hbox), widget, 0);
			break;
		case PANEL_APPLET_POSITION_LAST:
			gtk_box_pack_end(GTK_BOX(panel->hbox), widget, expand,
					fill, 0);
			gtk_box_reorder_child(GTK_BOX(panel->hbox), widget, 0);
			break;
		case PANEL_APPLET_POSITION_START:
			gtk_box_pack_start(GTK_BOX(panel->hbox), widget, expand,
					fill, 0);
			break;
	}
}
