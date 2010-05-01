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



#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "gsm.h"
#include "callbacks.h"
#include "phone.h"
#define _(string) gettext(string)


/* Phone */
/* private */
/* types */
typedef enum _PhoneTrack
{
	PHONE_TRACK_CODE_ENTERED = 0,
	PHONE_TRACK_CONTACT_LIST,
	PHONE_TRACK_MESSAGE_LIST,
	PHONE_TRACK_MESSAGE_SENT,
	PHONE_TRACK_REGISTRATION,
	PHONE_TRACK_SIGNAL_LEVEL
} PhoneTrack;
#define PHONE_TRACK_LAST	PHONE_TRACK_SIGNAL_LEVEL
#define PHONE_TRACK_COUNT	(PHONE_TRACK_LAST + 1)

struct _Phone
{
	GSM * gsm;
	guint ui_source;

	/* status */
	gdouble signal_level;
	char * operator;

	/* tracking */
	guint tr_source;
	gboolean tracks[PHONE_TRACK_COUNT];

	/* widgets */
	PangoFontDescription * bold;

	/* code */
	PhoneCode en_code;
	GtkWidget * en_window;
	GtkWidget * en_entry;
	GtkWidget * en_progress;

	/* contacts */
	GtkWidget * co_window;
	GtkListStore * co_store;
	GtkWidget * co_view;

	/* dialer */
	GtkWidget * di_window;
	GtkWidget * di_entry;

	/* messages */
	GtkWidget * me_window;
	GtkListStore * me_store;
	GtkWidget * me_view;

	/* write */
	GtkWidget * wr_window;
	GtkWidget * wr_entry;
	GtkWidget * wr_view;
	GtkWidget * wr_progress;

	/* systray */
#if GTK_CHECK_VERSION(2, 10, 0)
	GtkStatusIcon * sy_icon;
#endif
	GtkWidget * sy_level;
};


/* prototypes */
static GtkWidget * _phone_create_dialpad(Phone * phone,
		char const * button1_image, char const * button1_label,
		GCallback button1_callback,
		char const * button2_image, char const * button2_label,
		GCallback button2_callback,
		GCallback button_callback);
static GtkWidget * _phone_create_progress(GtkWidget * parent,
		char const * text);

static void _phone_error(GtkWidget * window, char const * message);

static void _phone_fetch_contacts(Phone * phone, unsigned int start,
		unsigned int end);
static void _phone_fetch_messages(Phone * phone, unsigned int start,
		unsigned int end);

static void _phone_info(GtkWidget * window, char const * message);

static GtkWidget * _phone_progress_delete(GtkWidget * widget);
static void _phone_progress_pulse(GtkWidget * widget);

static void _phone_set_operator(Phone * phone, char const * operator);
static void _phone_set_signal_level(Phone * phone, gdouble level);
static void _phone_set_status(Phone * phone, GSMStatus status);

static void _phone_track(Phone * phone, PhoneTrack what, gboolean track);

/* callbacks */
static int _phone_gsm_event(GSMEvent * event, gpointer data);
static gboolean _phone_timeout_track(gpointer data);


/* public */
/* functions */
/* phone_new */
static gboolean _new_idle(gpointer data);

Phone * phone_new(char const * device, unsigned int baudrate, int retry)
{
	Phone * phone;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u)\n", __func__, (device != NULL)
			? device : "", baudrate);
#endif
	if((phone = malloc(sizeof(*phone))) == NULL)
		return NULL;
	if(device == NULL)
		device = "/dev/modem";
	phone->gsm = gsm_new(device, baudrate);
	phone->ui_source = g_idle_add(_new_idle, phone);
	phone->tr_source = 0;
	memset(&phone->tracks, 0, sizeof(phone->tracks));
	phone->signal_level = 0.0 / 0.0;
	phone->operator = NULL;
	/* widgets */
	phone->bold = pango_font_description_new();
	pango_font_description_set_weight(phone->bold, PANGO_WEIGHT_BOLD);
	phone->en_window = NULL;
	phone->en_progress = NULL;
	phone->co_window = NULL;
	phone->co_store = gtk_list_store_new(3, G_TYPE_UINT, G_TYPE_STRING,
			G_TYPE_STRING);
	phone->di_window = NULL;
	phone->me_window = NULL;
	phone->me_store = gtk_list_store_new(2, G_TYPE_UINT, G_TYPE_STRING);
	phone->wr_window = NULL;
	phone->wr_progress = NULL;
