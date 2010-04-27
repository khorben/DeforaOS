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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "gsm.h"
#include "callbacks.h"
#include "phone.h"
#define _(string) gettext(string)


/* Phone */
/* private */
/* types */
struct _Phone
{
	GSM * gsm;
	guint source;

	/* widgets */
	PangoFontDescription * bold;

	/* contacts */
	GtkWidget * co_window;

	/* dialer */
	GtkWidget * di_window;
	GtkWidget * di_entry;

	/* messages */
	GtkWidget * me_window;
};


/* prototypes */
static GtkWidget * _phone_create_dialpad(Phone * phone);


/* public */
/* functions */
/* phone_new */
static gboolean _new_idle(gpointer data);

Phone * phone_new(char const * device, unsigned int baudrate)
{
	Phone * phone;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %u)\n", __func__, device, baudrate);
#endif
	if((phone = malloc(sizeof(*phone))) == NULL)
		return NULL;
	if(device == NULL)
		device = "/dev/modem";
	phone->gsm = gsm_new(device, baudrate);
	phone->source = g_idle_add(_new_idle, phone);
	/* widgets */
	phone->bold = pango_font_description_new();
	pango_font_description_set_weight(phone->bold, PANGO_WEIGHT_BOLD);
	phone->di_window = NULL;
	phone->co_window = NULL;
	phone->me_window = NULL;
	/* check errors */
	if(phone->gsm == NULL)
	{
		phone_delete(phone);
		return NULL;
	}
	return phone;
}

static gboolean _new_idle(gpointer data)
{
	Phone * phone = data;

	phone_show_contacts(phone, FALSE);
	phone_show_dialer(phone, TRUE);
	phone_show_messages(phone, FALSE);
	gsm_report_contacts(phone->gsm);
	phone->source = 0;
	return FALSE;
}


/* phone_delete */
void phone_delete(Phone * phone)
{
	if(phone->source != 0)
		g_source_remove(phone->source);
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
	/* FIXME implement */
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs("phone: ", stderr);
	perror(message);
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


/* phone_dialer_append */
void phone_dialer_append(Phone * phone, char character)
{
	char const * text;
	size_t len;
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%c)\n", __func__, character);
#endif
	if((character < '0' || character > '9') && character != '*'
			&& character != '+' && character != '#')
		return;
	/* FIXME ask GSM if in a call; if yes, send DTMF */
	text = gtk_entry_get_text(GTK_ENTRY(phone->di_entry));
	len = strlen(text);
	if((p = malloc(len + 2)) == NULL)
		return; /* XXX report error */
	snprintf(p, len + 2, "%s%c", text, character);
	gtk_entry_set_text(GTK_ENTRY(phone->di_entry), p);
	free(p);
}


/* phone_hangup */
void phone_hangup(Phone * phone)
{
	gsm_hangup(phone->gsm);
	gtk_entry_set_text(GTK_ENTRY(phone->di_entry), "");
}


/* phone_show_contacts */
void phone_show_contacts(Phone * phone, gboolean show)
{
	if(phone->co_window == NULL)
	{
		phone->co_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(phone->co_window),
				_("Contacts"));
		g_signal_connect_swapped(G_OBJECT(phone->co_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->co_window);
		/* FIXME implement */
	}
	if(show)
		gtk_widget_show(phone->co_window); /* XXX force focus? */
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
		gtk_window_set_title(GTK_WINDOW(phone->di_window), _("Dialer"));
		g_signal_connect_swapped(G_OBJECT(phone->di_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->di_window);
		vbox = gtk_vbox_new(FALSE, 0);
		/* entry */
		hbox = gtk_hbox_new(FALSE, 0);
		phone->di_entry = gtk_entry_new();
		gtk_widget_modify_font(phone->di_entry, phone->bold);
		g_signal_connect_swapped(G_OBJECT(phone->di_entry), "activate",
				G_CALLBACK(on_phone_dialer_call), phone);
		gtk_box_pack_start(GTK_BOX(hbox), phone->di_entry, TRUE, TRUE,
				0);
		widget = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(widget),
				gtk_image_new_from_icon_name(
					"stock_addressbook",
					GTK_ICON_SIZE_BUTTON));
		gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
		g_signal_connect_swapped(G_OBJECT(widget), "clicked",
				G_CALLBACK(on_phone_contacts_show), phone);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE,
				0);
		/* dialpad */
		widget = _phone_create_dialpad(phone);
		gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
		gtk_container_add(GTK_CONTAINER(phone->di_window), vbox);
		gtk_widget_show_all(vbox);
	}
	if(show)
		gtk_widget_show(phone->di_window); /* XXX force focus? */
	else
		gtk_widget_hide(phone->di_window);
}


/* phone_show_messages */
void phone_show_messages(Phone * phone, gboolean show)
{
	if(phone->me_window == NULL)
	{
		phone->me_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(phone->me_window),
				_("Messages"));
		g_signal_connect_swapped(G_OBJECT(phone->me_window),
				"delete-event", G_CALLBACK(on_phone_closex),
				phone->me_window);
		/* FIXME implement */
	}
	if(show)
		gtk_widget_show(phone->me_window); /* XXX force focus? */
	else
		gtk_widget_hide(phone->me_window);
}


/* private */
/* phone_create_dialpad */
static GtkWidget * _phone_create_dialpad(Phone * phone)
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
	image = gtk_image_new_from_icon_name("call-start",
			GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), _("Call"));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(
			on_phone_dialer_call), phone);
	gtk_table_attach(GTK_TABLE(table), button, 0, 3, 0, 1,
			GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 2, 2);
	button = gtk_button_new();
	image = gtk_image_new_from_icon_name("call-stop", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_label(GTK_BUTTON(button), _("Hang up"));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(
				on_phone_dialer_hangup), phone);
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
					on_phone_dialer_clicked), phone);
		gtk_table_attach(GTK_TABLE(table), button, (i % 3) * 2,
				((i % 3) + 1) * 2, (i / 3) + 1, (i / 3) + 2,
				GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
				2, 2);
	}
	return table;
}
