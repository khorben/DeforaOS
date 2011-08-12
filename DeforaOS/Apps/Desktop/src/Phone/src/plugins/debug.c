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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include "Phone.h"


/* Debug */
/* private */
/* types */
typedef struct _Debug
{
	GtkWidget * window;
	GtkWidget * gsm;
	GtkListStore * events;
	GtkWidget * view;
} Debug;

typedef struct _DebugPhoneEvents
{
	PhoneEventType event;
	char const * string;
} DebugPhoneEvents;


/* variables */
static struct
{
	char const * name;
	ModemEventType event;
} _debug_gsm_commands[] =
{
	{ "Battery charge",	MODEM_EVENT_TYPE_BATTERY_LEVEL	},
	{ "Call status",	MODEM_EVENT_TYPE_CALL		},
	{ "Contact list",	MODEM_EVENT_TYPE_CONTACT	},
	{ "Model",		MODEM_EVENT_TYPE_MODEL		},
	{ "Registration",	MODEM_EVENT_TYPE_REGISTRATION	},
	{ "Status",		MODEM_EVENT_TYPE_STATUS		},
	{ NULL,			0				}
};

static DebugPhoneEvents _debug_phone_events[] =
{
	{ PHONE_EVENT_TYPE_CALLING,		"CALLING"		},
	{ PHONE_EVENT_TYPE_KEY_TONE,		"KEY_TONE"		},
	{ PHONE_EVENT_TYPE_MODEM_EVENT,		"MODEM_EVENT"		},
	{ PHONE_EVENT_TYPE_NOTIFICATION_OFF,	"NOTIFICATION_OFF"	},
	{ PHONE_EVENT_TYPE_NOTIFICATION_ON,	"NOTIFICATION_ON"	},
	{ PHONE_EVENT_TYPE_ONLINE,		"ONLINE"		},
	{ PHONE_EVENT_TYPE_OFFLINE,		"OFFLINE"		},
	{ PHONE_EVENT_TYPE_ONLINE,		"ONLINE"		},
	{ PHONE_EVENT_TYPE_RESUME,		"RESUME"		},
	{ PHONE_EVENT_TYPE_SET_VOLUME,		"SET_VOLUME"		},
	{ PHONE_EVENT_TYPE_SMS_RECEIVING,	"SMS_RECEIVING"		},
	{ PHONE_EVENT_TYPE_SMS_SENDING,		"SMS_SENDING"		},
	{ PHONE_EVENT_TYPE_SMS_SENT,		"SMS_SENT"		},
	{ PHONE_EVENT_TYPE_SPEAKER_OFF,		"SPEAKER_OFF"		},
	{ PHONE_EVENT_TYPE_SPEAKER_ON,		"SPEAKER_ON"		},
	{ PHONE_EVENT_TYPE_STARTING,		"STARTING"		},
	{ PHONE_EVENT_TYPE_SUSPEND,		"SUSPEND"		},
	{ PHONE_EVENT_TYPE_VIBRATOR_OFF,	"VIBRATOR_OFF"		},
	{ PHONE_EVENT_TYPE_VIBRATOR_ON,		"VIBRATOR_ON"		},
	{ 0,					NULL			},
};


/* prototypes */
static int _debug_init(PhonePlugin * plugin);
static int _debug_destroy(PhonePlugin * plugin);
static int _debug_event(PhonePlugin * plugin, PhoneEvent * event);
static void _debug_send_message(PhonePlugin * plugin, PhoneMessageShow show);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Debugging",
	"applications-development",
	_debug_init,
	_debug_destroy,
	_debug_event,
	NULL,
	NULL
};


/* private */
/* functions */
static void _on_debug_contacts(gpointer data);
static void _on_debug_dialer(gpointer data);
static void _on_debug_logs(gpointer data);
static void _on_debug_messages(gpointer data);
static void _on_debug_queue_execute(gpointer data);
static void _on_debug_settings(gpointer data);

static int _debug_init(PhonePlugin * plugin)
{
	Debug * debug;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkWidget * hbox;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	size_t i;

	if((debug = object_new(sizeof(*debug))) == NULL)
		return 1;
	plugin->priv = debug;
	debug->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(debug->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(debug->window), plugin->icon);
#endif
	gtk_window_set_title(GTK_WINDOW(debug->window), plugin->name);
	vbox = gtk_vbox_new(FALSE, 4);
	/* toolbar */
	widget = gtk_toolbar_new();
	toolitem = gtk_tool_button_new(NULL, "Contacts");
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
			"stock_addressbook");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_debug_contacts), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new(NULL, "Dialer");
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
			"phone-dialer");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_debug_dialer), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new(NULL, "Logs");
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem), "logviewer");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_debug_logs), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new(NULL, "Messages");
	gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
			"stock_mail-compose");
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_debug_messages), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				_on_debug_settings), plugin);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	/* vbox */
	widget = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(debug->window), vbox);
	vbox = widget;
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
	/* gsm queue */
	hbox = gtk_hbox_new(FALSE, 4);
	debug->gsm = gtk_combo_box_new_text();
	for(i = 0; _debug_gsm_commands[i].name != NULL; i++)
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(debug->gsm), NULL,
				_debug_gsm_commands[i].name);
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(debug->gsm),
				_debug_gsm_commands[i].name);