#if GTK_CHECK_VERSION(2, 10, 0)
	phone->sy_icon = gtk_status_icon_new_from_icon_name(
			"stock_landline-phone");
# if GTK_CHECK_VERSION(2, 18, 0)
	gtk_status_icon_set_title(phone->sy_icon, _("Phone"));
# endif
#endif
	phone->sy_level = NULL;
	/* check errors */
	if(phone->gsm == NULL)
	{
		phone_delete(phone);
		return NULL;
	}
	if(retry >= 0)
		gsm_set_retry(phone->gsm, retry);
	gsm_set_callback(phone->gsm, _phone_gsm_event, phone);
	_phone_set_operator(phone, _("Initializing..."));
	return phone;
}

static gboolean _new_idle(gpointer data)
{
	Phone * phone = data;

	phone_show_contacts(phone, FALSE);
	phone_show_dialer(phone, TRUE);
	phone_show_messages(phone, FALSE);
	phone->ui_source = 0;
	return FALSE;
}


/* phone_delete */
void phone_delete(Phone * phone)
{
	if(phone->ui_source != 0)
		g_source_remove(phone->ui_source);
	if(phone->tr_source != 0)
		g_source_remove(phone->tr_source);
	free(phone->operator);
	pango_font_description_free(phone->bold);
	if(phone->gsm != NULL)
		gsm_delete(phone->gsm);
	free(phone);
}


/* useful */
/* phone_error */
static int _error_text(char const * message, int ret);

int phone_error(Phone * phone, char const * message, int ret)
{
	if(phone == NULL)
		return _error_text(message, ret);
	_phone_error(NULL, message);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fprintf(stderr, "phone: %s\n", message);
	return ret;
}


/* phone_call */
void phone_call(Phone * phone, char const * number)
{
	if(number == NULL)
		number = gtk_entry_get_text(GTK_ENTRY(phone->di_entry));
	if(number[0] == '\0')
		number = NULL; /* call the last number dialled */
	gsm_call(phone->gsm, GSM_CALL_TYPE_VOICE, number);
}


/* phone_code_append */
int phone_code_append(Phone * phone, char character)
{
	char const * text;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%c)\n", __func__, character);
#endif
	if((character < '0' || character > '9') && character != '*'
			&& character != '+' && character != '#')
		return 1; /* ignore the error */
	text = gtk_entry_get_text(GTK_ENTRY(phone->en_entry));
	len = strlen(text);
	if((p = malloc(len + 2)) == NULL)
		return phone_error(phone, strerror(errno), 1);
	snprintf(p, len + 2, "%s%c", text, character);
	gtk_entry_set_text(GTK_ENTRY(phone->en_entry), p);
	free(p);
	return 0;
}


/* phone_code_enter */
void phone_code_enter(Phone * phone)
{
	char const * p;

	if(phone->en_window == NULL)
		return;
	switch(phone->en_code)
	{
		case PHONE_CODE_SIM_PIN:
			p = gtk_entry_get_text(GTK_ENTRY(phone->en_entry));
			gsm_enter_sim_pin(phone->gsm, p);
			phone->en_progress = _phone_create_progress(
					phone->en_window,
					_("Checking SIM PIN code..."));
			_phone_track(phone, PHONE_TRACK_CODE_ENTERED, TRUE);
			break;
	}
}


/* code */
/* phone_code_clear */
void phone_code_clear(Phone * phone)
{
	if(phone->en_window != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->en_entry), "");
}


