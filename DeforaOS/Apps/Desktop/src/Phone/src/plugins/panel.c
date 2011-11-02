/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <string.h>
#include <gtk/gtk.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include "Phone.h"


/* Panel */
/* private */
/* types */
typedef enum _PanelBattery
{
	PANEL_BATTERY_UNKNOWN = 0,
	PANEL_BATTERY_ERROR,
	PANEL_BATTERY_EMPTY,
	PANEL_BATTERY_CAUTION,
	PANEL_BATTERY_LOW,
	PANEL_BATTERY_GOOD,
	PANEL_BATTERY_FULL
} PanelBattery;
#define PANEL_BATTERY_LAST	PANEL_BATTERY_FULL
#define PANEL_BATTERY_COUNT	(PANEL_BATTERY_LAST + 1)

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
	guint timeout;
	GtkWidget * plug;
	GtkWidget * hbox;
	/* battery */
	PanelBattery battery_level;
	GtkWidget * battery_image;
	guint battery_timeout;
	/* connection status */
	GtkWidget * data;
	GtkWidget * roaming;
	/* signal */
	PanelSignal signal_level;
	GtkWidget * signal_image;
	/* operator */
	GtkWidget * operator;
	/* preferences */
	GtkWidget * window;
	GtkWidget * battery;
	GtkWidget * truncate;
} Panel;


/* prototypes */
static int _panel_init(PhonePlugin * plugin);
static void _panel_destroy(PhonePlugin * plugin);
static int _panel_event(PhonePlugin * plugin, PhoneEvent * event);
static void _panel_settings(PhonePlugin * plugin);

static void _panel_set_battery_level(Panel * panel, gdouble level,
		gboolean charging);
static void _panel_set_operator(Panel * panel, ModemRegistrationStatus status,
		char const * _operator);
static void _panel_set_signal_level(Panel * panel, gdouble level);
static void _panel_set_status(Panel * panel, gboolean data, gboolean roaming);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Panel",
	"gnome-monitor",
	_panel_init,
	_panel_destroy,
	_panel_event,
	_panel_settings,
	NULL
};


/* private */
/* functions */
/* panel_init */
static gboolean _on_plug_delete_event(gpointer data);
static void _on_plug_embedded(gpointer data);
static gboolean _on_battery_timeout(gpointer data);

static int _panel_init(PhonePlugin * plugin)
{
	Panel * panel;
	PangoFontDescription * bold;
	char const * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((panel = object_new(sizeof(*panel))) == NULL)
		return 1;
	plugin->priv = panel;
	panel->timeout = 0;
	bold = pango_font_description_new();
	pango_font_description_set_weight(bold, PANGO_WEIGHT_BOLD);
	panel->plug = gtk_plug_new(0);
	g_signal_connect_swapped(G_OBJECT(panel->plug), "delete-event",
			G_CALLBACK(_on_plug_delete_event), plugin);
	g_signal_connect_swapped(G_OBJECT(panel->plug), "embedded", G_CALLBACK(
				_on_plug_embedded), plugin);
	panel->hbox = gtk_hbox_new(FALSE, 2);
	/* battery */
	panel->battery_timeout = 0;
	panel->battery_level = -1;
	panel->battery_image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->battery_image, FALSE,
			TRUE, 0);
	if((p = plugin->helper->config_get(plugin->helper->phone, "panel",
					"battery")) == NULL
			|| strtol(p, NULL, 10) == 0)
		gtk_widget_set_no_show_all(panel->battery_image, TRUE);
	else if(_on_battery_timeout(plugin) == TRUE)
		panel->battery_timeout = g_timeout_add(5000,
				_on_battery_timeout, plugin);
	/* signal */
	panel->signal_level = -1;
	panel->signal_image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->signal_image, FALSE,
			TRUE, 0);
	/* operator */
	panel->operator = gtk_label_new(NULL);
	if((p = plugin->helper->config_get(plugin->helper->phone, "panel",
					"truncate")) != NULL
			&& strtol(p, NULL, 10) != 0)
		gtk_label_set_ellipsize(GTK_LABEL(panel->operator),
				PANGO_ELLIPSIZE_END);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_label_set_max_width_chars(GTK_LABEL(panel->operator), 12);