#endif
	gtk_combo_box_set_active(GTK_COMBO_BOX(debug->gsm), 0);
	gtk_box_pack_start(GTK_BOX(hbox), debug->gsm, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(_on_debug_queue_execute), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* events */
	debug->events = gtk_list_store_new(3, G_TYPE_UINT, G_TYPE_STRING,
			G_TYPE_STRING);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	debug->view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				debug->events));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(debug->view), TRUE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Time", renderer,
			"text", 1, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug->view), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Event", renderer,
			"text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(debug->view), column);
	gtk_container_add(GTK_CONTAINER(widget), debug->view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_widget_show_all(debug->window);
	return 0;
}

static void _on_debug_contacts(gpointer data)
{
	PhonePlugin * plugin = data;

	_debug_send_message(plugin, PHONE_MESSAGE_SHOW_CONTACTS);
}

static void _on_debug_dialer(gpointer data)
{
	PhonePlugin * plugin = data;

	_debug_send_message(plugin, PHONE_MESSAGE_SHOW_DIALER);
}

static void _on_debug_logs(gpointer data)
{
	PhonePlugin * plugin = data;

	_debug_send_message(plugin, PHONE_MESSAGE_SHOW_LOGS);
}

static void _on_debug_messages(gpointer data)
{
	PhonePlugin * plugin = data;

	_debug_send_message(plugin, PHONE_MESSAGE_SHOW_MESSAGES);
}

static void _on_debug_queue_execute(gpointer data)
{
	PhonePlugin * plugin = data;
	Debug * debug = plugin->priv;
	gchar * text;
	size_t i;

	if((text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(debug->gsm)))
			== NULL)
		return;
	for(i = 0; _debug_gsm_commands[i].name != NULL; i++)
		if(strcmp(_debug_gsm_commands[i].name, text) == 0)
			break;
	g_free(text);
	plugin->helper->trigger(plugin->helper->phone,
			_debug_gsm_commands[i].event);
}

static void _on_debug_settings(gpointer data)
{
	PhonePlugin * plugin = data;

	_debug_send_message(plugin, PHONE_MESSAGE_SHOW_SETTINGS);
}


/* debug_destroy */
static int _debug_destroy(PhonePlugin * plugin)
{
	Debug * debug = plugin->priv;

	gtk_widget_destroy(debug->window);
	object_delete(debug);
	return 0;
}


/* debug_event */
static int _debug_event(PhonePlugin * plugin, PhoneEvent * event)
{
	Debug * debug = plugin->priv;
	time_t date;
	struct tm t;
	char tbuf[32];
	size_t i;
	char ebuf[32];
	DebugPhoneEvents * dbe = _debug_phone_events;
	GtkTreeIter iter;

	date = time(NULL);
	localtime_r(&date, &t);
	strftime(tbuf, sizeof(tbuf), "%d/%m/%Y %H:%M:%S", &t);
	snprintf(ebuf, sizeof(ebuf), "Unknown (%u)", event->type);
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			snprintf(ebuf, sizeof(ebuf), "MODEM (%u)",
					event->modem_event.event->type);
			break;
		default:
			for(i = 0; dbe[i].string != NULL; i++)
				if(dbe[i].event == event->type)
				{
					snprintf(ebuf, sizeof(ebuf), "%s",
							dbe[i].string);
					break;
				}
			break;
	}
	gtk_list_store_append(debug->events, &iter);
	gtk_list_store_set(debug->events, &iter, 0, date, 1, tbuf, 2, ebuf, -1);
	return 0;
}


/* debug_send_message */
static void _debug_send_message(PhonePlugin * plugin, PhoneMessageShow show)
{
	GdkEvent event;
	GdkEventClient * client = &event.client;

	memset(&event, 0, sizeof(event));
	client->type = GDK_CLIENT_EVENT;
	client->window = NULL;
	client->send_event = TRUE;
	client->message_type = gdk_atom_intern(PHONE_CLIENT_MESSAGE, FALSE);
	client->data_format = 8;
	client->data.b[0] = PHONE_MESSAGE_SHOW;
	client->data.b[1] = show;
	client->data.b[2] = TRUE;
	gdk_event_send_clientmessage_toall(&event);
}