/* contacts */
/* phone_contacts_add */
void phone_contacts_add(Phone * phone, unsigned int index, char const * name,
		char const * number)
{
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u, \"%s\", \"%s\")\n", __func__, index,
			name, number);
#endif
	gtk_list_store_append(phone->co_store, &iter);
	gtk_list_store_set(phone->co_store, &iter, 0, index, 1, name, 2, number,
			-1);
}


/* phone_contacts_call_selected */
void phone_contacts_call_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	unsigned int index;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter, 0, &index,
			-1);
	gsm_call_contact(phone->gsm, GSM_CALL_TYPE_VOICE, index);
}


/* phone_contacts_write_selected */
void phone_contacts_write_selected(Phone * phone)
{
	GtkTreeSelection * treesel;
	GtkTreeIter iter;
	gchar * number = NULL;

	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						phone->co_view))) == NULL)
		return;
	if(gtk_tree_selection_get_selected(treesel, NULL, &iter) == TRUE)
		gtk_tree_model_get(GTK_TREE_MODEL(phone->co_store), &iter,
				2, &number, -1);
	phone_messages_write(phone, number, "");
	g_free(number);
}


/* phone_dialer_append */
int phone_dialer_append(Phone * phone, char character)
{
	char const * text;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%c)\n", __func__, character);
#endif
	if(phone->di_window == NULL)
		return 1;
	if((character < '0' || character > '9') && character != '*'
			&& character != '+' && character != '#')
		return 1; /* ignore the error */
	/* FIXME ask GSM if in a call; if yes, send DTMF */
	text = gtk_entry_get_text(GTK_ENTRY(phone->di_entry));
	len = strlen(text);
	if((p = malloc(len + 2)) == NULL)
		return phone_error(phone, strerror(errno), 1);
	snprintf(p, len + 2, "%s%c", text, character);
	gtk_entry_set_text(GTK_ENTRY(phone->di_entry), p);
	free(p);
	return 0;
}


/* phone_hangup */
void phone_hangup(Phone * phone)
{
	gsm_hangup(phone->gsm);
	if(phone->di_window != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->di_entry), "");
}


/* messages */
/* phone_messages_send */
void phone_messages_send(Phone * phone)
{
	gchar const * number;
	gchar * text;
	GtkTextBuffer * tbuf;
	GtkTextIter start;
	GtkTextIter end;

	phone_show_write(phone, TRUE);
	number = gtk_entry_get_text(GTK_ENTRY(phone->wr_entry));
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tbuf), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tbuf), &end);
	text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(tbuf), &start, &end,
			FALSE);
	if(number == NULL || number[0] == '\0' || text == NULL)
		return;
	gsm_send_message(phone->gsm, number, text);
	g_free(text);
	phone->wr_progress = _phone_create_progress(phone->wr_window,
			_("Sending message..."));
	_phone_track(phone, PHONE_TRACK_MESSAGE_SENT, TRUE);
}


/* phone_messages_write */
void phone_messages_write(Phone * phone, char const * number, char const * text)
{
	GtkTextBuffer * tbuf;

	phone_show_write(phone, TRUE);
	if(number != NULL)
		gtk_entry_set_text(GTK_ENTRY(phone->wr_entry), number);
	if(text != NULL)
	{
		tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(phone->wr_view));
		gtk_text_buffer_set_text(tbuf, text, strlen(text));
	}
}


/* phone_show_code */
void phone_show_code(Phone * phone, gboolean show, ...)
{
	va_list ap;
	PhoneCode code;
	GtkWidget * vbox;
	GtkWidget * hbox; /* XXX create in phone_create_dialpad? */
	GtkWidget * widget;

	if(show == FALSE)
	{
		if(phone->en_window != NULL)
			gtk_widget_hide(phone->en_window);
		return;
	}
	va_start(ap, show);
	code = va_arg(ap, PhoneCode);
	va_end(ap);
	if(phone->en_window == NULL)
	{
		phone->en_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->en_window),
				"stock_lock");