#endif
	gtk_widget_modify_font(panel->operator, bold);
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->operator, TRUE, TRUE,
			0);
	_panel_set_signal_level(panel, 0.0 / 0.0);
	/* connection status */
	panel->data = gtk_image_new_from_icon_name("stock_internet",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(panel->data, "Connected to GPRS");
#endif
	gtk_widget_set_no_show_all(panel->data, TRUE);
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->data, FALSE, TRUE, 0);
	panel->roaming = gtk_image_new_from_icon_name("phone-roaming",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
#if GTK_CHECK_VERSION(2, 12, 0)
	gtk_widget_set_tooltip_text(panel->roaming, "Roaming");
#endif
	gtk_widget_set_no_show_all(panel->roaming, TRUE);
	gtk_box_pack_start(GTK_BOX(panel->hbox), panel->roaming, FALSE, TRUE,
			0);
	gtk_container_add(GTK_CONTAINER(panel->plug), panel->hbox);
	gtk_widget_show_all(panel->hbox);
	/* preferences */
	panel->window = NULL;
	pango_font_description_free(bold);
	_on_plug_delete_event(plugin);
	return 0;
}

static gboolean _on_plug_delete_event(gpointer data)
{
	PhonePlugin * plugin = data;
	Panel * panel = plugin->priv;
	GdkEvent event;
	GdkEventClient * client = &event.client;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(panel->timeout == 0)
		panel->timeout = g_timeout_add(5000, _on_plug_delete_event,
				plugin);
	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PHONE_EMBED_MESSAGE, FALSE);
	client->data_format = 32;
	client->data.l[0] = gtk_plug_get_id(GTK_PLUG(panel->plug));
	gdk_event_send_clientmessage_toall(&event);
	return TRUE;
}

static void _on_plug_embedded(gpointer data)
{
	PhonePlugin * plugin = data;
	Panel * panel = plugin->priv;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(panel->timeout != 0)
		g_source_remove(panel->timeout);
	panel->timeout = 0;
	gtk_widget_show(panel->plug);
	plugin->helper->trigger(plugin->helper->phone,
			MODEM_EVENT_TYPE_REGISTRATION);
}

static gboolean _on_battery_timeout(gpointer data)
{
	PhonePlugin * plugin = data;
	ModemRequest request;

	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_BATTERY_LEVEL;
	plugin->helper->request(plugin->helper->phone, &request);
	return TRUE;
}


/* panel_destroy */
static void _panel_destroy(PhonePlugin * plugin)
{
	Panel * panel = plugin->priv;

	if(panel->battery_timeout != 0)
		g_source_remove(panel->battery_timeout);
	if(panel->timeout != 0)
		g_source_remove(panel->timeout);
	gtk_widget_destroy(panel->hbox);
	object_delete(panel);
}


/* panel_event */
static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event);

static int _panel_event(PhonePlugin * plugin, PhoneEvent * event)
{
	Panel * panel = plugin->priv;

	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			return _event_modem_event(plugin,
					event->modem_event.event);
		case PHONE_EVENT_TYPE_OFFLINE:
			_panel_set_operator(panel, -1, "Offline");
			_panel_set_signal_level(panel, 0.0 / 0.0);
			_panel_set_status(panel, FALSE, FALSE);
			break;
		case PHONE_EVENT_TYPE_STARTED:
			_panel_set_operator(panel, -1, "Connecting...");
			_panel_set_signal_level(panel, 0.0 / 0.0);
			_panel_set_status(panel, FALSE, FALSE);
			break;
		case PHONE_EVENT_TYPE_STARTING:
			_panel_set_operator(panel, -1, "Starting...");
			_panel_set_signal_level(panel, 0.0 / 0.0);
			_panel_set_status(panel, FALSE, FALSE);
			break;
		case PHONE_EVENT_TYPE_STOPPED:
			_panel_set_operator(panel, -1, "Disconnected");
			_panel_set_signal_level(panel, 0.0 / 0.0);
			_panel_set_status(panel, FALSE, FALSE);
			break;
		case PHONE_EVENT_TYPE_UNAVAILABLE:
			_panel_set_operator(panel, -1, "Unavailable");
			_panel_set_signal_level(panel, 0.0 / 0.0);
			_panel_set_status(panel, FALSE, FALSE);
			break;
		default:
			break;
	}
	return 0;
}

