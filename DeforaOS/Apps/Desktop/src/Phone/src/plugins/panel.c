/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Phone */
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
#include <gtk/gtk.h>
#include "Phone.h"
#include "../phone.h"


/* Panel */
/* private */
/* types */
typedef enum _PanelSignal
{
	PANEL_SIGNAL_UNKNOWN,
	PANEL_SIGNAL_00,
	PANEL_SIGNAL_25,
	PANEL_SIGNAL_50,
	PANEL_SIGNAL_75,
	PANEL_SIGNAL_100
} PanelSignal;
#define PANEL_SIGNAL_LAST	PANEL_SIGNAL_100
#define PANEL_SIGNAL_COUNT	(PANEL_SIGNAL_LAST + 1)

typedef struct _Panel
{
	PanelSignal signal;
	GtkWidget * hbox;
	GtkWidget * icon;
	GtkWidget * image;
	GtkWidget * operator;
} Panel;


/* prototypes */
static int _panel_init(PhonePlugin * plugin);
static int _panel_destroy(PhonePlugin * plugin);
static int _panel_event(PhonePlugin * plugin, PhoneEvent event, ...);

static void _panel_set_signal_level(Panel * panel, gdouble level);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Panel",
	NULL,
	_panel_init,
	_panel_destroy,
	_panel_event,
	NULL,
	NULL
};


/* private */
/* functions */
/* panel_init */
static gboolean _on_plug_delete_event(gpointer data);
static void _on_plug_embedded(gpointer data);

static int _panel_init(PhonePlugin * plugin)
{
	Panel * panel;
	PangoFontDescription * bold;
	GdkEvent event;
	GdkEventClient * client = &event.client;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((panel = object_new(sizeof(*panel))) == NULL)
		return 1;
	plugin->priv = panel;
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	panel->signal = -1;
	panel->icon = gtk_plug_new(0);
	g_signal_connect_swapped(G_OBJECT(panel->icon), "delete-event",
			G_CALLBACK(_on_plug_delete_event), panel);
	g_signal_connect_swapped(G_OBJECT(panel->icon), "embedded", G_CALLBACK(
				_on_plug_embedded), panel);
	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PHONE_EMBED_MESSAGE, FALSE);
	client->data_format = 32;
	client->data.l[0] = gtk_plug_get_id(GTK_PLUG(panel->icon));
	gdk_event_send_clientmessage_toall(&event);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	panel->image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->image, FALSE, TRUE, 0);
	panel->operator = gtk_label_new(NULL);
	gtk_widget_modify_font(panel->operator, bold);
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->operator, TRUE, TRUE,
			0);
	_panel_set_signal_level(panel, 0.0 / 0.0);
	gtk_container_add(GTK_CONTAINER(panel->icon), panel->hbox);
	gtk_widget_show_all(panel->icon);
	gtk_widget_map(panel->icon);
	pango_font_description_free(bold);
	return 0;
}

static gboolean _on_plug_delete_event(gpointer data)
{
	/* FIXME start sending messages around */
	return TRUE;
}

static void _on_plug_embedded(gpointer data)
{
	/* FIXME stop sending messages around */
}


/* panel_destroy */
static int _panel_destroy(PhonePlugin * plugin)
{
	Panel * panel = plugin->priv;

	_on_plug_embedded(panel);
	gtk_widget_destroy(panel->hbox);
	return 0;
}


/* panel_event */
static int _event_set_operator(Panel * panel, char const * operator);

static int _panel_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	Panel * panel = plugin->priv;
	va_list ap;
	char const * operator;
	gdouble level;

	va_start(ap, event);
	switch(event)
	{
		case PHONE_EVENT_SET_OPERATOR:
			operator = va_arg(ap, char const *);
			_event_set_operator(panel, operator);
			break;
		case PHONE_EVENT_SET_SIGNAL_LEVEL:
			level = va_arg(ap, gdouble);
			_panel_set_signal_level(panel, level);
			break;
		default:
			break;
	}
	va_end(ap);
	return 0;
}

static int _event_set_operator(Panel * panel, char const * operator)
{
	gtk_label_set_text(GTK_LABEL(panel->operator), operator);
	return 0;
}


/* panel_set_signal_level */
static void _signal_level_set_image(Panel * panel, PanelSignal signal);

static void _panel_set_signal_level(Panel * panel, gdouble level)
{
	if(level < 0.0)
		_signal_level_set_image(panel, PANEL_SIGNAL_00);
	else if(level < 0.25)
		_signal_level_set_image(panel, PANEL_SIGNAL_25);
	else if(level < 0.50)
		_signal_level_set_image(panel, PANEL_SIGNAL_50);
	else if(level < 0.75)
		_signal_level_set_image(panel, PANEL_SIGNAL_75);
	else if(level <= 1.0)
		_signal_level_set_image(panel, PANEL_SIGNAL_100);
	else
		_signal_level_set_image(panel, PANEL_SIGNAL_UNKNOWN);
}

static void _signal_level_set_image(Panel * panel, PanelSignal signal)
{
	char const * icons[PANEL_SIGNAL_COUNT] =
	{
		"stock_cell-phone",
		"phone-signal-00",
		"phone-signal-25",
		"phone-signal-50",
		"phone-signal-75",
		"phone-signal-100"
	};

	if(panel->signal == signal)
		return;
	panel->signal = signal;
	/* XXX may not be the correct size */
	gtk_image_set_from_icon_name(GTK_IMAGE(panel->image), icons[signal],
			GTK_ICON_SIZE_SMALL_TOOLBAR);
}
