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


/* Debug */
/* private */
/* types */
typedef struct _Debug
{
	GtkWidget * window;
	GtkWidget * gsm;
	GtkWidget * queue;
} Debug;


/* variables */
static struct
{
	char const * name;
	char const * command;
} _debug_gsm_commands[] =
{
	{ "Alive",			"AT"				},
	{ "Answer call",		"ATA"				},
	{ "Battery charge",		"AT+CBC"			},
	{ "Call waiting control",	"AT+CCWA?"			},
	{ "Contact list",		"AT+CPBR=?"			},
	{ "Disable phone",		"AT+CFUN=0"			},
	{ "Enable phone",		"AT+CFUN=1"			},
	{ "Hangup call",		"ATH"				},
	{ "Messages",			"AT+CMGL=4"			},
	{ "Messages read",		"AT+CMGL=1"			},
	{ "Messages sent",		"AT+CMGL=3"			},
	{ "Messages unread",		"AT+CMGL=0"			},
	{ "Messages unsent",		"AT+CMGL=2"			},
	{ "Mute",			"AT+CMUT?"			},
	{ "Operator",			"AT+COPS?"			},
	{ "Phone active",		"AT+CPAS"			},
	{ "Phone functional",		"AT+CFUN?"			},
	{ "Registered",			"AT+CREG?"			},
	{ "Reject call",		"AT+CHUP"			},
	{ "Reset",			"ATZ"				},
	{ "Signal level",		"AT+CSQ"			},
	{ "SIM PIN status",		"AT+CPIN?"			},
	{ NULL,				NULL				}
};


/* prototypes */
static int _debug_init(PhonePlugin * plugin);
static int _debug_destroy(PhonePlugin * plugin);
static int _debug_event(PhonePlugin * plugin, PhoneEvent event, ...);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"Debugging",
	"stock_compile",
	_debug_init,
	_debug_destroy,
	_debug_event,
	NULL,
	NULL
};


/* private */
/* functions */
static void _on_debug_gsm_execute(gpointer data);
static void _on_debug_queue_execute(gpointer data);

static int _debug_init(PhonePlugin * plugin)
{
	Debug * debug;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	size_t i;

	if((debug = object_new(sizeof(*debug))) == NULL)
		return 1;
	plugin->priv = debug;
	debug->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(debug->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(debug->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(debug->window), plugin->icon);
#endif
	gtk_window_set_title(GTK_WINDOW(debug->window), plugin->name);
	vbox = gtk_vbox_new(FALSE, 4);
	/* gsm commands */
	widget = gtk_label_new("GSM commands");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 4);
	debug->gsm = gtk_combo_box_new_text();
	for(i = 0; _debug_gsm_commands[i].name != NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(debug->gsm),
				_debug_gsm_commands[i].name);
	gtk_combo_box_set_active(GTK_COMBO_BOX(debug->gsm), 0);
	gtk_box_pack_start(GTK_BOX(hbox), debug->gsm, TRUE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(_on_debug_gsm_execute), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* gsm queue */
	widget = gtk_label_new("GSM queue");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	debug->queue = gtk_entry_new();
	g_signal_connect_swapped(G_OBJECT(debug->queue), "activate",
			G_CALLBACK(_on_debug_queue_execute), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), debug->queue, TRUE, TRUE,
			0);
	widget = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked",
			G_CALLBACK(_on_debug_queue_execute), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* quit */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				gtk_main_quit), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(debug->window), vbox);
	gtk_widget_show_all(debug->window);
	return 0;
}

static void _on_debug_gsm_execute(gpointer data)
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
	if(_debug_gsm_commands[i].command != NULL)
		plugin->helper->queue(plugin->helper->phone,
				_debug_gsm_commands[i].command);
}

static void _on_debug_queue_execute(gpointer data)
{
	PhonePlugin * plugin = data;
	Debug * debug = plugin->priv;
	char const * text;

	if((text = gtk_entry_get_text(GTK_ENTRY(debug->queue))) == NULL)
		return;
	plugin->helper->queue(plugin->helper->phone, text);
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
static int _debug_event(PhonePlugin * plugin, PhoneEvent event, ...)
{
	/* FIXME implement an event console */
	return 0;
}