#endif
		vbox = gtk_vbox_new(FALSE, 0);
		hbox = gtk_hbox_new(FALSE, 0);
		phone->en_entry = gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(phone->en_entry), FALSE);
		gtk_widget_modify_font(phone->en_entry, phone->bold);
		g_signal_connect_swapped(G_OBJECT(phone->en_entry), "activate",
				G_CALLBACK(on_phone_code_enter), phone);
		gtk_box_pack_start(GTK_BOX(hbox), phone->en_entry, TRUE, TRUE,
				2);
		widget = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name("edit-undo",
					GTK_ICON_SIZE_BUTTON));
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_code_clear), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
		widget = _phone_create_dialpad(phone, GTK_STOCK_OK, _("Enter"),
				G_CALLBACK(on_phone_code_enter),
				GTK_STOCK_CANCEL, _("Skip"),
				G_CALLBACK(on_phone_code_leave),
				G_CALLBACK(on_phone_code_clicked));
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->en_window), vbox);
		gtk_widget_show_all(vbox);
	}
	switch(code)
	{
		case PHONE_CODE_SIM_PIN:
			gtk_window_set_title(GTK_WINDOW(phone->en_window),
					_("Enter SIM PIN code"));
			break;
	}
	if(phone->en_code != code)
		phone_code_clear(phone);
	phone->en_code = code;
	gtk_window_present(GTK_WINDOW(phone->en_window));
}


/* phone_show_contacts */
void phone_show_contacts(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(phone->co_window == NULL)
	{
		phone->co_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->co_window),
				"stock_addressbook");
#endif
		gtk_window_set_default_size(GTK_WINDOW(phone->co_window), 200,
				300);
		gtk_window_set_title(GTK_WINDOW(phone->co_window),
				_("Contacts"));
		g_signal_connect_swapped(G_OBJECT(phone->co_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->co_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Call"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"call-start");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_call), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_edit), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Write"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-reply-sender");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_write), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_contacts_delete), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		phone->co_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->co_store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->co_view),
				FALSE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Contact"),
				renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->co_view),
				column);
		gtk_container_add(GTK_CONTAINER(widget), phone->co_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->co_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->co_window));
	else
		gtk_widget_hide(phone->co_window);
}


/* phone_show_dialer */
void phone_show_dialer(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if(phone->di_window == NULL)
	{
		phone->di_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(phone->di_window),
				"stock_landline-phone");
#endif
		gtk_window_set_title(GTK_WINDOW(phone->di_window), _("Dialer"));
		g_signal_connect_swapped(G_OBJECT(phone->di_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->di_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* XXX signal level (place in systray) */
		hbox = gtk_hbox_new(FALSE, 0);
		widget = gtk_label_new(_("Signal:"));
		gtk_widget_modify_font(widget, phone->bold);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
		phone->sy_level = gtk_progress_bar_new();
		_phone_set_signal_level(phone, phone->signal_level);
		gtk_box_pack_start(GTK_BOX(hbox), phone->sy_level, TRUE, TRUE,
				2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
		/* entry */
		hbox = gtk_hbox_new(FALSE, 0);
		phone->di_entry = gtk_entry_new();
		gtk_widget_modify_font(phone->di_entry, phone->bold);
		g_signal_connect_swapped(G_OBJECT(phone->di_entry), "activate",
				G_CALLBACK(on_phone_dialer_call), phone);
		gtk_box_pack_start(GTK_BOX(hbox), phone->di_entry, TRUE, TRUE,
				2);
		widget = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name(
					"stock_addressbook",
					GTK_ICON_SIZE_BUTTON));
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_contacts_show), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE,
				2);
		/* dialpad */
		widget = _phone_create_dialpad(phone, "call-start", _("Call"),
				G_CALLBACK(on_phone_dialer_call),
				"call-stop", _("Hang up"),
				G_CALLBACK(on_phone_dialer_hangup),
				G_CALLBACK(on_phone_dialer_clicked));
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->di_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->di_window));
	else
		gtk_widget_hide(phone->di_window);
}


