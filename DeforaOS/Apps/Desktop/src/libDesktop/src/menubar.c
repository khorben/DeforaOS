/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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
#include <libintl.h>
#include "Desktop.h"
#define _(string) gettext(string)


/* Menubar */
/* desktop_menubar_create */
static GtkWidget * _menubar_create_menu(DesktopMenu const * menu, gpointer data,
		GtkAccelGroup * accel);
static GtkWidget * _menubar_create_menu_from_image(DesktopMenu const * menu);

GtkWidget * desktop_menubar_create(DesktopMenubar const * menubar,
		gpointer data, GtkAccelGroup * accel)
{
	GtkWidget * ret;
	GtkWidget * menuitem;
	GtkWidget * menu;
	size_t i;

	ret = gtk_menu_bar_new();
	for(i = 0; menubar[i].name != NULL; i++)
	{
		menuitem = gtk_menu_item_new_with_mnemonic(_(menubar[i].name));
		menu = _menubar_create_menu(menubar[i].menu, data, accel);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), menu);
		gtk_menu_shell_append(GTK_MENU_SHELL(ret), menuitem);
	}
	return ret;
}

static GtkWidget * _menubar_create_menu(DesktopMenu const * menu, gpointer data,
		GtkAccelGroup * accel)
{
	GtkWidget * ret;
	size_t i;
	DesktopMenu const * p;
	GtkWidget * menuitem;

	ret = gtk_menu_new();
	for(i = 0; menu[i].name != NULL; i++)
	{
		p = &menu[i];
		if(p->name[0] == '\0')
			menuitem = gtk_separator_menu_item_new();
		else if(p->stock == NULL)
			menuitem = gtk_menu_item_new_with_mnemonic(_(p->name));
		else if(strncmp(p->stock, "gtk-", 4) == 0)
			menuitem = gtk_image_menu_item_new_from_stock(p->stock,
					NULL);
		else
			menuitem = _menubar_create_menu_from_image(p);
		if(p->callback != NULL)
			g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
					G_CALLBACK(p->callback), data);
		else
			gtk_widget_set_sensitive(menuitem, FALSE);
		if(accel != NULL && p->accel != 0)
			gtk_widget_add_accelerator(menuitem, "activate", accel,
					p->accel, p->modifier,
					GTK_ACCEL_VISIBLE);
		gtk_menu_shell_append(GTK_MENU_SHELL(ret), menuitem);
	}
	return ret;
}

static GtkWidget * _menubar_create_menu_from_image(DesktopMenu const * menu)
{
	GtkWidget * ret;
	GtkWidget * image;

	ret = gtk_image_menu_item_new_with_mnemonic(_(menu->name));
	image = gtk_image_new_from_icon_name(menu->stock, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ret), image);
	return ret;
}
