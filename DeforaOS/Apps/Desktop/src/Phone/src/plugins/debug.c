/* $Id$ */
/* Copyright (c) 2011-2012 Pierre Pronchery <khorben@defora.org> */
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
typedef struct _PhonePlugin
{
	PhonePluginHelper * helper;
	GtkWidget * window;
	GtkWidget * requests;
	GtkWidget * triggers;
	GtkListStore * events;
	GtkWidget * view;
} Debug;

typedef struct _DebugModemEvent
{
	ModemEventType event;
	char const * string;
} DebugModemEvent;

typedef struct _DebugPhoneEvent
{
	PhoneEventType event;
	char const * string;
} DebugPhoneEvent;


/* variables */
static const struct
{
	char const * name;
	ModemRequestType request;
} _debug_modem_requests[] =
{
	{ "Answer call",	MODEM_REQUEST_CALL_ANSWER	},
	{ "Battery level",	MODEM_REQUEST_BATTERY_LEVEL	},
	{ "List contacts",	MODEM_REQUEST_CONTACT_LIST	},
	{ "List messages",	MODEM_REQUEST_MESSAGE_LIST	},
	{ "Signal level",	MODEM_REQUEST_SIGNAL_LEVEL	},
	{ "Terminate call",	MODEM_REQUEST_CALL_HANGUP	},
	{ NULL,			0				}
};

static struct
{
	char const * name;
	ModemEventType event;
} _debug_modem_triggers[] =
{
	{ "Battery charge",	MODEM_EVENT_TYPE_BATTERY_LEVEL	},
	{ "Call status",	MODEM_EVENT_TYPE_CALL		},
	{ "Contact list",	MODEM_EVENT_TYPE_CONTACT	},
	{ "Message list",	MODEM_EVENT_TYPE_MESSAGE	},
	{ "Model",		MODEM_EVENT_TYPE_MODEL		},
	{ "Registration",	MODEM_EVENT_TYPE_REGISTRATION	},
	{ "Status",		MODEM_EVENT_TYPE_STATUS		},
	{ NULL,			0				}
};

static DebugModemEvent _debug_modem_events[] =
{
	{ MODEM_EVENT_TYPE_AUTHENTICATION,	"AUTHENTICATION"	},
	{ MODEM_EVENT_TYPE_BATTERY_LEVEL,	"BATTERY_LEVEL"		},
	{ MODEM_EVENT_TYPE_CALL,		"CALL"			},
	{ MODEM_EVENT_TYPE_CONNECTION,		"CONNECTION"		},
	{ MODEM_EVENT_TYPE_CONTACT,		"CONTACT"		},
	{ MODEM_EVENT_TYPE_CONTACT_DELETED,	"CONTACT_DELETED"	},
	{ MODEM_EVENT_TYPE_ERROR,		"ERROR"			},
	{ MODEM_EVENT_TYPE_MESSAGE,		"MESSAGE"		},
	{ MODEM_EVENT_TYPE_MESSAGE_DELETED,	"MESSAGE_DELETED"	},
	{ MODEM_EVENT_TYPE_MESSAGE_SENT,	"MESSAGE_SENT"		},
	{ MODEM_EVENT_TYPE_MODEL,		"MODEL"			},
	{ MODEM_EVENT_TYPE_REGISTRATION,	"REGISTRATION"		},
	{ MODEM_EVENT_TYPE_STATUS,		"STATUS"		},
	{ 0,					NULL			},
};