/* phone_show_messages */
void phone_show_messages(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkWidget * hbox;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(phone->me_window == NULL)
	{
		phone->me_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->me_window), 200,
				300);
		gtk_window_set_title(GTK_WINDOW(phone->me_window),
				_("Messages"));
		g_signal_connect_swapped(G_OBJECT(phone->me_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->me_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* toolbar */
		widget = gtk_toolbar_new();
		toolitem = gtk_tool_button_new(NULL, _("Call"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"call-start");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_messages_call), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("New message"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"stock_mail-compose");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_messages_write), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new(NULL, _("Reply"));
		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(toolitem),
				"mail-reply-sender");
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_messages_reply), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
		g_signal_connect_swapped(G_OBJECT(toolitem), "clicked",
				G_CALLBACK(on_phone_messages_delete), phone);
		gtk_toolbar_insert(GTK_TOOLBAR(widget), toolitem, -1);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
		/* selector */
		/* FIXME consider:
		 * - creating a notebook here ("Inbox", "Outbox", "All")
		 * - each page has a GtkTreeView
		 * - each GtkTreeView has a GtkTreeModelFilter as model
		 * - each GtkTreeModelFilter is a variant of the GtkListStore */
		hbox = gtk_hbox_new(TRUE, 0);
		widget = gtk_button_new_with_label(_("Inbox"));
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name("stock_inbox",
					GTK_ICON_SIZE_BUTTON));
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_messages_inbox), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
		widget = gtk_button_new_with_label(_("Sent"));
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name("stock_outbox",
					GTK_ICON_SIZE_BUTTON));
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_messages_outbox), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
		/* view */
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		phone->me_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
					phone->me_store));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(phone->me_view),
				FALSE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Message"),
				renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(phone->me_view),
				column);
		gtk_container_add(GTK_CONTAINER(widget), phone->me_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->me_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->me_window));
	else
		gtk_widget_hide(phone->me_window);
}


/* phone_show_write */
void phone_show_write(Phone * phone, gboolean show)
{
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;

	if(phone->wr_window == NULL)
	{
		phone->wr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(phone->wr_window), 200,
				300);
		gtk_window_set_title(GTK_WINDOW(phone->wr_window),
				_("Write message"));
		g_signal_connect(G_OBJECT(phone->wr_window), "delete-event",
				G_CALLBACK(on_phone_closex), phone->wr_window);
		vbox = gtk_vbox_new(FALSE, 0);
		hbox = gtk_hbox_new(FALSE, 0);
		phone->wr_entry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(hbox), phone->wr_entry, TRUE, TRUE,
				2);
		widget = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name("mail-send",
					GTK_ICON_SIZE_BUTTON));
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_messages_send), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
		widget = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		phone->wr_view = gtk_text_view_new();
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(phone->wr_view),
				GTK_WRAP_WORD_CHAR);
		gtk_container_add(GTK_CONTAINER(widget), phone->wr_view);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 2);
		gtk_container_add(GTK_CONTAINER(phone->wr_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_window_present(GTK_WINDOW(phone->wr_window));
	else
		gtk_widget_hide(phone->wr_window);
}


/* private */
/* phone_create_dialpad */
static GtkWidget * _phone_create_dialpad(Phone * phone,
		char const * button1_image, char const * button1_label,
		GCallback button1_callback,
		char const * button2_image, char const * button2_label,
		GCallback button2_callback,
		GCallback button_callback)
{
	static struct
	{
		char character;
		char const * label;
	} numbers[12] = {
		{ '1', "<b>_1</b>\n" },
		{ '2', "<b>_2</b>\nABC" },
		{ '3', "<b>_3</b>\nDEF" },
		{ '4', "<b>_4</b>\nGHI" },
		{ '5', "<b>_5</b>\nJKL" },
		{ '6', "<b>_6</b>\nMNO" },
		{ '7', "<b>_7</b>\nPQRS" },
		{ '8', "<b>_8</b>\nTUV" },
		{ '9', "<b>_9</b>\nWXYZ" },
		{ '*', "<b>_*</b>\n+" },
		{ '0', "<b>_0</b>\n" },
		{ '#', "<b>_#</b>\n" }
	};
	GtkWidget * table;
	GtkWidget * button;
	GtkWidget * image;
	GtkWidget * label;
	int i;

	table = gtk_table_new(5, 6, TRUE);
	/* call */
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name(button1_image,
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), button1_label);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", button1_callback,
			phone);
	gtk_table_attach(GTK_TABLE(table), button, 0, 3, 0, 1,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name(button2_image,
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), button2_label);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", button2_callback,
			phone);
	gtk_table_attach(GTK_TABLE(table), button, 3, 6, 0, 1,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);
	/* numbers */
	for(i = 0; i < 12; i++)
	{
		button = gtk_button_new();
		label = gtk_label_new(NULL);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),
				numbers[i].label);
		gtk_container_add(GTK_CONTAINER(button), label);
		g_object_set_data(G_OBJECT(button), "character",
				&numbers[i].character);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					button_callback), phone);
		gtk_table_attach(GTK_TABLE(table), button, (i % 3) * 2,
				((i % 3) + 1) * 2, (i / 3) + 1, (i / 3) + 2,
				GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
				2, 2);
	}
	return table;
}


