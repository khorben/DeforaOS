/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
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
#include "common.h"


/* common_new_menubar */
GtkWidget * common_new_menubar(GtkWindow * window, struct _menubar * mb,
		gpointer data)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	GtkWidget * image;
	size_t i;
	size_t j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
	group = gtk_accel_group_new();
	for(i = 0; mb[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(mb[i].name);
		menu = gtk_menu_new();
		for(j = 0; mb[i].menu[j].name != NULL; j++)
		{
			p = &mb[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == NULL)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else if(strncmp(p->stock, "gtk-", 4) == 0)
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			else
			{
				image = gtk_image_new_from_icon_name(p->stock,
						GTK_ICON_SIZE_MENU);
				menuitem
					= gtk_image_menu_item_new_with_mnemonic(
						p->name);
				gtk_image_menu_item_set_image(
						GTK_IMAGE_MENU_ITEM(menuitem),
						image);
			}
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback), data);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			if(p->accel != 0)
				gtk_widget_add_accelerator(menuitem, "activate",
						group, p->accel,
						GDK_CONTROL_MASK,
						GTK_ACCEL_VISIBLE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	gtk_window_add_accel_group(window, group);
	return tb_menubar;
}


/* common_new_toolbar */
GtkWidget * common_new_toolbar(struct _toolbar * tb, gpointer data)
{
	GtkWidget * toolbar;
	size_t i;
	GtkWidget * widget;
	GtkToolItem * toolitem;

	toolbar = gtk_toolbar_new();
	for(i = 0; tb[i].name != NULL; i++)
	{
		if(tb[i].name[0] == '\0')
		{
			toolitem = gtk_separator_tool_item_new();
			gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
			continue;
		}
		widget = gtk_image_new_from_icon_name(tb[i].stock,
				GTK_ICON_SIZE_LARGE_TOOLBAR);
		toolitem = gtk_tool_button_new(widget, tb[i].name);
		g_signal_connect(toolitem, "clicked", tb[i].callback, data);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	}
	return toolbar;
}