static int _event_modem_event(PhonePlugin * plugin, ModemEvent * event)
{
	Panel * panel = plugin->priv;
	char const * media = "";
	gboolean data;

	switch(event->type)
	{
		case MODEM_EVENT_TYPE_BATTERY_LEVEL:
			_panel_set_battery_level(panel,
					event->battery_level.level,
					event->battery_level.charging);
			break;
		case MODEM_EVENT_TYPE_REGISTRATION:
			media = event->registration.media;
			data = (media != NULL && strcmp("GPRS", media) == 0)
				? TRUE : FALSE;
			_panel_set_operator(panel, event->registration.status,
					event->registration._operator);
			_panel_set_signal_level(panel,
					event->registration.signal);
			_panel_set_status(panel, data,
					event->registration.roaming);
			break;
		default:
			break;
	}
	return 0;
}


/* panel_set_battery_level */
static void _set_battery_image(Panel * panel, PanelBattery battery,
		gboolean charging);

static void _panel_set_battery_level(Panel * panel, gdouble level,
		gboolean charging)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(plugin, %lf)\n", __func__, level);
#endif
	if(level < 0.0)
		_set_battery_image(panel, PANEL_BATTERY_UNKNOWN, charging);
	else if(level <= 0.01)
		_set_battery_image(panel, PANEL_BATTERY_EMPTY, charging);
	else if(level <= 0.1)
		_set_battery_image(panel, PANEL_BATTERY_CAUTION, charging);
	else if(level <= 0.2)
		_set_battery_image(panel, PANEL_BATTERY_LOW, charging);
	else if(level <= 0.75)
		_set_battery_image(panel, PANEL_BATTERY_GOOD, charging);
	else if(level <= 1.0)
		_set_battery_image(panel, PANEL_BATTERY_FULL, charging);
	else
		_set_battery_image(panel, PANEL_BATTERY_ERROR, FALSE);
}

static void _set_battery_image(Panel * panel, PanelBattery battery,
		gboolean charging)
{
	struct
	{
		char const * icon;
		char const * charging;
	} icons[PANEL_BATTERY_COUNT] =
	{
		{ "stock_dialog-question", "stock_dialog-question" },
		{ "battery-missing", "battery-missing" },
		{ "battery-empty", "battery-caution-charging" },
		{ "battery-caution", "battery-caution-charging" },
		{ "battery-low", "battery-low-charging" },
		{ "battery-good", "battery-good-charging" },
		{ "battery-full", "battery-full-charging" }
	};

	if(panel->battery_level == battery)
		return;
	panel->battery_level = battery;
	/* XXX may not be the correct size */
	gtk_image_set_from_icon_name(GTK_IMAGE(panel->battery_image), charging
			? icons[battery].charging : icons[battery].icon,
			GTK_ICON_SIZE_SMALL_TOOLBAR);
}


/* panel_set_operator */
static void _panel_set_operator(Panel * panel, ModemRegistrationStatus status,
		char const * _operator)
{
	switch(status)
	{
		case MODEM_REGISTRATION_STATUS_DENIED:
			_operator = "Denied";
			break;
		case MODEM_REGISTRATION_STATUS_NOT_SEARCHING:
			_operator = "Not searching";
			break;
		case MODEM_REGISTRATION_STATUS_SEARCHING:
			_operator = "Searching...";
			break;
		case MODEM_REGISTRATION_STATUS_UNKNOWN:
			_operator = "Unknown";
			break;
		case MODEM_REGISTRATION_STATUS_REGISTERED:
		default:
			break;
	}
	gtk_label_set_text(GTK_LABEL(panel->operator), _operator);
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
		"network-wireless",
		"phone-signal-00",
		"phone-signal-25",
		"phone-signal-50",
		"phone-signal-75",
		"phone-signal-100"
	};

	if(panel->signal_level == signal)
		return;
	panel->signal_level = signal;
	/* XXX may not be the correct size */
	gtk_image_set_from_icon_name(GTK_IMAGE(panel->signal_image),
			icons[signal], GTK_ICON_SIZE_SMALL_TOOLBAR);
}


