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



#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* GPRS */
/* private */
/* types */
typedef struct _GPRS
{
	guint source;
	gboolean connected;

	gboolean active;
	GtkWidget * window;
	GtkWidget * attach;
	GtkWidget * apn;
	GtkWidget * username;
	GtkWidget * password;
	GtkWidget * connect;
	GtkWidget * st_image;
	GtkWidget * st_label;
	GtkWidget * st_in;
	GtkWidget * st_out;
} GPRS;


/* prototypes */
/* plugins */
static int _gprs_init(PhonePlugin * plugin);
static int _gprs_destroy(PhonePlugin * plugin);
static int _gprs_event(PhonePlugin * plugin, PhoneEvent * event);
static void _gprs_settings(PhonePlugin * plugin);

static void _gprs_set_connected(PhonePlugin * plugin, gboolean connected,
		char const * message, size_t in, size_t out);

static int _gprs_access_point(PhonePlugin * plugin);
static int _gprs_connect(PhonePlugin * plugin);
static int _gprs_disconnect(PhonePlugin * plugin);

/* callbacks */
static gboolean _gprs_on_timeout(gpointer data);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"GPRS",
	"stock_internet",
	_gprs_init,
	_gprs_destroy,
	_gprs_event,
	_gprs_settings,
	NULL
};


/* private */
/* functions */
/* gprs_init */
static int _gprs_init(PhonePlugin * plugin)
{
	GPRS * gprs;

	if((gprs = object_new(sizeof(*gprs))) == NULL)
		return 1;
	plugin->priv = gprs;
	gprs->source = 0;
	gprs->connected = FALSE;
	gprs->active = FALSE;
	gprs->window = NULL;
	return 0;
}


/* gprs_destroy */
static int _gprs_destroy(PhonePlugin * plugin)
{
	GPRS * gprs = plugin->priv;

	if(gprs->source != 0)
		g_source_remove(gprs->source);
	if(gprs->window != NULL)
		gtk_widget_destroy(gprs->window);
	object_delete(gprs);
	return 0;
}


/* gprs_event */
static int _gprs_event_modem(PhonePlugin * plugin, ModemEvent * event);

static int _gprs_event(PhonePlugin * plugin, PhoneEvent * event)
{
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			return _gprs_event_modem(plugin,
					event->modem_event.event);
		default: /* not relevant */
			return 0;
	}
}

static int _gprs_event_modem(PhonePlugin * plugin, ModemEvent * event)
{
	GPRS * gprs = plugin->priv;
	gboolean connected;

	switch(event->type)
	{
		case MODEM_EVENT_TYPE_CONNECTION:
			connected = event->connection.connected;
			if(connected && gprs->source == 0)
				gprs->source = g_timeout_add(1000,
						_gprs_on_timeout, plugin);
			_gprs_set_connected(plugin, connected, connected
					? "Connected" : "Not connected",
					event->connection.in,
					event->connection.out);
			return 0;
		case MODEM_EVENT_TYPE_REGISTRATION:
			if(gprs->active != FALSE)
				break;
			if(event->registration.status
					!= MODEM_REGISTRATION_STATUS_REGISTERED)
				break;
			gprs->active = TRUE;
			/* FIXME optionally force GPRS registration */
			return 0;
		default:
			break;
	}
	return 0;
}


/* gprs_settings */
static void _on_settings_apply(gpointer data);
static void _on_settings_cancel(gpointer data);
static gboolean _on_settings_closex(gpointer data);
static void _on_settings_connect(gpointer data);
static void _on_settings_ok(gpointer data);

static void _gprs_settings(PhonePlugin * plugin)
{
	GPRS * gprs = plugin->priv;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkSizeGroup * group;
	GtkWidget * bbox;
	GtkWidget * widget;

	if(gprs->window != NULL)
	{
		gtk_window_present(GTK_WINDOW(gprs->window));
		return;
	}
	gprs->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(gprs->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(gprs->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(gprs->window), "stock_internet");
#endif
	gtk_window_set_title(GTK_WINDOW(gprs->window), "GPRS preferences");
	g_signal_connect_swapped(G_OBJECT(gprs->window), "delete-event",
			G_CALLBACK(_on_settings_closex), plugin);
	vbox = gtk_vbox_new(FALSE, 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* attachment */
	gprs->attach = gtk_check_button_new_with_label(
			"Force GPRS registration");
	gtk_box_pack_start(GTK_BOX(vbox), gprs->attach, FALSE, TRUE, 0);
	/* access point */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Access point:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->apn = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), gprs->apn, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* username */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Username:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->username = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), gprs->username, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* password */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->password = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(gprs->password), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->password, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* connect */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gprs->connect = gtk_button_new_from_stock(GTK_STOCK_CONNECT);
	g_signal_connect_swapped(G_OBJECT(gprs->connect), "clicked", G_CALLBACK(
				_on_settings_connect), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->connect, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* status */
	widget = gtk_frame_new("Status");
	bbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(bbox), 4);
	hbox = gtk_hbox_new(FALSE, 4);
	gprs->st_image = gtk_image_new_from_icon_name(GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_image, FALSE, TRUE, 0);
	gprs->st_label = gtk_label_new("Not connected");
	gtk_misc_set_alignment(GTK_MISC(gprs->st_label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bbox), hbox, FALSE, TRUE, 0);
	gprs->st_in = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(gprs->st_in), 0.0, 0.5);
	gtk_widget_set_no_show_all(gprs->st_in, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), gprs->st_in, FALSE, TRUE, 0);
	gprs->st_out = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(gprs->st_out), 0.0, 0.5);
	gtk_widget_set_no_show_all(gprs->st_out, TRUE);
	gtk_box_pack_start(GTK_BOX(bbox), gprs->st_out, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(widget), bbox);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* button box */
	bbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_cancel), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_settings_ok), plugin);
	gtk_container_add(GTK_CONTAINER(bbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(gprs->window), vbox);
	_on_settings_cancel(plugin);
	gtk_widget_show_all(gprs->window);
}