/* phone_create_progress */
static GtkWidget * _phone_create_progress(GtkWidget * parent, char const * text)
{
	GtkWidget * window;
	GtkWidget * vbox;
	GtkWidget * widget;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				on_phone_closex), NULL);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_title(GTK_WINDOW(window), _("Operation in progress..."));
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
	vbox = gtk_vbox_new(FALSE, 0);
	widget = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 2);
	widget = gtk_progress_bar_new();
	g_object_set_data(G_OBJECT(window), "progress", widget);
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(widget));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget), " ");
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 2);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
	return window;
}


/* phone_error */
static void _phone_error(GtkWidget * window, char const * message)
{
	GtkWidget * dialog;
	GtkWindow * w = (window != NULL) ? GTK_WINDOW(window) : NULL;
	GtkDialogFlags flags = (window != NULL)
		? GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT : 0;

	dialog = gtk_message_dialog_new(w, flags, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
}


/* phone_fetch_contacts */
static void _phone_fetch_contacts(Phone * phone, unsigned int start,
		unsigned int end)
{
	unsigned int i;

	for(i = start + 10; i < end; i+=10)
	{
		gsm_fetch_contacts(phone->gsm, start, i);
		start = i;
	}
	if(start < end)
		gsm_fetch_contacts(phone->gsm, start, end);
}


/* phone_fetch_messages */
static void _phone_fetch_messages(Phone * phone, unsigned int start,
		unsigned int end)
{
	unsigned int i;

	for(i = start + 10; i < end; i+=10)
	{
		gsm_fetch_messages(phone->gsm, start, i);
		start = i;
	}
	if(start < end)
		gsm_fetch_messages(phone->gsm, start, end);
}


/* phone_info */
static void _phone_info(GtkWidget * window, char const * message)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s",
#if GTK_CHECK_VERSION(2, 8, 0)
			_("Information"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Information"));
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
}


/* phone_progress_delete */
static GtkWidget * _phone_progress_delete(GtkWidget * widget)
{
	if(widget != NULL)
		gtk_widget_destroy(widget);
	return NULL;
}


/* phone_progress_pulse */
static void _phone_progress_pulse(GtkWidget * widget)
{
	GtkWidget * progress;

	if((progress = g_object_get_data(G_OBJECT(widget), "progress")) == NULL)
		return;
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
}


/* phone_set_operator */
static void _phone_set_operator(Phone * phone, char const * operator)
{
	free(phone->operator);
	phone->operator = (operator != NULL) ? strdup(operator) : NULL;
	/* XXX ugly */
	_phone_set_signal_level(phone, phone->signal_level);
}


/* phone_set_signal_level */
static void _phone_set_signal_level(Phone * phone, gdouble level)
{
	char buf[32];

	phone->signal_level = level;
	if(phone->sy_level == NULL)
		return;
	if(level >= 0.0 && level <= 1.0)
		snprintf(buf, sizeof(buf), "%.0lf/10 %s", level * 10,
				(phone->operator != NULL) ? phone->operator
				: "");
	else
	{
		level = 0.0;
		snprintf(buf, sizeof(buf), "%s", (phone->operator != NULL)
				? phone->operator : "");
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(phone->sy_level), level);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(phone->sy_level), buf);
}