/* panel_set_status */
static void _panel_set_status(Panel * panel, gboolean data, gboolean roaming)
{
	if(data)
		gtk_widget_show(panel->data);
	else
		gtk_widget_hide(panel->data);
	if(roaming)
		gtk_widget_show(panel->roaming);
	else
		gtk_widget_hide(panel->roaming);
}


/* panel_settings */
static void _on_settings_cancel(gpointer data);
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_ok(gpointer data);

static void _panel_settings(PhonePlugin * plugin)
{
	Panel * panel = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * bbox;
	GtkWidget * widget;

	if(panel->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(panel->window));
		return;
	}
	panel->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(panel->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(panel->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(panel->window), "gnome-settings");
#endif
	gtk_window_set_title(GTK_WINDOW(panel->window), "Panel preferences");
	g_signal_connect_swapped(G_OBJECT(panel->window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 0);
	/* check button */
	panel->battery = gtk_check_button_new_with_label(
			"Monitor battery activity");
	gtk_box_pack_start(GTK_BOX(vbox), panel->battery, FALSE, TRUE, 0);
	panel->truncate = gtk_check_button_new_with_label(
			"Shorten the operator name");
	gtk_box_pack_start(GTK_BOX(vbox), panel->truncate, FALSE, TRUE, 0);
	/* button box */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_set_spacing(GTK_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_cancel), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_ok), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(panel->window), vbox);
	_on_settings_cancel(plugin);
	gtk_widget_show_all(panel->window);
}

static void _on_settings_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	Panel * panel = plugin->priv;
	char const * p;
	gboolean active;

	active = ((p = plugin->helper->config_get(plugin->helper->phone,
					"panel", "battery")) != NULL
			&& strtoul(p, NULL, 10) != 0) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(panel->battery), active);
	active = ((p = plugin->helper->config_get(plugin->helper->phone,
					"panel", "truncate")) != NULL
			&& strtoul(p, NULL, 10) != 0) ? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(panel->truncate),
			active);
	gtk_widget_hide(panel->window);
}

static gboolean _on_settings_closex(gpointer data)
{
	PhonePlugin * plugin = data;
	Panel * panel = plugin->priv;

	gtk_widget_hide(panel->window);
	return TRUE;
}

static void _on_settings_ok(gpointer data)
{
	PhonePlugin * plugin = data;
	Panel * panel = plugin->priv;
	gboolean value;

	gtk_widget_hide(panel->window);
	/* battery */
	if((value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						panel->battery))) == TRUE)
	{
		if(panel->battery_timeout == 0)
			panel->battery_timeout = g_timeout_add(5000,
					_on_battery_timeout, plugin);
		_on_battery_timeout(plugin);
		gtk_widget_show(panel->battery_image);
	}
	else
	{
		gtk_widget_hide(panel->battery_image);
		if(panel->battery_timeout != 0)
			g_source_remove(panel->battery_timeout);
		panel->battery_timeout = 0;
	}
	gtk_widget_set_no_show_all(panel->battery_image, !value);
	plugin->helper->config_set(plugin->helper->phone, "panel", "battery",
			value ? "1" : "0");
	/* truncate */
	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				panel->truncate));
	gtk_label_set_ellipsize(GTK_LABEL(panel->operator), value
			? PANGO_ELLIPSIZE_END : PANGO_ELLIPSIZE_NONE);
	plugin->helper->config_set(plugin->helper->phone, "panel", "truncate",
			value ? "1" : "0");
}
