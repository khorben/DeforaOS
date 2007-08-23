/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mailer */
/* Mailer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Mailer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Mailer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include "common.h"


/* common_new_menubar */
GtkWidget * common_new_menubar(struct _menubar * mb, gpointer data)
{
	GtkWidget * tb_menubar;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
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
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback), data);
			else
				gtk_widget_set_sensitive(menuitem, FALSE);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	return tb_menubar;
}


/* common_new_toolbar */
GtkWidget * common_new_toolbar(struct _toolbar * tb, gpointer data)
{
	GtkWidget * toolbar;
	GtkIconTheme * theme;
	unsigned int i;
	GtkWidget * widget;
	GtkToolItem * toolitem;

	toolbar = gtk_toolbar_new();
	theme = gtk_icon_theme_get_default();
	for(i = 0; tb[i].name != NULL; i++)
	{
		if(tb[i].name[0] == '\0')
		{
			toolitem = gtk_separator_tool_item_new();
			gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
			continue;
		}
		widget = gtk_image_new_from_pixbuf(gtk_icon_theme_load_icon(
					theme, tb[i].stock, 32, 0, NULL));
		toolitem = gtk_tool_button_new(widget, tb[i].name);
		g_signal_connect(toolitem, "clicked", tb[i].callback, data);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	}
	return toolbar;
}
