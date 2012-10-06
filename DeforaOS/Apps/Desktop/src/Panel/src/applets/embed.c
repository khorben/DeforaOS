/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - let automatic show/hide be in preferences */



#ifdef DEBUG
# include <stdio.h>
#endif
#include <System.h>
#include <Desktop.h>
#include "Panel.h"


/* Embed */
/* private */
/* types */
typedef struct _PanelApplet
{
	PanelAppletHelper * helper;

	guint source;
	guint count;

	/* widgets */
	GtkWidget * button;
	GtkWidget * window;
	GtkWidget * vbox;
} Embed;


/* prototypes */
static Embed * _embed_init(PanelAppletHelper * helper,
		GtkWidget ** widget);
static void _embed_destroy(Embed * embed);

/* callbacks */
static void _embed_on_added(gpointer data);
static int _embed_on_desktop_message(void * data, uint32_t value1,
		uint32_t value2, uint32_t value3);
static int _embed_on_idle(gpointer data);
static gboolean _embed_on_removed(GtkWidget * widget, gpointer data);
static void _embed_on_toggled(gpointer data);


/* public */
/* variables */
PanelAppletDefinition applet =
{
	"Embed",
	"gnome-window-manager",
	NULL,
	_embed_init,
	_embed_destroy,
	NULL,
	FALSE,
	TRUE
};


/* private */
/* functions */
/* embed_init */
static Embed * _embed_init(PanelAppletHelper * helper,
		GtkWidget ** widget)
{
	Embed * embed;
	GtkWidget * image;

	if((embed = object_new(sizeof(*embed))) == NULL)
	{
		helper->error(NULL, error_get(), 1);
		return NULL;
	}
	embed->helper = helper;
	embed->source = 0;
	embed->count = 0;
	embed->window = NULL;
	embed->vbox = NULL;
	embed->button = gtk_toggle_button_new();
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(embed->button, "Show embedded widgets");
#endif
	gtk_button_set_relief(GTK_BUTTON(embed->button), GTK_RELIEF_NONE);
	gtk_widget_set_sensitive(embed->button, FALSE);
	g_signal_connect_swapped(G_OBJECT(embed->button), "toggled", G_CALLBACK(
				_embed_on_toggled), embed);
	image = gtk_image_new_from_icon_name(applet.icon, helper->icon_size);
	gtk_container_add(GTK_CONTAINER(embed->button), image);
	gtk_widget_show_all(embed->button);
	*widget = embed->button;
	g_idle_add(_embed_on_idle, embed);
	return embed;
}


/* embed_destroy */
static void _embed_destroy(Embed * embed)
{
	if(embed->source != 0)
		g_source_remove(embed->source);
	object_delete(embed);
}


/* callbacks */
/* embed_on_added */
static void _embed_on_added(gpointer data)
{
	Embed * embed = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(embed->button), TRUE);
	gtk_widget_set_sensitive(embed->button, TRUE);
	embed->count++;
}


/* embed_on_desktop_message */
static int _embed_on_desktop_message(void * data, uint32_t value1,
		uint32_t value2, uint32_t value3)
{
	Embed * embed = data;
	GtkWidget * socket;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, %u, %u)\n", __func__, value1, value2,
			value3);
#endif
	if(value1 != PANEL_MESSAGE_EMBED)
		return 0;
	socket = gtk_socket_new();
	g_signal_connect_swapped(socket, "plug-added", G_CALLBACK(
				_embed_on_added), embed);
	g_signal_connect(socket, "plug-removed", G_CALLBACK(
				_embed_on_removed), embed);
	gtk_widget_show(socket);
	gtk_box_pack_start(GTK_BOX(embed->vbox), socket, FALSE, TRUE, 0);
	gtk_socket_add_id(GTK_SOCKET(socket), value2);
	return 0;
}


/* embed_on_idle */
static int _embed_on_idle(gpointer data)
{
	Embed * embed = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	embed->source = 0;
	embed->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_accept_focus(GTK_WINDOW(embed->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(embed->window), FALSE);
#endif
	gtk_window_set_type_hint(GTK_WINDOW(embed->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	embed->vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(embed->window), embed->vbox);
	gtk_widget_show(embed->vbox);
	desktop_message_register(PANEL_CLIENT_MESSAGE,
			_embed_on_desktop_message, embed);
	return FALSE;
}


/* embed_on_removed */
static gboolean _embed_on_removed(GtkWidget * widget, gpointer data)
{
	Embed * embed = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(embed->count > 0 && --embed->count == 0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(embed->button),
				FALSE);
		gtk_widget_set_sensitive(embed->button, FALSE);
	}
	return FALSE;
}


/* embed_on_toggled */
static void _embed_on_toggled(gpointer data)
{
	Embed * embed = data;
	PanelAppletHelper * helper = embed->helper;
	gint x = 0;
	gint y = 0;
	gboolean push_in;
#if GTK_CHECK_VERSION(2, 18, 0)
	GtkAllocation allocation;
#endif

	if(embed->window == NULL)
		_embed_on_idle(embed);
	if(embed->window == NULL)
		return;
	helper->position_menu(helper->panel, (GtkMenu *)embed->window, &x, &y,
			&push_in);
#if GTK_CHECK_VERSION(2, 18, 0)
	gtk_widget_get_allocation(embed->button, &allocation);
	x += allocation.x;
#endif
	gtk_window_move(GTK_WINDOW(embed->window), x, y);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(embed->button)))
		gtk_widget_show(embed->window);
	else
		gtk_widget_hide(embed->window);
}
