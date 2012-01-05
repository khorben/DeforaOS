/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Locker */
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
#ifdef DEBUG
# include <stdio.h>
#endif
#include <gtk/gtk.h>
#include "Locker.h"
#include "../../config.h"


/* Systray */
/* private */
/* types */
typedef struct _LockerPlugin
{
	LockerPluginHelper * helper;
	GtkStatusIcon * icon;
	GtkWidget * ab_window;
} Systray;


/* prototypes */
/* plug-in */
static Systray * _systray_init(LockerPluginHelper * helper);
static void _systray_destroy(Systray * systray);

/* callbacks */
#if GTK_CHECK_VERSION(2, 10, 0)
static void _systray_on_activate(gpointer data);
static void _systray_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data);
#endif


/* public */
/* variables */
LockerPluginDefinition plugin =
{
	"System tray",
	"gnome-monitor",
	NULL,
	_systray_init,
	_systray_destroy,
	NULL
};


/* private */
/* functions */
/* systray_init */
static Systray * _systray_init(LockerPluginHelper * helper)
{
	Systray * systray;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
#if GTK_CHECK_VERSION(2, 10, 0)
	if((systray = object_new(sizeof(*systray))) == NULL)
		return NULL;
	systray->helper = helper;
	systray->icon = gtk_status_icon_new_from_icon_name(
			"preferences-desktop-screensaver");
#if GTK_CHECK_VERSION(2, 16, 0)
	gtk_status_icon_set_tooltip_text(systray->icon, "Screensaver");
#endif
	g_signal_connect_swapped(systray->icon, "activate", G_CALLBACK(
				_systray_on_activate), systray);
	g_signal_connect(systray->icon, "popup-menu", G_CALLBACK(
				_systray_on_popup_menu), systray);
	systray->ab_window = NULL;
	return systray;
#else
	return 1;
#endif
}


/* systray_destroy */
static void _systray_destroy(Systray * systray)
{
	g_object_unref(systray->icon);
	object_delete(systray);
}


#if GTK_CHECK_VERSION(2, 10, 0)
/* callbacks */
/* systray_on_activate */
static void _systray_on_activate(gpointer data)
{
	Systray * systray = data;

	systray->helper->about_dialog(systray->helper->locker);
}


/* systray_on_popup_menu */
static void _popup_menu_on_lock(gpointer data);
static void _popup_menu_on_quit(gpointer data);
static void _popup_menu_on_show_settings(gpointer data);

static void _systray_on_popup_menu(GtkStatusIcon * icon, guint button,
		guint time, gpointer data)
{
	Systray * systray = data;
	GtkWidget * menu;
	GtkWidget * menuitem;
	GtkWidget * image;
	struct
	{
		char const * icon;
		char const * name;
		void (*callback)(gpointer data);
	} items[] = {
		{ "gnome-lockscreen", "_Lock screen",
			_popup_menu_on_lock },
		{ NULL, NULL, NULL },
		{ "gtk-preferences", "_Preferences",
			_popup_menu_on_show_settings },
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
		menuitem = gtk_image_menu_item_new_with_mnemonic(items[i].name);
		image = gtk_image_new_from_icon_name(items[i].icon,
				GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
				image);
		g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(
					items[i].callback), systray);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, time);
}

static void _popup_menu_on_lock(gpointer data)
{
	Systray * systray = data;

	systray->helper->action(systray->helper->locker, LOCKER_ACTION_LOCK);
}

static void _popup_menu_on_quit(gpointer data)
{
	gtk_main_quit();
}

static void _popup_menu_on_show_settings(gpointer data)
{
	Systray * systray = data;

	systray->helper->action(systray->helper->locker,
			LOCKER_ACTION_SHOW_PREFERENCES);
}
#endif