/* phone_set_status */
static void _phone_set_status(Phone * phone, GSMStatus status)
{
	GSMRegistrationReport report;
	char const * operator = NULL;
	gboolean track_registration = TRUE;

	report = GSM_REGISTRATION_REPORT_ENABLE_UNSOLLICITED_WITH_LOCATION;
	switch(status)
	{
		case GSM_STATUS_UNKNOWN:
			operator = _("Unknown");
			break;
		case GSM_STATUS_REGISTERING:
			operator = _("Registering...");
			break;
		case GSM_STATUS_REGISTERING_DENIED:
			operator = _("Denied");
			break;
		case GSM_STATUS_INITIALIZED:
			operator = _("SIM check...");
			track_registration = FALSE;
			gsm_is_pin_needed(phone->gsm);
			break;
		case GSM_STATUS_READY:
			operator = _("SIM ready...");
			_phone_track(phone, PHONE_TRACK_CONTACT_LIST, TRUE);
			_phone_track(phone, PHONE_TRACK_MESSAGE_LIST, TRUE);
			gsm_set_operator_mode(phone->gsm,
					GSM_OPERATOR_MODE_AUTOMATIC);
			gsm_set_registration_report(phone->gsm, report);
			break;
		case GSM_STATUS_REGISTERED_HOME:
		case GSM_STATUS_REGISTERED_ROAMING:
			track_registration = FALSE;
			_phone_track(phone, PHONE_TRACK_SIGNAL_LEVEL, TRUE);
			gsm_set_operator_format(phone->gsm,
					GSM_OPERATOR_FORMAT_LONG);
			gsm_fetch_operator(phone->gsm);
			gsm_fetch_signal_level(phone->gsm);
			return;
	}
	_phone_track(phone, PHONE_TRACK_REGISTRATION, track_registration);
	if(operator != NULL)
		_phone_set_operator(phone, operator);
	_phone_set_signal_level(phone, 0.0 / 0.0);
}


/* phone_track */
static void _phone_track(Phone * phone, PhoneTrack what, gboolean track)
{
	size_t i;

	phone->tracks[what] = track;
	if(track)
	{
		if(phone->tr_source == 0)
			phone->tr_source = g_timeout_add(2000,
					_phone_timeout_track, phone);
	}
	else if(phone->tr_source != 0)
	{
		for(i = 0; i < PHONE_TRACK_COUNT; i++)
			if(phone->tracks[i] != FALSE)
				return;
		g_source_remove(phone->tr_source);
		phone->tr_source = 0;
	}
}


/* callbacks */
/* phone_gsm_event */
static int _gsm_event_error(Phone * phone, GSMEvent * event);

static int _phone_gsm_event(GSMEvent * event, gpointer data)
{
	Phone * phone = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d)\n", __func__, event->type);
