/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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
#include "ghtml.h"
#include "surfer.h"


/* Surfer */
/* types */
#ifndef FOR_EMBEDDED
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
	{ "_New window",	G_CALLBACK(on_file_new_window), "window-new",
		GDK_N },
	{ "_Open...",		G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_O },
	{ "", NULL, NULL, 0 },
	{ "_Close",		G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_W },
	{ NULL,			NULL, NULL, 0 }
};

static struct _menu _menu_edit[] =
{
	{ "_Cut",		NULL, GTK_STOCK_CUT, GDK_X },
	{ "Cop_y",		NULL, GTK_STOCK_COPY, GDK_C },
	{ "_Paste",		NULL, GTK_STOCK_PASTE, GDK_V },
	{ "",			NULL, NULL, 0 },
	{ "Select _all",	G_CALLBACK(on_edit_select_all),
		GTK_STOCK_SELECT_ALL, GDK_A },
	{ "Unselect all",	G_CALLBACK(on_edit_unselect_all), NULL, 0 },
	{ "",			NULL, NULL, 0 },
	{ "_Preferences",	G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL,			NULL, NULL, 0 }
};

static struct _menu _menu_view[] =
{
	{ "Zoom in",		G_CALLBACK(on_view_zoom_in), "zoom-in",
		GDK_plus },
	{ "Zoom out",		G_CALLBACK(on_view_zoom_out), "zoom-out",
		GDK_minus },
	{ "Normal size",	G_CALLBACK(on_view_normal_size), "zoom-1",
		GDK_0 },
	{ "",			NULL, NULL, 0 },
	{ "_Refresh",		G_CALLBACK(on_view_refresh), GTK_STOCK_REFRESH,
		GDK_R },
	{ "_Force refresh",	G_CALLBACK(on_view_force_refresh), NULL, 0 },
	{ "_Stop",		G_CALLBACK(on_view_stop), GTK_STOCK_STOP, 0 },
	{ "",			NULL, NULL, 0 },
	{ "Page so_urce",	G_CALLBACK(on_view_page_source),
		"stock_view-html-source", GDK_U },
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
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};
#endif /* !FOR_EMBEDDED */

unsigned int surfer_cnt = 0;

/* functions */
#ifndef FOR_EMBEDDED
static GtkWidget * _new_menubar(Surfer * surfer);
#endif

Surfer * surfer_new(char const * url)
{
	Surfer * surfer;
	GtkWidget * vbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GtkWidget * widget;
	GtkWidget * hbox;

	if((surfer = malloc(sizeof(*surfer))) == NULL)
		return NULL;
	/* window */
	surfer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(surfer->window), 800, 600);
	surfer_set_title(surfer, NULL);
	g_signal_connect(G_OBJECT(surfer->window), "delete_event", G_CALLBACK(
				on_closex), surfer);
	vbox = gtk_vbox_new(FALSE, 0);
#ifndef FOR_EMBEDDED
	/* menubar */
	surfer->menubar = _new_menubar(surfer);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->menubar, FALSE, FALSE, 0);
#endif
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
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), surfer->tb_stop, -1);
	surfer->tb_refresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
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
	if((surfer->view = ghtml_new(surfer)) == NULL)
	{
		/* XXX display an error dialog */
		surfer_delete(surfer);
		return NULL;
	}
	if(url != NULL)
		ghtml_load_url(surfer->view, url);
	gtk_box_pack_start(GTK_BOX(vbox), surfer->view, TRUE, TRUE, 0);
	/* statusbar */
	hbox = gtk_hbox_new(FALSE, 0);
	surfer->progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(hbox), surfer->progress, FALSE, FALSE, 0);
	surfer->statusbar = gtk_statusbar_new();
	surfer->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				surfer->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					surfer->statusbar), ""), "Ready");
	gtk_box_pack_start(GTK_BOX(hbox), surfer->statusbar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(surfer->window), vbox);
	gtk_widget_grab_focus(GTK_WIDGET(surfer->tb_path));
	gtk_widget_show_all(surfer->window);
	surfer_cnt++;
	return surfer;
}

#ifndef FOR_EMBEDDED
static GtkWidget * _new_menubar(Surfer * surfer)
{
	GtkWidget * tb_menubar;
	GtkAccelGroup * group;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	GtkWidget * image;
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
			else if(strncmp(p->stock, "gtk-", 4) == 0)
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			else
			{
				image = gtk_image_new_from_icon_name(p->stock,
						GTK_ICON_SIZE_MENU);
				menuitem =
					gtk_image_menu_item_new_with_mnemonic(
							p->name);
				gtk_image_menu_item_set_image(
						GTK_IMAGE_MENU_ITEM(menuitem),
						image);
			}
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
	gtk_window_add_accel_group(GTK_WINDOW(surfer->window), group);
	return tb_menubar;
}
#endif /* !FOR_EMBEDDED */


/* surfer_delete */
void surfer_delete(Surfer * surfer)
{
	/* config_delete(surfer->config); */
	free(surfer);
	surfer_cnt--;
}


/* accessors */
/* surfer_set_progress */
void surfer_set_progress(Surfer * surfer, gdouble fraction)
{
	char buf[10];

	snprintf(buf, sizeof(buf), "%.1f%%", fraction * 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(surfer->progress), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(surfer->progress),
			fraction);
}


/* surfer_set_status */
void surfer_set_status(Surfer * surfer, char const * status)
{
	GtkStatusbar * sb;

	sb = GTK_STATUSBAR(surfer->statusbar);
	if(surfer->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				surfer->statusbar_id);
	surfer->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* surfer_set_title */
void surfer_set_title(Surfer * surfer, char const * title)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s%s", "Web surfer", title != NULL
			? " - " : "", title != NULL ? title : "");
	gtk_window_set_title(GTK_WINDOW(surfer->window), buf);
}


/* useful */
/* surfer_error */
int surfer_error(Surfer * surfer, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(surfer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
				gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}


/* surfer_refresh */
void surfer_refresh(Surfer * surfer)
{
	ghtml_refresh(surfer->view);
}


/* surfer_reload */
void surfer_reload(Surfer * surfer)
{
	ghtml_reload(surfer->view);
}


/* surfer_select_all */
void surfer_select_all(Surfer * surfer)
{
	ghtml_select_all(surfer->view);
}


/* surfer_stop */
void surfer_stop(Surfer * surfer)
{
	ghtml_stop(surfer->view);
}


/* surfer_unselect_all */
void surfer_unselect_all(Surfer * surfer)
{
	ghtml_unselect_all(surfer->view);
}


/* surfer_zoom_in */
void surfer_zoom_in(Surfer * surfer)
{
	ghtml_zoom_in(surfer->view);
}


/* surfer_zoom_out */
void surfer_zoom_out(Surfer * surfer)
{
	ghtml_zoom_out(surfer->view);
}


/* surfer_zoom_reset */
void surfer_zoom_reset(Surfer * surfer)
{
	ghtml_zoom_reset(surfer->view);
}
