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
#include "window.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* PanelWindow */
/* private */
/* types */
struct _PanelWindow
{
	PanelAppletHelper * helper;

	PanelPosition position;
	gint height;
	GdkRectangle root;

	GtkWidget * window;
	GtkWidget * hbox;
};


/* prototypes */
static void _panel_window_reset(PanelWindow * panel);
static void _panel_window_reset_strut(PanelWindow * panel);


/* public */
/* functions */
/* panel_window_new */
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
	gtk_window_set_accept_focus(GTK_WINDOW(panel->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(panel->window), FALSE);
#endif
	gtk_window_set_keep_above(GTK_WINDOW(panel->window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(panel->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_stick(GTK_WINDOW(panel->window));
	g_signal_connect(G_OBJECT(panel->window), "delete-event", G_CALLBACK(
				_on_closex), panel);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(panel->window), panel->hbox);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_widget_show_all(panel->hbox);
	panel_window_reset(panel, position, root);
	panel_window_show(panel, TRUE);
	return panel;
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
/* panel_window_get_height */
int panel_window_get_height(PanelWindow * panel)
{
	return panel->height;
}


/* useful */
/* panel_window_append */
void panel_window_append(PanelWindow * panel, GtkWidget * widget,
		gboolean expand, gboolean fill)
{
	gtk_box_pack_start(GTK_BOX(panel->hbox), widget, expand, fill, 0);
}


/* panel_window_reset */
void panel_window_reset(PanelWindow * panel, PanelPosition position,
		GdkRectangle * root)
{
	panel->position = position;
	memcpy(&panel->root, root, sizeof(*root));
	_panel_window_reset(panel);
	_panel_window_reset_strut(panel);
}


/* panel_window_show */
void panel_window_show(PanelWindow * panel, gboolean show)
{
	if(show)
	{
		_panel_window_reset(panel);
		gtk_widget_show(panel->window);
		_panel_window_reset_strut(panel);
	}
	else
		gtk_widget_hide(panel->window);
}


/* private */
/* functions */
/* panel_window_reset */
static void _panel_window_reset(PanelWindow * panel)
{
	gtk_window_resize(GTK_WINDOW(panel->window), panel->root.width,
			panel->height);
	if(panel->position == PANEL_POSITION_TOP)
		gtk_window_move(GTK_WINDOW(panel->window), panel->root.x, 0);
	else
		gtk_window_move(GTK_WINDOW(panel->window), panel->root.x,
				panel->root.y + panel->root.height
				- panel->height);
}


/* panel_window_reset_strut */
static void _panel_window_reset_strut(PanelWindow * panel)
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
	switch(panel->position)
	{
		case PANEL_POSITION_TOP:
			strut[2] = panel->height;
			strut[8] = panel->root.x;
			strut[9] = panel->root.x + panel->root.width;
			break;
		case PANEL_POSITION_BOTTOM:
			strut[3] = panel->height;
			strut[10] = panel->root.x;
			strut[11] = panel->root.x + panel->root.width;
			break;
	}
	atom = gdk_atom_intern("_NET_WM_STRUT", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 4);
	atom = gdk_atom_intern("_NET_WM_STRUT_PARTIAL", FALSE);
	gdk_property_change(window, atom, cardinal, 32, GDK_PROP_MODE_REPLACE,
			(guchar*)strut, 12);
}