static DebugPhoneEvent _debug_phone_events[] =
{
	{ PHONE_EVENT_TYPE_KEY_TONE,		"KEY_TONE"		},
	{ PHONE_EVENT_TYPE_MODEM_EVENT,		"MODEM_EVENT"		},
	{ PHONE_EVENT_TYPE_NOTIFICATION_OFF,	"NOTIFICATION_OFF"	},
	{ PHONE_EVENT_TYPE_NOTIFICATION_ON,	"NOTIFICATION_ON"	},
	{ PHONE_EVENT_TYPE_ONLINE,		"ONLINE"		},
	{ PHONE_EVENT_TYPE_OFFLINE,		"OFFLINE"		},
	{ PHONE_EVENT_TYPE_ONLINE,		"ONLINE"		},
	{ PHONE_EVENT_TYPE_RESUME,		"RESUME"		},
	{ PHONE_EVENT_TYPE_MESSAGE_RECEIVING,	"MESSAGE_RECEIVING"	},
	{ PHONE_EVENT_TYPE_MESSAGE_SENDING,	"MESSAGE_SENDING"	},
	{ PHONE_EVENT_TYPE_MESSAGE_SENT,	"MESSAGE_SENT"		},
	{ PHONE_EVENT_TYPE_SPEAKER_OFF,		"SPEAKER_OFF"		},
	{ PHONE_EVENT_TYPE_SPEAKER_ON,		"SPEAKER_ON"		},
	{ PHONE_EVENT_TYPE_STARTED,		"STARTED"		},
	{ PHONE_EVENT_TYPE_STARTING,		"STARTING"		},
	{ PHONE_EVENT_TYPE_STOPPED,		"STOPPED"		},
	{ PHONE_EVENT_TYPE_STOPPING,		"STOPPING"		},
	{ PHONE_EVENT_TYPE_SUSPEND,		"SUSPEND"		},
	{ PHONE_EVENT_TYPE_UNAVAILABLE,		"UNAVAILABLE"		},
	{ PHONE_EVENT_TYPE_VIBRATOR_OFF,	"VIBRATOR_OFF"		},
	{ PHONE_EVENT_TYPE_VIBRATOR_ON,		"VIBRATOR_ON"		},
	{ PHONE_EVENT_TYPE_VOLUME_GET,		"VOLUME_GET"		},
	{ PHONE_EVENT_TYPE_VOLUME_SET,		"VOLUME_SET"		},
	{ 0,					NULL			},
};


/* prototypes */
/* plug-in */
static Debug * _debug_init(PhonePluginHelper * helper);
static void _debug_destroy(Debug * debug);
static int _debug_event(Debug * debug, PhoneEvent * event);
static void _debug_settings(Debug * debug);


/* public */
/* variables */
PhonePluginDefinition plugin =
{
	"Debugging",
	"applications-development",
	NULL,
	_debug_init,
	_debug_destroy,
	_debug_event,
	_debug_settings
};


/* private */
/* functions */
/* plug-in */
/* debug_init */
static gboolean _debug_on_closex(gpointer data);
static void _debug_on_queue_request(gpointer data);
static void _debug_on_queue_trigger(gpointer data);

static Debug * _debug_init(PhonePluginHelper * helper)
{
	Debug * debug;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkWidget * hbox;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	size_t i;

	if((debug = object_new(sizeof(*debug))) == NULL)
		return NULL;
	debug->helper = helper;
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	debug->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(debug->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(debug->window), plugin.icon);
#endif
	gtk_window_set_title(GTK_WINDOW(debug->window), plugin.name);
	g_signal_connect_swapped(debug->window, "delete-event", G_CALLBACK(
				_debug_on_closex), debug);
	/* vbox */
	vbox = gtk_vbox_new(FALSE, 0);
	/* modem requests */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	debug->requests = gtk_combo_box_text_new();
	for(i = 0; _debug_modem_requests[i].name != NULL; i++)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(debug->requests),
				NULL, _debug_modem_requests[i].name);
#else
	debug->requests = gtk_combo_box_new_text();
	for(i = 0; _debug_modem_requests[i].name != NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(debug->requests),
				_debug_modem_requests[i].name);
#endif
	gtk_combo_box_set_active(GTK_COMBO_BOX(debug->requests), 0);
	gtk_box_pack_start(GTK_BOX(hbox), debug->requests, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_debug_on_queue_request), debug);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* modem triggers */
	hbox = gtk_hbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	debug->triggers = gtk_combo_box_text_new();
	for(i = 0; _debug_modem_triggers[i].name != NULL; i++)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(debug->triggers),
				NULL, _debug_modem_triggers[i].name);
