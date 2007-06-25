/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "surfer.h"


/* Surfer */
/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
	unsigned int accel; /* FIXME doesn't work */
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};

/* variables */
static struct _menu _menu_file[] =
{
	{ "_New window",	G_CALLBACK(on_file_new_window), NULL, GDK_N },
	{ "",			NULL, NULL, 0 },
	{ "_Refresh",		G_CALLBACK(on_file_refresh), GTK_STOCK_REFRESH,
		GDK_R },
	{ "_Force refresh",	G_CALLBACK(on_file_force_refresh), NULL, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Close",		G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_W },
	{ NULL,			NULL, NULL, 0 }
};

static struct _menu _menu_edit[] =
{
	{ "_Preferences",	G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL,			NULL, NULL, 0 }
};

static struct _menu _menu_help[] =
{
	{ "_About",		G_CALLBACK(on_help_about),
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_ABOUT, 0 },
#else
		NULL, 0 },
#endif
	{ NULL,			NULL, NULL, 0 }
};

static struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};

unsigned int surfer_cnt = 0;

/* functions */
static GtkWidget * _new_menubar(Surfer * surfer);

Surfer * surfer_new(char const * url)
{
	Surfer * surfer;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * widget;
	char buf[256];

	if((surfer = malloc(sizeof(*surfer))) == NULL)
		return NULL;
	/* window */
	surfer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(surfer->window), 800, 600);
	gtk_window_set_title(GTK_WINDOW(surfer->window), "Web surfer");
	g_signal_connect(G_OBJECT(surfer->window), "delete_event", G_CALLBACK(
				on_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	surfer->menubar = _new_menubar(surfer);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->menubar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	surfer->tb_back = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	g_signal_connect(G_OBJECT(surfer->tb_back), "clicked", G_CALLBACK(
				on_back), surfer);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_back), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), surfer->tb_back, -1);
	surfer->tb_forward = gtk_tool_button_new_from_stock(
			GTK_STOCK_GO_FORWARD);
	g_signal_connect(G_OBJECT(surfer->tb_forward), "clicked", G_CALLBACK(
				on_forward), surfer);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_forward), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), surfer->tb_forward, -1);
	surfer->tb_stop = gtk_tool_button_new_from_stock(GTK_STOCK_STOP);
	g_signal_connect(G_OBJECT(surfer->tb_stop), "clicked", G_CALLBACK(
				on_stop), surfer);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_stop), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), surfer->tb_stop, -1);
	surfer->tb_refresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	gtk_widget_set_sensitive(GTK_WIDGET(surfer->tb_refresh), FALSE);
	g_signal_connect(G_OBJECT(surfer->tb_refresh), "clicked", G_CALLBACK(
				on_refresh), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), surfer->tb_refresh, -1);
	toolitem = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(on_home),
			surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#if GTK_CHECK_VERSION(2, 8, 0)
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_FULLSCREEN);
#else
	toolitem = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_ZOOM_FIT);
#endif
	g_signal_connect(G_OBJECT(toolitem), "toggled", G_CALLBACK(
				on_fullscreen), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	toolitem = gtk_tool_item_new();
	widget = gtk_label_new(" Location: ");
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_item_new();
	surfer->tb_path = gtk_combo_box_entry_new_text();
	widget = gtk_bin_get_child(GTK_BIN(surfer->tb_path));
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(
				on_path_activate), surfer);
	if(url != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), url);
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), surfer->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_path_activate), surfer);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* view */
	gtk_moz_embed_set_comp_path(SURFER_GTKMOZEMBED_COMPPATH);
	if(buf[0] != '\0')
		gtk_moz_embed_set_profile_path(buf, "gecko");
	if((surfer->view = gtk_moz_embed_new()) == NULL)
	{
		surfer_delete(surfer);
		return NULL;
	}
	g_signal_connect(G_OBJECT(surfer->view), "link_message", G_CALLBACK(
				on_view_link_message), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "location", G_CALLBACK(
				on_view_location), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "net_start", G_CALLBACK(
				on_view_net_start), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "net_stop", G_CALLBACK(
				on_view_net_stop), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "new_window", G_CALLBACK(
				on_view_new_window), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "progress", G_CALLBACK(
				on_view_progress), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "size_to", G_CALLBACK(
				on_view_resize), surfer);
	g_signal_connect(G_OBJECT(surfer->view), "title", G_CALLBACK(
				on_view_title), surfer);
	if(url != NULL)
		gtk_moz_embed_load_url(GTK_MOZ_EMBED(surfer->view), url);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->view, TRUE, TRUE, 0);
	/* statusbar */
	surfer->statusbar = gtk_statusbar_new();
	surfer->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				surfer->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					surfer->statusbar), ""), "Ready");
	gtk_box_pack_start(GTK_BOX(vbox), surfer->statusbar, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(surfer->window), vbox);
	gtk_widget_grab_focus(GTK_WIDGET(surfer->tb_path));
	gtk_widget_show_all(surfer->window);
	surfer_cnt++;
	return surfer;
}

static GtkWidget * _new_menubar(Surfer * surfer)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;
	struct _menu * p;

	tb_menubar = gtk_menu_bar_new();
	group = gtk_accel_group_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			p = &_menubar[i].menu[j];
			if(p->name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(p->stock == 0)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback),
						surfer);
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
	return tb_menubar;
}


/* surfer_delete */
void surfer_delete(Surfer * surfer)
{
	/* config_delete(surfer->config); */
	free(surfer);
	surfer_cnt--;
}


/* useful */
/* surfer_error */
int surfer_error(Surfer * surfer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(surfer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}