#endif
	switch(event->type)
	{
		case GSM_EVENT_TYPE_ERROR:
			return _gsm_event_error(phone, event);
		case GSM_EVENT_TYPE_CONTACT:
			phone_contacts_add(phone, event->contact.index,
					event->contact.name,
					event->contact.number);
			return 0;
		case GSM_EVENT_TYPE_CONTACT_LIST:
			_phone_fetch_contacts(phone, event->contact_list.start,
					event->contact_list.end);
			return 0;
		case GSM_EVENT_TYPE_MESSAGE_LIST:
			_phone_fetch_messages(phone, event->message_list.start,
					event->message_list.end);
			return 0;
		case GSM_EVENT_TYPE_MESSAGE_SENT:
			_phone_track(phone, PHONE_TRACK_MESSAGE_SENT, FALSE);
			phone->wr_progress = _phone_progress_delete(
					phone->wr_progress);
			_phone_info(phone->wr_window, _("Message sent"));
			return 0;
		case GSM_EVENT_TYPE_OPERATOR:
			_phone_set_operator(phone, event->operator.operator);
			return 0;
		case GSM_EVENT_TYPE_REGISTRATION:
			return 0; /* we also get a status update about it */
		case GSM_EVENT_TYPE_SIGNAL_LEVEL:
			_phone_set_signal_level(phone,
					event->signal_level.level);
			return 0;
		case GSM_EVENT_TYPE_SIM_PIN_VALID:
			_phone_track(phone, PHONE_TRACK_CODE_ENTERED, FALSE);
			phone->en_progress = _phone_progress_delete(
					phone->en_progress);
			_phone_info(phone->en_window, _("SIM PIN is valid"));
			return 0;
		case GSM_EVENT_TYPE_STATUS:
			_phone_set_status(phone, event->status.status);
			return 0;
	}
	return 1;
}

static int _gsm_event_error(Phone * phone, GSMEvent * event)
{
	if(event->error.error == GSM_ERROR_SIM_PIN_REQUIRED)
	{
		phone_code_clear(phone);
		phone_show_code(phone, TRUE, PHONE_CODE_SIM_PIN);
	}
	else if(event->error.error == GSM_ERROR_SIM_PIN_WRONG)
	{
		_phone_track(phone, PHONE_TRACK_CODE_ENTERED, FALSE);
		phone->en_progress = _phone_progress_delete(phone->en_progress);
		_phone_error(phone->en_window, _("Wrong SIM PIN code"));
	}
	else if(event->error.error == GSM_ERROR_CONTACT_FETCH_FAILED
			|| event->error.error == GSM_ERROR_MESSAGE_FETCH_FAILED)
		return 0; /* we can ignore this error */
	else if(event->error.error == GSM_ERROR_CONTACT_LIST_FAILED)
		_phone_track(phone, PHONE_TRACK_CONTACT_LIST, TRUE);
	else if(event->error.error == GSM_ERROR_MESSAGE_LIST_FAILED)
		_phone_track(phone, PHONE_TRACK_MESSAGE_LIST, TRUE);
	else if(event->error.error == GSM_ERROR_MESSAGE_SEND_FAILED)
	{
		_phone_track(phone, PHONE_TRACK_MESSAGE_SENT, FALSE);
		phone->wr_progress = _phone_progress_delete(phone->wr_progress);
		_phone_error(phone->wr_window, _("Could not send message"));
	}
	else
		phone_error(phone, event->error.message, 0);
	return 0;
}


/* phone_timeout_track */
static gboolean _phone_timeout_track(gpointer data)
{
	Phone * phone = data;

	if(phone->tracks[PHONE_TRACK_CODE_ENTERED])
		_phone_progress_pulse(phone->en_progress);
	if(phone->tracks[PHONE_TRACK_CONTACT_LIST])
	{
		_phone_track(phone, PHONE_TRACK_CONTACT_LIST, FALSE);
		gsm_fetch_contact_list(phone->gsm);
	}
	if(phone->tracks[PHONE_TRACK_MESSAGE_LIST])
	{
		_phone_track(phone, PHONE_TRACK_MESSAGE_LIST, FALSE);
		gsm_fetch_message_list(phone->gsm);
	}
	if(phone->tracks[PHONE_TRACK_MESSAGE_SENT])
		_phone_progress_pulse(phone->wr_progress);
	if(phone->tracks[PHONE_TRACK_REGISTRATION])
		gsm_is_registered(phone->gsm);
	if(phone->tracks[PHONE_TRACK_SIGNAL_LEVEL])
		gsm_fetch_signal_level(phone->gsm);
	return TRUE;
}