#else
	debug->triggers = gtk_combo_box_new_text();
	for(i = 0; _debug_modem_triggers[i].name != NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(debug->triggers),
				_debug_modem_triggers[i].name);
#endif
	gtk_combo_box_set_active(GTK_COMBO_BOX(debug->triggers), 0);
	gtk_box_pack_start(GTK_BOX(hbox), debug->triggers, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_debug_on_queue_trigger), debug);
	gtk_size_group_add_widget(group, widget);
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
	gtk_container_add(GTK_CONTAINER(debug->window), vbox);
	gtk_widget_show_all(debug->window);
	return debug;
}

static gboolean _debug_on_closex(gpointer data)
{
	Debug * debug = data;

	gtk_widget_hide(debug->window);
	return TRUE;
}

static void _debug_on_queue_request(gpointer data)
{
	Debug * debug = data;
	PhonePluginHelper * helper = debug->helper;
	gchar * text;
	size_t i;
	ModemRequest request;

	if((text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
						debug->requests))) == NULL)
		return;
	for(i = 0; _debug_modem_requests[i].name != NULL; i++)
		if(strcmp(_debug_modem_requests[i].name, text) == 0)
			break;
	g_free(text);
	memset(&request, 0, sizeof(request));
	request.type = _debug_modem_requests[i].request;
	helper->request(helper->phone, &request);
}

static void _debug_on_queue_trigger(gpointer data)
{
	Debug * debug = data;
	PhonePluginHelper * helper = debug->helper;
	gchar * text;
	size_t i;

	if((text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(
						debug->triggers))) == NULL)
		return;
	for(i = 0; _debug_modem_triggers[i].name != NULL; i++)
		if(strcmp(_debug_modem_triggers[i].name, text) == 0)
			break;
	g_free(text);
	helper->trigger(helper->phone, _debug_modem_triggers[i].event);
}


/* debug_destroy */
static void _debug_destroy(Debug * debug)
{
	gtk_widget_destroy(debug->window);
	object_delete(debug);
}


/* debug_event */
static int _debug_event(Debug * debug, PhoneEvent * event)
{
	time_t date;
	struct tm t;
	char tbuf[32];
	size_t i;
	char ebuf[32];
	DebugModemEvent * dme = _debug_modem_events;
	ModemEventType met;
	DebugPhoneEvent * dpe = _debug_phone_events;
	GtkTreeIter iter;

	date = time(NULL);
	localtime_r(&date, &t);
	strftime(tbuf, sizeof(tbuf), "%d/%m/%Y %H:%M:%S", &t);
	snprintf(ebuf, sizeof(ebuf), "Unknown (%u)", event->type);
	switch(event->type)
	{
		case PHONE_EVENT_TYPE_MODEM_EVENT:
			met = event->modem_event.event->type;
			snprintf(ebuf, sizeof(ebuf), "%s (%u)", "MODEM", met);
			for(i = 0; dme[i].string != NULL; i++)
				if(dme[i].event == met)
				{
					snprintf(ebuf, sizeof(ebuf), "%s %s",
							"MODEM", dme[i].string);
					break;
				}
			break;
		default:
			snprintf(ebuf, sizeof(ebuf), "%s (%u)", "PHONE",
					event->type);
			for(i = 0; dpe[i].string != NULL; i++)
				if(dpe[i].event == event->type)
				{
					snprintf(ebuf, sizeof(ebuf), "%s %s",
							"PHONE", dpe[i].string);
					break;
				}
			break;
	}
	gtk_list_store_append(debug->events, &iter);
	gtk_list_store_set(debug->events, &iter, 0, date, 1, tbuf, 2, ebuf, -1);
	return 0;
}


/* debug_settings */
static void _debug_settings(Debug * debug)
{
	gtk_window_present(GTK_WINDOW(debug->window));
}
