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
#include <time.h>
#include <Phone/modem.h>
#include <gtk/gtk.h>
#include <System.h>


/* Debug */
/* private */
/* types */
typedef struct _Debug
{
	guint source;

	/* widgets */
	GtkWidget * window;
	GtkWidget * status;
	GtkWidget * number;
	GtkWidget * folder;
	GtkWidget * message;

	/* events */
	ModemEvent event_message;
} Debug;


/* prototypes */
/* modem */
static int _debug_init(ModemPlugin * modem);
static int _debug_destroy(ModemPlugin * modem);
static int _debug_start(ModemPlugin * modem, unsigned int retry);
static int _debug_stop(ModemPlugin * modem);

/* accessors */
static void _debug_set_status(ModemPlugin * modem, char const * status);

/* callbacks */
static gboolean _debug_on_closex(gpointer data);
static void _debug_on_message_send(gpointer data);


/* public */
/* variables */
ModemPlugin plugin =
{
	NULL,
	"Debug",
	NULL,
	NULL,
	_debug_init,
	_debug_destroy,
	_debug_start,
	_debug_stop,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* modem */
static int _debug_init(ModemPlugin * modem)
{
	Debug * debug;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if((debug = object_new(sizeof(*debug))) == NULL)
		return -1;
	modem->priv = debug;
	debug->source = 0;
	memset(&debug->event_message, 0, sizeof(debug->event_message));
	/* window */
	debug->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(debug->window), 4);
	gtk_window_set_title(GTK_WINDOW(debug->window), "Debug");
	g_signal_connect_swapped(G_OBJECT(debug->window), "delete-event",
			G_CALLBACK(_debug_on_closex), modem);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox = gtk_vbox_new(FALSE, 4);
	/* status */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Status:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	debug->status = gtk_label_new("initialized");
	gtk_misc_set_alignment(GTK_MISC(debug->status), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), debug->status, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* message */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Number:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	debug->number = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), debug->number, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Folder:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	debug->folder = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(debug->folder), "Unknown");
	gtk_combo_box_append_text(GTK_COMBO_BOX(debug->folder), "Inbox");
	gtk_combo_box_append_text(GTK_COMBO_BOX(debug->folder), "Outbox");
	gtk_combo_box_append_text(GTK_COMBO_BOX(debug->folder), "Drafts");
	gtk_combo_box_append_text(GTK_COMBO_BOX(debug->folder), "Trash");
	gtk_combo_box_set_active(GTK_COMBO_BOX(debug->folder), 1);
	gtk_box_pack_start(GTK_BOX(hbox), debug->folder, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	debug->message = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(widget), debug->message);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_button_new_with_label("Send");
	gtk_button_set_image(GTK_BUTTON(widget), gtk_image_new_from_icon_name(
				"mail-send", GTK_ICON_SIZE_BUTTON));
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(
				_debug_on_message_send), modem);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(debug->window), vbox);
	gtk_widget_show_all(debug->window);
	return 0;
}


/* debug_destroy */
static int _debug_destroy(ModemPlugin * modem)
{
	Debug * debug = modem->priv;

	if(debug->source != 0)
		g_source_remove(debug->source);
	gtk_widget_destroy(debug->window);
	object_delete(debug);
	return 0;
}


/* debug_start */
static gboolean _start_on_idle(gpointer data);

static int _debug_start(ModemPlugin * modem, unsigned int retry)
{
	Debug * debug = modem->priv;

	_debug_set_status(modem, "starting");
	if(debug->source != 0)
		g_source_remove(debug->source);
	debug->source = g_idle_add(_start_on_idle, modem);
	return 0;
}

static gboolean _start_on_idle(gpointer data)
{
	ModemPlugin * modem = data;
	Debug * debug = modem->priv;

	debug->source = 0;
	_debug_set_status(modem, "started");
	return FALSE;
}


/* debug_stop */
static gboolean _stop_on_idle(gpointer data);

static int _debug_stop(ModemPlugin * modem)
{
	Debug * debug = modem->priv;

	_debug_set_status(modem, "stopping");
	if(debug->source != 0)
		g_source_remove(debug->source);
	debug->source = g_idle_add(_stop_on_idle, modem);
	return 0;
}

static gboolean _stop_on_idle(gpointer data)
{
	ModemPlugin * modem = data;
	Debug * debug = modem->priv;

	debug->source = 0;
	_debug_set_status(modem, "stopped");
	return FALSE;
}


/* accessors */
/* debug_set_status */
static void _debug_set_status(ModemPlugin * modem, char const * status)
{
	Debug * debug = modem->priv;

	gtk_label_set_text(GTK_LABEL(debug->status), status);
}


/* callbacks */
/* debug_on_closex */
static gboolean _debug_on_closex(gpointer data)
{
	ModemPlugin * modem = data;
	Debug * debug = modem->priv;

	gtk_widget_hide(debug->window);
	/* FIXME tell the application we're closing */
	return TRUE;
}


/* debug_on_message_send */
static void _debug_on_message_send(gpointer data)
{
	ModemPlugin * modem = data;
	Debug * debug = modem->priv;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;
	gchar * content;

	debug->event_message.message.type = MODEM_EVENT_TYPE_MESSAGE;
	debug->event_message.message.id++;
	debug->event_message.message.date = time(NULL);
	debug->event_message.message.number = gtk_entry_get_text(GTK_ENTRY(
				debug->number));
	debug->event_message.message.folder = gtk_combo_box_get_active(
			GTK_COMBO_BOX(debug->folder));
	debug->event_message.message.status = MODEM_MESSAGE_STATUS_NEW;
	debug->event_message.message.encoding = MODEM_MESSAGE_ENCODING_UTF8;
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(debug->message));
	gtk_text_buffer_get_start_iter(tbuf, &start);
	gtk_text_buffer_get_end_iter(tbuf, &end);
	content = gtk_text_buffer_get_text(tbuf, &start, &end, FALSE);
	debug->event_message.message.length = strlen(content);
	debug->event_message.message.content = content;
	modem->helper->event(modem->helper->modem, &debug->event_message);
	g_free(content);
}