static void _on_settings_apply(gpointer data)
{
	PhonePlugin * plugin = data;
	GPRS * gprs = plugin->priv;
	gboolean active;
	char const * p;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gprs->attach));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "attach",
			active ? "1" : "0");
	p = gtk_entry_get_text(GTK_ENTRY(gprs->apn));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "apn", p);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->username));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "username",
			p);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->password));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "password",
			p);
	_gprs_access_point(plugin);
	gprs->active = FALSE;
}

static void _on_settings_cancel(gpointer data)
{
	PhonePlugin * plugin = data;
	GPRS * gprs = plugin->priv;
	char const * p;

	gtk_widget_hide(gprs->window);
	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"attach")) != NULL
			&& strtoul(p, NULL, 10) != 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gprs->attach),
				TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gprs->attach),
				FALSE);
	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"apn")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->apn), p);
	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"username")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->username), p);
	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"password")) == NULL)
		p = "";
	gtk_entry_set_text(GTK_ENTRY(gprs->password), p);
}

static gboolean _on_settings_closex(gpointer data)
{
	PhonePlugin * plugin = data;

	_on_settings_cancel(plugin);
	return TRUE;
}

static void _on_settings_connect(gpointer data)
{
	PhonePlugin * plugin = data;
	GPRS * gprs = plugin->priv;

	_on_settings_apply(plugin);
	if(gprs->connected)
		_gprs_disconnect(plugin);
	else
		_gprs_connect(plugin);
}

static void _on_settings_ok(gpointer data)
{
	PhonePlugin * plugin = data;
	GPRS * gprs = plugin->priv;

	gtk_widget_hide(gprs->window);
	_on_settings_apply(plugin);
}


/* accessors */
/* gprs_set_connected */
static void _gprs_set_connected(PhonePlugin * plugin, gboolean connected,
		char const * message, size_t in, size_t out)
{
	GPRS * gprs = plugin->priv;
	char buf[32];

	gprs->connected = connected;
	if(gprs->window == NULL)
		return;
	gtk_image_set_from_icon_name(GTK_IMAGE(gprs->st_image), connected
			? GTK_STOCK_CONNECT : GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_label_set_text(GTK_LABEL(gprs->st_label), message);
	gtk_button_set_label(GTK_BUTTON(gprs->connect), connected
			? GTK_STOCK_DISCONNECT : GTK_STOCK_CONNECT);
	if(connected)
	{
		snprintf(buf, sizeof(buf), "Received: %lu kB", in / 1024);
		gtk_label_set_text(GTK_LABEL(gprs->st_in), buf);
		snprintf(buf, sizeof(buf), "Sent: %lu kB", out / 1024);
		gtk_label_set_text(GTK_LABEL(gprs->st_out), buf);
		gtk_widget_show(gprs->st_in);
		gtk_widget_show(gprs->st_out);
	}
	else
	{
		if(gprs->source != 0)
			g_source_remove(gprs->source);
		gprs->source = 0;
		gtk_widget_hide(gprs->st_in);
		gtk_widget_hide(gprs->st_out);
	}
}


/* useful */
/* gprs_access_point */
static int _gprs_access_point(PhonePlugin * plugin)
{
	int ret = 0;
	char const * p;
	ModemRequest request;

	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"apn")) == NULL)
		return 0;
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_AUTHENTICATE;
	/* set the access point */
	request.authenticate.name = "APN";
	request.authenticate.username = "IP";
	request.authenticate.password = p;
	ret |= plugin->helper->request(plugin->helper->phone, &request);
	/* set the credentials */
	request.authenticate.name = "GPRS";
	p = plugin->helper->config_get(plugin->helper->phone, "gprs",
			"username");
	request.authenticate.username = p;
	p = plugin->helper->config_get(plugin->helper->phone, "gprs",
			"password");
	request.authenticate.password = p;
	ret |= plugin->helper->request(plugin->helper->phone, &request);
	return ret;
}


/* gprs_connect */
static int _gprs_connect(PhonePlugin * plugin)
{
	ModemRequest request;

	if(_gprs_access_point(plugin) != 0)
		return -1;
	_gprs_set_connected(plugin, TRUE, "Connecting...", 0, 0);
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL;
	request.call.call_type = MODEM_CALL_TYPE_DATA;
	request.call.number = "*99***1#";
	return plugin->helper->request(plugin->helper->phone, &request);
}


/* gprs_disconnect */
static int _gprs_disconnect(PhonePlugin * plugin)
{
	ModemRequest request;

	if(_gprs_access_point(plugin) != 0)
		return -1;
	_gprs_set_connected(plugin, TRUE, "Disconnecting...", 0, 0);
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL_HANGUP;
	return plugin->helper->request(plugin->helper->phone, &request);
}


/* callbacks */
static gboolean _gprs_on_timeout(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->trigger(plugin->helper->phone,
			MODEM_EVENT_TYPE_CONNECTION);
	return TRUE;
}
