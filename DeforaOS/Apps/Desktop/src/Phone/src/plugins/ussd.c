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



#include <string.h>
#include <gtk/gtk.h>
#include <System.h>
#include "Phone.h"


/* USSD */
/* private */
/* types */
typedef struct _GPRS
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * operators;
	GtkWidget * codes;
} USSD;

typedef struct _USSDCode
{
	char const * number;
	char const * name;
} USSDCode;


/* constants */
/* Germany */
/* E-plus, see http://www.prepaid-wiki.de/index.php5/E-Plus */
static USSDCode _ussd_codes_de_eplus[] =
{
	{ "*100#",	"Balance enquiry"				},
	{ NULL,		NULL						}
};

/* MTN, see http://www.mtn.co.za/Support/faq/Pages/USSD.aspx */
static USSDCode _ussd_codes_za_mtn[] =
{
	{ "*141#",	"Balance enquiry"				},
	{ "*141*4#",	"Tariff Analyser and Priceplan Migrations menu"	},
	{ "*141*4*0#",	"Tariff Analyser"				},
	{ "*141*6*0#",	"Data bundle cancellation"			},
	{ "*141*7*0#",	"SMS bundle cancellation"			},
	{ "*141*8#",	"Yello Fortune Entries"				},
	{ NULL,		NULL						}
};

/* Virgin Mobile, see
 * http://fr.wikipedia.org/wiki/Unstructured_Supplementary_Service_Data */
static USSDCode _ussd_codes_fr_virgin[] =
{
	{ "*144#",	"Balance enquiry"				},
	{ NULL,		NULL						}
};

static const struct
{
	char const * name;
	USSDCode * codes;
} _ussd_operators[] =
{
	{ "E-plus",	_ussd_codes_de_eplus				},
	{ "Monacell",	_ussd_codes_fr_virgin				},
	{ "MTN",	_ussd_codes_za_mtn				},
	{ "NRJ",	_ussd_codes_fr_virgin				},
	{ "Virgin",	_ussd_codes_fr_virgin				},
	{ NULL,		NULL						}
};


/* prototypes */
/* plugins */
static int _ussd_init(PhonePlugin * plugin);
static void _ussd_destroy(PhonePlugin * plugin);
static int _ussd_event(PhonePlugin * plugin, PhoneEvent * event);
static void _ussd_settings(PhonePlugin * plugin);

/* callbacks */
static void _ussd_on_operators_changed(gpointer data);
static void _ussd_on_settings_close(gpointer data);
static void _ussd_on_settings_send(gpointer data);


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"USSD",
	NULL,
	_ussd_init,
	_ussd_destroy,
	_ussd_event,
	_ussd_settings,
	NULL
};


/* private */
/* functions */
/* ussd_init */
static int _ussd_init(PhonePlugin * plugin)
{
	USSD * ussd;

	if((ussd = object_new(sizeof(*ussd))) == NULL)
		return -1;
	plugin->priv = ussd;
	ussd->window = NULL;
	ussd->operators = NULL;
	ussd->codes = NULL;
	return 0;
}


/* ussd_destroy */
static void _ussd_destroy(PhonePlugin * plugin)
{
	USSD * ussd = plugin->priv;

	if(ussd->window != NULL)
		gtk_widget_destroy(ussd->window);
	object_delete(ussd);
}


/* ussd_event */
static int _ussd_event(PhonePlugin * plugin, PhoneEvent * event)
{
	return 0;
}


/* ussd_settings */
static void _settings_window(PhonePlugin * plugin);

static void _ussd_settings(PhonePlugin * plugin)
{
	USSD * ussd = plugin->priv;

	if(ussd->window == NULL)
		_settings_window(plugin);
	gtk_window_present(GTK_WINDOW(ussd->window));
}

static void _settings_window(PhonePlugin * plugin)
{
	USSD * ussd = plugin->priv;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * widget;
	size_t i;

	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	ussd->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(ussd->window), 4);
	gtk_window_set_default_size(GTK_WINDOW(ussd->window), 200, 300);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(ussd->window), "gnome-settings");
#endif
	gtk_window_set_title(GTK_WINDOW(ussd->window), "USSD");
	g_signal_connect(ussd->window, "delete-event", G_CALLBACK(
				gtk_widget_hide), NULL);
	vbox = gtk_vbox_new(FALSE, 4);
	/* operators */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Operator:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	ussd->operators = gtk_combo_box_new_text();
	for(i = 0; _ussd_operators[i].name != NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(ussd->operators),
				_ussd_operators[i].name);
	g_signal_connect_swapped(ussd->operators, "changed", G_CALLBACK(
				_ussd_on_operators_changed), ussd);
	gtk_box_pack_start(GTK_BOX(hbox), ussd->operators, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* codes */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new("Code:");
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	ussd->codes = gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(hbox), ussd->codes, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* send */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_with_label("Send request");
	image = gtk_image_new_from_icon_name("mail-send", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(widget), image);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_ussd_on_settings_send), plugin);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* button box */
	hbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(hbox), 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_ussd_on_settings_close), ussd);
	gtk_container_add(GTK_CONTAINER(hbox), widget);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(ussd->window), vbox);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ussd->operators), 0);
	gtk_widget_show_all(vbox);
}


/* callbacks */
/* ussd_on_operators_changed */
static void _ussd_on_operators_changed(gpointer data)
{
	USSD * ussd = data;
	GtkTreeModel * model;
	int i;
	USSDCode * codes;

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(ussd->codes));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	i = gtk_combo_box_get_active(GTK_COMBO_BOX(ussd->operators));
	codes = _ussd_operators[i].codes;
	for(i = 0; codes[i].name != NULL; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(ussd->codes),
				codes[i].name);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ussd->codes), 0);
}


/* ussd_on_settings_close */
static void _ussd_on_settings_close(gpointer data)
{
	USSD * ussd = data;

	gtk_widget_hide(ussd->window);
}


/* ussd_on_settings_send */
static void _ussd_on_settings_send(gpointer data)
{
	PhonePlugin * plugin = data;
	PhonePluginHelper * helper = plugin->helper;
	USSD * ussd = plugin->priv;
	int i;
	USSDCode * codes;
	ModemRequest request;

	i = gtk_combo_box_get_active(GTK_COMBO_BOX(ussd->operators));
	codes = _ussd_operators[i].codes;
	i = gtk_combo_box_get_active(GTK_COMBO_BOX(ussd->codes));
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, codes[i].number);
#endif
	memset(&request, 0, sizeof(request));
	request.type = MODEM_REQUEST_CALL;
	request.call.call_type = MODEM_CALL_TYPE_VOICE;
	request.call.number = codes[i].number;
	helper->request(helper->phone, &request);
}
