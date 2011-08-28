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
#include <Desktop.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <gtk/gtk.h>
#include "Phone.h"
#include "../../config.h"


/* Systray */
/* private */
/* types */
typedef struct _Systray
{
	GtkStatusIcon * icon;
	GtkWidget * ab_window;
} Systray;


/* prototypes */
/* plug-in */
static int _systray_init(PhonePlugin * plugin);
static int _systray_destroy(PhonePlugin * plugin);

/* callbacks */
#if GTK_CHECK_VERSION(2, 10, 0)
static void _systray_on_activate(gpointer data);
static void _systray_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data);
#endif


/* public */
/* variables */
PhonePlugin plugin =
{
	NULL,
	"System tray",
	"gnome-monitor",
	_systray_init,
	_systray_destroy,
	NULL,
	NULL,
	NULL
};


/* private */
/* functions */
/* systray_init */
static int _systray_init(PhonePlugin * plugin)
{
	Systray * systray;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
#if GTK_CHECK_VERSION(2, 10, 0)
	if((systray = object_new(sizeof(*systray))) == NULL)
		return 1;
	plugin->priv = systray;
	systray->icon = gtk_status_icon_new_from_icon_name("phone-dialer");
	g_signal_connect_swapped(systray->icon, "activate", G_CALLBACK(
				_systray_on_activate), plugin);
	g_signal_connect(systray->icon, "popup-menu", G_CALLBACK(
				_systray_on_popup_menu), plugin);
	systray->ab_window = NULL;
	return 0;
#else
	return 1;
#endif
}


/* systray_destroy */
static int _systray_destroy(PhonePlugin * plugin)
{
	Systray * systray = plugin->priv;

	g_object_unref(systray->icon);
	return 0;
}


#if GTK_CHECK_VERSION(2, 10, 0)
/* callbacks */
/* systray_on_activate */
static void _systray_on_activate(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->about_dialog(plugin->helper->phone);
}


/* systray_on_popup_menu */
static void _popup_menu_on_show_contacts(gpointer data);
static void _popup_menu_on_show_dialer(gpointer data);
static void _popup_menu_on_show_logs(gpointer data);
static void _popup_menu_on_show_messages(gpointer data);
static void _popup_menu_on_show_write(gpointer data);
static void _popup_menu_on_show_settings(gpointer data);
static void _popup_menu_on_resume(gpointer data);
static void _popup_menu_on_suspend(gpointer data);
static void _popup_menu_on_quit(gpointer data);

static void _systray_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data)
{
	PhonePlugin * plugin = data;
	GtkWidget * menu;
	GtkWidget * menuitem;
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * label;
	struct
	{
		char const * icon;
		char const * name;
		void (*callback)(gpointer data);
	} items[] = {
		{ "stock_addressbook", "Show _contacts",
			_popup_menu_on_show_contacts },
		{ "phone-dialer", "Show _dialer", _popup_menu_on_show_dialer },
		{ "logviewer", "Show _logs", _popup_menu_on_show_logs },
		{ "stock_mail-compose", "Show _messages",
			_popup_menu_on_show_messages },
		{ "stock_mail-compose", "_Write a message",
			_popup_menu_on_show_write },
		{ NULL, NULL, NULL },
		{ "gtk-preferences", "_Preferences",
			_popup_menu_on_show_settings },
		{ NULL, NULL, NULL },
		{ "gtk-media-play-ltr", "_Resume telephony",
			_popup_menu_on_resume },
		{ "gtk-media-pause", "S_uspend telephony",
			_popup_menu_on_suspend },
		{ NULL, NULL, NULL },
		{ "gtk-quit", "_Quit", _popup_menu_on_quit },
	};
	size_t i;

	menu = gtk_menu_new();
	for(i = 0; i < sizeof(items) / sizeof(*items); i++)
	{
		if(items[i].name == NULL)
		{
			menuitem = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
			continue;
		}
		hbox = gtk_hbox_new(FALSE, 4);
		image = gtk_image_new_from_icon_name(items[i].icon,
				GTK_ICON_SIZE_MENU),
		gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, TRUE, 0);
		label = gtk_label_new_with_mnemonic(items[i].name);
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
		menuitem = gtk_menu_item_new();
		g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
					items[i].callback), plugin);
		gtk_container_add(GTK_CONTAINER(menuitem), hbox);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, time);
}

static void _popup_menu_on_show_contacts(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_CONTACTS);
}

static void _popup_menu_on_show_dialer(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_DIALER);
}

static void _popup_menu_on_show_logs(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_LOGS);
}

static void _popup_menu_on_show_messages(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_MESSAGES);
}

static void _popup_menu_on_show_settings(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_SETTINGS);
}

static void _popup_menu_on_show_write(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone, PHONE_MESSAGE_SHOW,
			PHONE_MESSAGE_SHOW_WRITE);
}

static void _popup_menu_on_resume(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone,
			PHONE_MESSAGE_POWER_MANAGEMENT,
			PHONE_MESSAGE_POWER_MANAGEMENT_RESUME);
}

static void _popup_menu_on_suspend(gpointer data)
{
	PhonePlugin * plugin = data;

	plugin->helper->message(plugin->helper->phone,
			PHONE_MESSAGE_POWER_MANAGEMENT,
			PHONE_MESSAGE_POWER_MANAGEMENT_SUSPEND);
}

static void _popup_menu_on_quit(gpointer data)
{
	gtk_main_quit();
}
#endif
