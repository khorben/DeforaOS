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



#include <stdarg.h>
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
	gboolean connected;

	GtkWidget * window;
	GtkWidget * attach;
	GtkWidget * apn;
	GtkWidget * username;
	GtkWidget * password;
	GtkWidget * connect;
	GtkWidget * st_image;
	GtkWidget * st_label;
} GPRS;


/* prototypes */
/* plugins */
static int _gprs_init(PhonePlugin * plugin);
static int _gprs_destroy(PhonePlugin * plugin);
static int _gprs_event(PhonePlugin * plugin, PhoneEvent event, ...);
static void _gprs_settings(PhonePlugin * plugin);

static void _gprs_set_connected(PhonePlugin * plugin, gboolean connected,
		char const * message);

static int _gprs_access_point(PhonePlugin * plugin);
static int _gprs_attach(PhonePlugin * plugin);
static int _gprs_connect(PhonePlugin * plugin);
static int _gprs_disconnect(PhonePlugin * plugin);


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
	gprs->connected = FALSE;
	gprs->window = NULL;
	gprs->st_image = NULL;
	gprs->st_label = NULL;
	return 0;
}


/* gprs_destroy */
static int _gprs_destroy(PhonePlugin * plugin)
{
	GPRS * gprs = plugin->priv;

	if(gprs->window != NULL)
		gtk_widget_destroy(gprs->window);
	object_delete(gprs);
	return 0;
}


/* gprs_event */
static int _gprs_event_functional(PhonePlugin * plugin);
static int _gprs_event_gprs_connection(PhonePlugin * plugin,
		gboolean connected);

static int _gprs_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	va_list ap;
	gboolean connected;

	switch(event)
	{
		case PHONE_EVENT_FUNCTIONAL:
			return _gprs_event_functional(plugin);
		case PHONE_EVENT_GPRS_CONNECTION:
			va_start(ap, event);
			connected = va_arg(ap, gboolean);
			va_end(ap);
			return _gprs_event_gprs_connection(plugin, connected);
		default: /* not relevant */
			return 0;
	}
}

static int _gprs_event_functional(PhonePlugin * plugin)
{
	return _gprs_attach(plugin) | _gprs_access_point(plugin);
}

static int _gprs_event_gprs_connection(PhonePlugin * plugin, gboolean connected)
{
	_gprs_set_connected(plugin, connected, connected ? "Connected"
			: "Not connected");
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
	GtkWidget * widget;
	GtkWidget * bbox;

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
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
	gprs->st_image = gtk_image_new_from_icon_name(GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_image, FALSE, TRUE, 0);
	gprs->st_label = gtk_label_new("Not connected");
	gtk_misc_set_alignment(GTK_MISC(gprs->st_label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), gprs->st_label, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(widget), hbox);
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
	char const * p;
	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gprs->attach));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "attach",
				active ? "1" : "0");
	_gprs_attach(plugin);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->apn));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "apn", p);
	_gprs_access_point(plugin);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->username));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "username",
			p);
	p = gtk_entry_get_text(GTK_ENTRY(gprs->password));
	plugin->helper->config_set(plugin->helper->phone, "gprs", "password",
			p);
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


/* gprs_set_connected */
static void _gprs_set_connected(PhonePlugin * plugin, gboolean connected,
		char const * message)
{
	GPRS * gprs = plugin->priv;

	gprs->connected = connected;
	if(gprs->window == NULL)
		return;
	gtk_image_set_from_icon_name(GTK_IMAGE(gprs->st_image), connected
			? GTK_STOCK_CONNECT : GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
	gtk_label_set_text(GTK_LABEL(gprs->st_label), message);
	gtk_button_set_label(GTK_BUTTON(gprs->connect), connected
			? GTK_STOCK_DISCONNECT : GTK_STOCK_CONNECT);
}


/* gprs_access_point */
static int _gprs_access_point(PhonePlugin * plugin)
{
	int ret;
	char const cmd[] = "AT+CGDCONT=1,\"IP\",";
	char const * p;
	char * q;

	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"apn")) == NULL)
		return 0;
	if((q = string_new_append(cmd, "\"", p, "\"", NULL)) == NULL)
		return -1;
	ret = plugin->helper->queue(plugin->helper->phone, q);
	string_delete(q);
	return ret;
}


/* gprs_attach */
static int _gprs_attach(PhonePlugin * plugin)
{
	char const * cmd = "AT+CGATT=0";
	char const * p;

	if((p = plugin->helper->config_get(plugin->helper->phone, "gprs",
					"attach")) != NULL
			&& strtoul(p, NULL, 10) != 0)
		cmd = "AT+CGATT=1";
	if(plugin->helper->queue(plugin->helper->phone, cmd) != 0)
		return 1;
	return plugin->helper->queue(plugin->helper->phone, "AT+CGATT?");
}


/* gprs_connect */
static int _gprs_connect(PhonePlugin * plugin)
{
	GPRS * gprs = plugin->priv;
	char const * cmd = "ATD*99***1#";

	_gprs_set_connected(plugin, gprs->connected, "Connecting...");
	return plugin->helper->queue(plugin->helper->phone, cmd);
}


/* gprs_disconnect */
static int _gprs_disconnect(PhonePlugin * plugin)
{
	GPRS * gprs = plugin->priv;
	char const * cmd = "ATH"; /* XXX requires interpretation from Phone */

	_gprs_set_connected(plugin, gprs->connected, "Disconnecting...");
	return plugin->helper->queue(plugin->helper->phone, cmd);
}
