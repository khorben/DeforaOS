/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
/* This file is part of Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <gdk/gdkkeysyms.h>
#include "callbacks.h"
#include "browser.h"


/* constants */
#define IDLE_LOOP_ICON_CNT	16	/* number of icons added in a loop */


/* macros */
#define min(a, b) ((a) > (b) ? (b) : (a))


/* types */
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
	unsigned int accel;
};

struct _menubar
{
	char * name;
	struct _menu * menu;
};


/* constants */
struct _menu _menu_file[] =
{
	{ "_New window", G_CALLBACK(on_file_new_window), NULL, GDK_N },
	{ "", NULL, NULL, 0 },
	{ "_Refresh", G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, GDK_R },
	{ "_Properties", G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Close", G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_edit[] =
{
	{ "_Cut", G_CALLBACK(on_edit_cut), GTK_STOCK_CUT, GDK_X },
	{ "Cop_y", G_CALLBACK(on_edit_copy), GTK_STOCK_COPY, GDK_C },
	{ "_Paste", NULL, GTK_STOCK_PASTE, GDK_V },
	{ "", NULL, NULL, 0 },
	{ "_Delete", G_CALLBACK(on_edit_delete), GTK_STOCK_DELETE, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Select all", G_CALLBACK(on_edit_select_all), NULL, GDK_A },
	{ "_Unselect all", G_CALLBACK(on_edit_unselect_all), NULL, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_view[] =
{
	{ "_Home", G_CALLBACK(on_view_home), GTK_STOCK_HOME, 0 },
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "", NULL, NULL, 0 },
	{ "_Details", G_CALLBACK(on_view_details), NULL, 0 },
	{ "_Icons", G_CALLBACK(on_view_icons), NULL, 0 },
	{ "_List", G_CALLBACK(on_view_list), NULL, 0 },
	{ "_Thumbnails", G_CALLBACK(on_view_thumbnails), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ "_About", G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_View", _menu_view },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};


/* Browser */
/* variables */
unsigned int browser_cnt = 0;

/* browser_new */
static int _new_pixbufs(Browser * browser);
static GtkWidget * _new_menubar(Browser * browser);
static GtkListStore * _create_store(Browser * browser);

Browser * browser_new(char const * directory)
{
	Browser * browser;
	GtkWidget * vbox;
	GtkWidget * tb_menubar;
	GtkWidget * toolbar;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkToolItem * tb_button;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * menu;
	GtkWidget * menuitem;
#endif

	if((browser = malloc(sizeof(*browser))) == NULL)
	{
		browser_error(NULL, directory != NULL ? directory : ".", 0);
		return NULL;
	}
	browser->window = NULL;
	if(_new_pixbufs(browser) != 0)
	{
		browser_error(browser, "Error while loading default icons", -1);
		free(browser);
		return NULL;
	}

	/* config */
	/* FIXME implement */
	browser->prefs.sort_folders_first = TRUE;
	browser->prefs.show_hidden_files = FALSE;

	/* mime */
	browser->mime = mime_new(); /* FIXME share MIME instances */

	/* history */
	browser->history = NULL;
	browser->current = NULL;

	/* refresh */
	browser->refresh_id = 0;
	browser->refresh_dir = NULL;

	browser->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(browser->window), 640, 480);
	gtk_window_set_title(GTK_WINDOW(browser->window), "File browser");
	g_signal_connect(browser->window, "delete-event", G_CALLBACK(on_closex),
			browser);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	tb_menubar = _new_menubar(browser);
	gtk_box_pack_start(GTK_BOX(vbox), tb_menubar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	browser->tb_back = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	g_signal_connect(browser->tb_back, "clicked", G_CALLBACK(on_back),
			browser);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_back, -1);
	browser->tb_updir = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir), FALSE);
	g_signal_connect(browser->tb_updir, "clicked", G_CALLBACK(on_updir),
			browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_updir, -1);
	browser->tb_forward = gtk_tool_button_new_from_stock(
			GTK_STOCK_GO_FORWARD);
	g_signal_connect(browser->tb_forward, "clicked", G_CALLBACK(on_forward),
			browser);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_forward, -1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect(tb_button, "clicked", G_CALLBACK(on_refresh), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	g_signal_connect(tb_button, "clicked", G_CALLBACK(on_home), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_PROPERTIES);
	g_signal_connect(G_OBJECT(tb_button), "clicked", G_CALLBACK(
				on_properties), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
#if GTK_CHECK_VERSION(2, 6, 0)
	toolitem = gtk_menu_tool_button_new(NULL, "View as...");
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(on_view_as),
			browser);
	menu = gtk_menu_new();
	menuitem = gtk_menu_item_new_with_label("Details");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_details), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Icons");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_icons), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("List");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_list), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Thumbnails");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_thumbnails), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolitem), menu);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	widget = gtk_label_new(" Location: ");
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	browser->tb_path = gtk_combo_box_entry_new_text();
	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	g_signal_connect(G_OBJECT(widget), "activate", G_CALLBACK(
				on_path_activate), browser);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), browser->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_path_activate), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* view */
	browser->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(browser->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), browser->scrolled, TRUE, TRUE, 0);
	/* statusbar */
	browser->statusbar = gtk_statusbar_new();
	browser->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), browser->statusbar, FALSE, FALSE, 0);
	/* store */
	browser->store = _create_store(browser);
	browser->detailview = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->iconview = NULL;
	browser_set_view(browser, BV_ICONS);
	gtk_widget_grab_focus(browser->iconview);
#else
	browser_set_view(browser, BV_DETAILS);
	gtk_widget_grab_focus(browser->detailview);
#endif
	browser_set_location(browser, directory != NULL ? directory
			: g_get_home_dir());

	/* preferences */
	browser->pr_window = NULL;

	/* about */
	browser->ab_window = NULL;

	gtk_container_add(GTK_CONTAINER(browser->window), vbox);
	gtk_widget_show_all(browser->window);
	browser_cnt++;
	return browser;
}

static int _new_pixbufs(Browser * browser)
{
	char * file[] = { "gnome-fs-regular",
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_FILE,
#endif
		GTK_STOCK_MISSING_IMAGE, NULL };
	char * folder[] = { "gnome-fs-directory",
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_DIRECTORY,
#endif
		GTK_STOCK_MISSING_IMAGE, NULL };
	char * executable[] = { "gnome-fs-executable", "gnome-fs-regular",
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_FILE,
#endif
		GTK_STOCK_MISSING_IMAGE, NULL };
	char ** p;

	browser->theme = gtk_icon_theme_get_default();
	browser->pb_file_24 = NULL;
	for(p = file; *p != NULL && browser->pb_file_24 == NULL; p++)
		browser->pb_file_24 = gtk_icon_theme_load_icon(browser->theme,
				*p, 24, 0, NULL);
	browser->pb_folder_24 = NULL;
	for(p = folder; *p != NULL && browser->pb_folder_24 == NULL; p++)
		browser->pb_folder_24 = gtk_icon_theme_load_icon(browser->theme,
				*p, 24, 0, NULL);
	browser->pb_executable_24 = NULL;
	for(p = executable; *p != NULL && browser->pb_executable_24 == NULL;
			p++)
		browser->pb_executable_24 = gtk_icon_theme_load_icon(
				browser->theme, *p, 24, 0, NULL);
#if !GTK_CHECK_VERSION(2, 6, 0)
	return browser->pb_file_24 == NULL || browser->pb_folder_24 == NULL;
#else
	browser->pb_file_48 = NULL;
	for(p = file; *p != NULL && browser->pb_file_48 == NULL; p++)
		browser->pb_file_48 = gtk_icon_theme_load_icon(browser->theme,
				*p, 48, 0, NULL);
	browser->pb_folder_48 = NULL;
	for(p = folder; *p != NULL && browser->pb_folder_48 == NULL; p++)
		browser->pb_folder_48 = gtk_icon_theme_load_icon(browser->theme,
				*p, 48, 0, NULL);
	browser->pb_executable_48 = NULL;
	for(p = executable; *p != NULL && browser->pb_executable_48 == NULL;
			p++)
		browser->pb_executable_48 = gtk_icon_theme_load_icon(
				browser->theme, *p, 48, 0, NULL);
	browser->pb_file_96 = NULL;
	for(p = file; *p != NULL && browser->pb_file_96 == NULL; p++)
		browser->pb_file_96 = gtk_icon_theme_load_icon(browser->theme,
				*p, 96, 0, NULL);
	browser->pb_folder_96 = NULL;
	for(p = folder; *p != NULL && browser->pb_folder_96 == NULL; p++)
		browser->pb_folder_96 = gtk_icon_theme_load_icon(browser->theme,
				*p, 96, 0, NULL);
	browser->pb_executable_96 = NULL;
	for(p = executable; *p != NULL && browser->pb_executable_96 == NULL;
			p++)
		browser->pb_executable_96 = gtk_icon_theme_load_icon(
				browser->theme, *p, 96, 0, NULL);
	return browser->pb_file_24 == NULL || browser->pb_folder_24 == NULL
		|| browser->pb_file_48 == NULL || browser->pb_folder_48 == NULL
		|| browser->pb_file_96 == NULL || browser->pb_folder_96 == NULL;
#endif
}

static GtkWidget * _new_menubar(Browser * browser)
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
			else if(p->stock == NULL)
				menuitem = gtk_menu_item_new_with_mnemonic(
						p->name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						p->stock, NULL);
			if(p->callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(p->callback),
						browser);
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
	gtk_window_add_accel_group(GTK_WINDOW(browser->window), group);
	return tb_menubar;
}

static int _sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
		gpointer data)
{
	Browser * browser = data;
	gboolean is_dir_a;
	gboolean is_dir_b;
	gchar * name_a;
	gchar * name_b;
	int ret = 0;

	gtk_tree_model_get(model, a, BR_COL_IS_DIRECTORY, &is_dir_a,
			BR_COL_DISPLAY_NAME, &name_a, -1);
	gtk_tree_model_get(model, b, BR_COL_IS_DIRECTORY, &is_dir_b,
			BR_COL_DISPLAY_NAME, &name_b, -1);
	if(browser->prefs.sort_folders_first)
	{
		if(!is_dir_a && is_dir_b)
			ret = 1;
		else if(is_dir_a && !is_dir_b)
			ret = -1;
	}
	if(ret == 0)
		ret = g_utf8_collate(name_a, name_b);
	g_free(name_a);
	g_free(name_b);
	return ret;
}

static GtkListStore * _create_store(Browser * browser)
{
	GtkListStore * store;

	store = gtk_list_store_new(BR_NUM_COLS, G_TYPE_BOOLEAN, G_TYPE_STRING,
			G_TYPE_STRING, GDK_TYPE_PIXBUF,
#if GTK_CHECK_VERSION(2, 6, 0)
			GDK_TYPE_PIXBUF, GDK_TYPE_PIXBUF,
#endif
			G_TYPE_UINT64, G_TYPE_BOOLEAN, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store),
			_sort_func, browser, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),
			GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
			GTK_SORT_ASCENDING); /* FIXME make it an option */
	return store;
}


/* browser_delete */
void browser_delete(Browser * browser)
{
	gtk_widget_hide(browser->window);
	if(browser->refresh_id)
		g_source_remove(browser->refresh_id);
	g_list_foreach(browser->history, (GFunc)free, NULL);
	g_list_free(browser->history);
	g_object_unref(browser->store);
	gtk_widget_destroy(browser->window);
	free(browser);
	browser_cnt--;
}


/* private */
static void _browser_set_status(Browser * browser, char const * status)
{
	if(browser->statusbar_id)
		gtk_statusbar_remove(GTK_STATUSBAR(browser->statusbar),
				gtk_statusbar_get_context_id(
					GTK_STATUSBAR(browser->statusbar), ""),
				browser->statusbar_id);
	browser->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				browser->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					browser->statusbar), ""), status);
}


/* useful */
static int _browser_error(char const * message, int ret);
static void _error_response(GtkDialog * dialog, gint arg, gpointer data);
int browser_error(Browser * browser, char const * message, int ret)
{
	GtkWidget * dialog;

	if(browser == NULL)
		return _browser_error(message, ret);
	dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	if(ret < 0)
	{
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
					_error_response), browser);
		ret = -ret;
	}
	else
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
					gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}

static int _browser_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "browser: ");
	perror(message);
	return ret;
}

static void _error_response(GtkDialog * dialog, gint arg, gpointer data)
{
	Browser * browser = data;

	browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
}


static void _refresh_title(Browser * browser);
static void _refresh_path(Browser * browser);
static void _refresh_new(Browser * browser);
static void _refresh_current(Browser * browser);
void browser_refresh(Browser * browser)
{
	DIR * dir;
	struct stat st;
	int fd;

	if(browser->refresh_id != 0)
		g_source_remove(browser->refresh_id);
	browser->refresh_id = 0;
	if(browser->refresh_dir != NULL)
	{
		closedir(browser->refresh_dir);
		browser->refresh_dir = NULL;
	}
	if(browser->current == NULL)
		return;
#if defined(__sun__)
	if((fd = open(browser->current->data, O_RDONLY)) < 0
			|| fstat(fd, &st) != 0
			|| (dir = fdopendir(fd)) == NULL)
	{
		browser_error(browser, strerror(errno), 0);
		if(fd >= 0)
			close(fd);
		return;
	}
#else
	if((dir = opendir(browser->current->data)) == NULL)
	{
		browser_error(browser, strerror(errno), 0);
		return;
	}
	fd = dirfd(dir);
	if(fstat(fd, &st) != 0)
	{
		browser_error(browser, strerror(errno), 0);
		closedir(dir);
		return;
	}
#endif
	browser->refresh_dir = dir;
	browser->refresh_mti = st.st_mtime;
	browser->refresh_cnt = 0;
	browser->refresh_hid = 0;
	_refresh_title(browser);
	_refresh_path(browser);
	_browser_set_status(browser, "Refreshing folder...");
	if(st.st_dev != browser->refresh_dev
			|| st.st_ino != browser->refresh_ino)
		_refresh_new(browser);
	else
		_refresh_current(browser);
	browser->refresh_dev = st.st_dev;
	browser->refresh_ino = st.st_ino;
}

static void _refresh_title(Browser * browser)
{
	char buf[256];

	snprintf(buf, sizeof(buf), "%s%s", "File browser - ",
			(char*)browser->current->data);
	gtk_window_set_title(GTK_WINDOW(browser->window), buf);
}

static void _refresh_path(Browser * browser)
{
	static unsigned int cnt = 0;
	GtkWidget * widget;
	unsigned int i;
	char * p;
	char * q;

	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	gtk_entry_set_text(GTK_ENTRY(widget), browser->current->data);
	for(i = 0; i < cnt; i++)
		gtk_combo_box_remove_text(GTK_COMBO_BOX(browser->tb_path), 0);
	p = g_path_get_dirname(browser->current->data);
	if(strcmp(p, ".") != 0)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(browser->tb_path), p);
		for(cnt = 1; strcmp(p, "/") != 0; cnt++)
		{
			q = g_path_get_dirname(p);
			g_free(p);
			p = q;
			gtk_combo_box_append_text(GTK_COMBO_BOX(
						browser->tb_path), p);
		}
	}
	g_free(p);
}

static int _new_loop(Browser * browser);
static gboolean _new_idle(gpointer data);
static void _refresh_done(Browser * browser);
static void _refresh_new(Browser * browser)
{
	unsigned int i;

	gtk_list_store_clear(browser->store);
	for(i = 0; i < IDLE_LOOP_ICON_CNT && _new_loop(browser) == 0; i++);
	if(i == IDLE_LOOP_ICON_CNT)
		browser->refresh_id = g_idle_add(_new_idle, browser);
	else
		_refresh_done(browser);
}

static int _loop_status(Browser * browser);
static void _loop_insert(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * st,
		gboolean updated);
static int _new_loop(Browser * browser)
{
	struct dirent * de;
	GtkTreeIter iter;
	char * path;
	struct stat st;

	while((de = readdir(browser->refresh_dir)) != NULL)
	{
		if(de->d_name[0] == '.')
		{
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
			browser->refresh_hid++;
		}
		browser->refresh_cnt++;
		if(de->d_name[0] != '.' || browser->prefs.show_hidden_files)
			break;
	}
	if(de == NULL)
		return _loop_status(browser);
	if((path = g_build_filename(browser->current->data, de->d_name, NULL))
			== NULL || lstat(path, &st) != 0)
	{
		_browser_error(de->d_name, 0);
		if(path != NULL)
			g_free(path);
		return 0;
	}
	_loop_insert(browser, &iter, path, de->d_name, &st, 0);
	g_free(path);
	return 0;
}

static int _loop_status(Browser * browser)
{
	char status[36];

	snprintf(status, sizeof(status), "%u file%c (%u hidden)",
			browser->refresh_cnt, browser->refresh_cnt <= 1
			? '\0' : 's', browser->refresh_hid);
	_browser_set_status(browser, status);
	return 1;
}

static char * _insert_size(off_t size);
static void _loop_insert(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * st,
		gboolean updated)
{
	char const * p;
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	uint64_t inode = 0;
	char * size = "";
	char const * type = NULL;
	GdkPixbuf * icon_24 = browser->pb_file_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48 = browser->pb_file_48;
	GdkPixbuf * icon_96 = browser->pb_file_96;
#endif

	if((p = g_filename_to_utf8(display, -1, NULL, NULL, NULL)) != NULL)
		display = p;
	if(st != NULL)
	{
		inode = st->st_ino;
		size = _insert_size(st->st_size);
		pw = getpwuid(st->st_uid);
		gr = getgrgid(st->st_gid);
		if(S_ISDIR(st->st_mode))
		{
			icon_24 = browser->pb_folder_24;
#if GTK_CHECK_VERSION(2, 6, 0)
			icon_48 = browser->pb_folder_48;
			icon_96 = browser->pb_folder_96;
#endif
			type = "inode/directory";
		}
		else if(!S_ISLNK(st->st_mode) && st->st_mode & S_IXUSR)
		{
			icon_24 = browser->pb_executable_24
				? browser->pb_executable_24
				: browser->pb_file_24;
#if GTK_CHECK_VERSION(2, 6, 0)
			icon_48 = browser->pb_executable_48
				? browser->pb_executable_48
				: browser->pb_file_48;
			icon_96 = browser->pb_executable_96
				? browser->pb_executable_96
				: browser->pb_file_96;
#endif
		}
		else if(S_ISBLK(st->st_mode))
			type = "inode/blockdevice";
		else if(S_ISCHR(st->st_mode))
			type = "inode/chardevice";
		else if(S_ISFIFO(st->st_mode))
			type = "inode/fifo";
		else if(S_ISLNK(st->st_mode))
			type = "inode/symlink";
#ifdef S_ISSOCK
		else if(S_ISSOCK(st->st_mode))
			type = "inode/socket";
#endif
		else if(browser->mime != NULL
				&& (type = mime_type(browser->mime, display))
				!= NULL)
		{
			mime_icons(browser->mime, browser->theme, type,
					24, &icon_24,
#if !GTK_CHECK_VERSION(2, 6, 0)
					-1);
#else
					48, &icon_48, 96, &icon_96, -1);
			if(strncmp(type, "image/", 6) == 0)
				icon_96 = gdk_pixbuf_new_from_file_at_size(
						path, 96, 96, NULL);
#endif
		}
	}
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_list_store_insert_with_values(browser->store, iter, -1,
#else
	gtk_list_store_insert_after(browser->store, iter, NULL);
	gtk_list_store_set(browser->store, iter,
#endif
			BR_COL_UPDATED, updated, BR_COL_PATH, path,
			BR_COL_DISPLAY_NAME, display, BR_COL_INODE, inode,
			BR_COL_IS_DIRECTORY, st != NULL ? S_ISDIR(st->st_mode)
			: 0,
			BR_COL_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BR_COL_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
			BR_COL_PIXBUF_96, icon_96 != NULL ? icon_96
			: browser->pb_file_96,
#endif
			BR_COL_MIME_TYPE, type != NULL ? type : "",
			BR_COL_SIZE, size,
			BR_COL_OWNER, pw != NULL ? pw->pw_name : "",
			BR_COL_GROUP, gr != NULL ? gr->gr_name : "",
			-1);
}

static char * _insert_size(off_t size)
{
	static char buf[11];
	double sz = size;
	char * unit;

	if(sz < 1024)
	{
		snprintf(buf, sizeof(buf), "%.0f%s", sz, " bytes");
		return buf;
	}
	else if((sz /= 1024) < 1024)
		unit = "KB";
	else if((sz /= 1024) < 1024)
		unit = "MB";
	else if((sz /= 1024) < 1024)
		unit = "GB";
	else
	{
		sz /= 1024;
		unit = "TB";
	}
	snprintf(buf, sizeof(buf), "%.1f %s", sz, unit);
	return buf;
}

static gboolean _new_idle(gpointer data)
{
	Browser * browser = data;
	unsigned int i;

	for(i = 0; i < IDLE_LOOP_ICON_CNT && _new_loop(browser) == 0; i++);
	if(i == IDLE_LOOP_ICON_CNT)
		return TRUE;
	_refresh_done(browser);
	return FALSE;
}

static gboolean _done_timeout(gpointer data);
static void _refresh_done(Browser * browser)
{
	closedir(browser->refresh_dir);
	browser->refresh_dir = NULL;
	browser->refresh_id = g_timeout_add(1000, _done_timeout, browser);
}

static gboolean _done_timeout(gpointer data)
{
	Browser * browser = data;
	struct stat st;

	if(stat(browser->current->data, &st) != 0)
	{
		browser->refresh_id = 0;
		return _browser_error(browser->current->data, FALSE);
	}
	if(st.st_mtime == browser->refresh_mti)
		return TRUE;
	browser_refresh(browser);
	return FALSE;
}

static int _current_loop(Browser * browser);
static gboolean _current_idle(gpointer data);
static void _current_deleted(Browser * browser);
static void _refresh_current(Browser * browser)
{
	unsigned int i;

	for(i = 0; i < IDLE_LOOP_ICON_CNT && _current_loop(browser) == 0; i++);
	if(i == IDLE_LOOP_ICON_CNT)
	{
		browser->refresh_id = g_idle_add(_current_idle, browser);
		return;
	}
	_current_deleted(browser);
	_refresh_done(browser);
}

static void _loop_update(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * st);
static int _current_loop(Browser * browser)
{
	struct dirent * de;
	char * path;
	struct stat st;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter iter;
	gboolean valid;
	uint64_t inode;

	while((de = readdir(browser->refresh_dir)) != NULL)
	{
		if(de->d_name[0] == '.')
		{
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
			browser->refresh_hid++;
		}
		browser->refresh_cnt++;
		if(de->d_name[0] != '.' || browser->prefs.show_hidden_files)
			break;
	}
	if(de == NULL)
		return _loop_status(browser);
	if((path = g_build_filename(browser->current->data, de->d_name, NULL))
			== NULL || lstat(path, &st) != 0)
	{
		_browser_error(de->d_name, 0);
		if(path != NULL)
			g_free(path);
		return 0;
	}
	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BR_COL_INODE, &inode, -1);
		if(inode == st.st_ino)
			break;
	}
	if(valid != TRUE)
		_loop_insert(browser, &iter, path, de->d_name, &st, 1);
	else
		_loop_update(browser, &iter, path, de->d_name, &st);
	g_free(path);
	return 0;
}

static void _loop_update(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * st)
	/* FIXME code duplication */
{
	char const * p;
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	uint64_t inode = 0;
	char * size = "";
	char const * type = NULL;
	GdkPixbuf * icon_24 = browser->pb_file_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48 = browser->pb_file_48;
	GdkPixbuf * icon_96 = browser->pb_file_96;
#endif

	if((p = g_filename_to_utf8(display, -1, NULL, NULL, NULL)) != NULL)
		display = p;
	if(st != NULL)
	{
		inode = st->st_ino;
		size = _insert_size(st->st_size);
		pw = getpwuid(st->st_uid);
		gr = getgrgid(st->st_gid);
		if(S_ISDIR(st->st_mode))
		{
			icon_24 = browser->pb_folder_24;
#if GTK_CHECK_VERSION(2, 6, 0)
			icon_48 = browser->pb_folder_48;
#endif
			type = "inode/directory";
		}
		else if(!S_ISLNK(st->st_mode) && st->st_mode & S_IXUSR
#if GTK_CHECK_VERSION(2, 6, 0)
				&& browser->pb_executable_48 != NULL
#endif
				&& browser->pb_executable_24 != NULL)
		{
			icon_24 = browser->pb_executable_24;
#if GTK_CHECK_VERSION(2, 6, 0)
			icon_48 = browser->pb_executable_48;
#endif
		}
		else if(S_ISBLK(st->st_mode))
			type = "inode/blockdevice";
		else if(S_ISCHR(st->st_mode))
			type = "inode/chardevice";
		else if(S_ISFIFO(st->st_mode))
			type = "inode/fifo";
		else if(S_ISLNK(st->st_mode))
			type = "inode/symlink";
#ifdef S_ISSOCK
		else if(S_ISSOCK(st->st_mode))
			type = "inode/socket";
#endif
		else if(browser->mime != NULL
				&& (type = mime_type(browser->mime, display))
				!= NULL)
		{
			mime_icons(browser->mime, browser->theme, type,
					24, &icon_24,
#if !GTK_CHECK_VERSION(2, 6, 0)
					-1);
#else
					48, &icon_48, 96, &icon_96, -1);
			if(strncmp(type, "image/", 6) == 0)
				icon_96 = gdk_pixbuf_new_from_file_at_size(
						path, 96, 96, NULL);
#endif
		}
	}
	gtk_list_store_set(browser->store, iter, BR_COL_UPDATED, 1,
			BR_COL_PATH, path, BR_COL_DISPLAY_NAME, display,
			BR_COL_INODE, inode,
			BR_COL_IS_DIRECTORY, st != NULL ? S_ISDIR(st->st_mode)
			: 0,
			BR_COL_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BR_COL_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
#endif
			BR_COL_MIME_TYPE, type != NULL ? type : "",
			BR_COL_SIZE, size,
			BR_COL_OWNER, pw != NULL ? pw->pw_name : "",
			BR_COL_GROUP, gr != NULL ? gr->gr_name : "",
			-1);
}

static gboolean _current_idle(gpointer data)
{
	Browser * browser = data;
	unsigned int i;

	for(i = 0; i < IDLE_LOOP_ICON_CNT && _current_loop(browser) == 0; i++);
	if(i == IDLE_LOOP_ICON_CNT)
		return TRUE;
	_current_deleted(browser);
	_refresh_done(browser);
	return FALSE;
}

static void _current_deleted(Browser * browser)
{
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter iter;
	gboolean valid;
	gboolean updated;

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid == TRUE)
	{
		gtk_tree_model_get(model, &iter, BR_COL_UPDATED, &updated, -1);
		gtk_list_store_set(browser->store, &iter, BR_COL_UPDATED, FALSE,
				-1);
		if(updated == TRUE)
			valid = gtk_tree_model_iter_next(model, &iter);
		else
			valid = gtk_list_store_remove(browser->store, &iter);
	}
}


void browser_select_all(Browser * browser)
{
	GtkTreeSelection * sel;

#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
	{
		gtk_icon_view_select_all(GTK_ICON_VIEW(browser->iconview));
		return;
	}
#endif
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(browser->detailview));
	gtk_tree_selection_select_all(sel);
}


static char * _location_real_path(char const * path);
static int _location_directory(Browser * browser, char * path);
void browser_set_location(Browser * browser, char const * path)
{
	char * realpath = NULL;

	if((realpath = _location_real_path(path)) == NULL)
		return;
	/* XXX check browser_cnt to disallow filenames at startup */
	if(browser_cnt && g_file_test(realpath, G_FILE_TEST_IS_REGULAR))
	{
		if(browser->mime != NULL)
			mime_action(browser->mime, "open", realpath);
	}
	else if(g_file_test(realpath, G_FILE_TEST_IS_DIR)
			&& _location_directory(browser, realpath) == 0)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
				strcmp(browser->current->data, "/") != 0);
		browser_refresh(browser);
		return;
	}
	else
		browser_error(browser, realpath, 0); /* XXX call strerror() */
	free(realpath);
}

static char * _location_real_path(char const * path)
{
	char * p;
	char * cur;

	if(path == NULL)
		return NULL;
	if(path[0] == '/')
		return strdup(path);
	cur = g_get_current_dir();
	p = g_build_filename(cur, path, NULL);
	g_free(cur);
	return p;
}

static int _location_directory(Browser * browser, char * path)
{
	size_t i;

	for(i = strlen(path); i > 1 && path[--i] == '/'; path[i] = '\0');
	if(browser->history == NULL)
	{
		if((browser->history = g_list_alloc()) == NULL)
			return 1;
		browser->history->data = path;
		browser->current = browser->history;
		return 0;
	}
	if(strcmp(browser->current->data, path) == 0)
		return 0;
	g_list_foreach(browser->current->next, (GFunc)free, NULL);
	g_list_free(browser->current->next);
	browser->current->next = NULL;
	browser->history = g_list_append(browser->history, path);
	browser->current = g_list_last(browser->history);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward), FALSE);
	return 0;
}


static void _view_details(Browser * browser);
#if GTK_CHECK_VERSION(2, 6, 0)
static void _view_icons(Browser * browser);
static void _view_list(Browser * browser);
static void _view_thumbnails(Browser * browser);
#endif
void browser_set_view(Browser * browser, BrowserView view)
{
	switch(view)
	{
		case BV_DETAILS:
			_view_details(browser);
			break;
#if GTK_CHECK_VERSION(2, 6, 0)
		case BV_ICONS:
			_view_icons(browser);
			break;
		case BV_LIST:
			_view_list(browser);
			break;
		case BV_THUMBNAILS:
			_view_thumbnails(browser);
			break;
#endif
	}
}

static void _details_column_text(GtkTreeView * view, char const * title,
		int id);
static void _view_details(Browser * browser)
{
	GtkTreeSelection * treesel;
	GtkCellRenderer * renderer;
	GtkTreeView * view;

	if(browser->detailview != NULL)
		return;
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
	{
		/* FIXME keep selection */
		gtk_widget_destroy(browser->iconview);
		browser->iconview = NULL;
	}
#endif
	browser->detailview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	view = GTK_TREE_VIEW(browser->detailview);
	if((treesel = gtk_tree_view_get_selection(view)) != NULL)
		gtk_tree_selection_set_mode(treesel, GTK_SELECTION_MULTIPLE);
	gtk_tree_view_append_column(view,
			gtk_tree_view_column_new_with_attributes("",
				gtk_cell_renderer_pixbuf_new(), "pixbuf",
				BR_COL_PIXBUF_24, NULL));
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	_details_column_text(view, "Filename", BR_COL_DISPLAY_NAME);
	_details_column_text(view, "Size", BR_COL_SIZE);
	_details_column_text(view, "Owner", BR_COL_OWNER);
	_details_column_text(view, "Group", BR_COL_GROUP);
	_details_column_text(view, "MIME type", BR_COL_MIME_TYPE);
	gtk_tree_view_set_headers_visible(view, TRUE);
	g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(
				on_detail_default), browser);
	g_signal_connect(G_OBJECT(view), "button-press-event", G_CALLBACK(
				on_view_press), browser);
	gtk_container_add(GTK_CONTAINER(browser->scrolled),
			browser->detailview);
	gtk_widget_show(browser->detailview);
}

static void _details_column_text(GtkTreeView * view, char const * title, int id)
{
	GtkTreeViewColumn * column;

	column = gtk_tree_view_column_new_with_attributes(title,
			gtk_cell_renderer_text_new(), "text", id, NULL);
	gtk_tree_view_column_set_sort_column_id(column, id);
	gtk_tree_view_append_column(view, column);
}

#if GTK_CHECK_VERSION(2, 6, 0)
static void _view_icon_view(Browser * browser);
static void _view_icons(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkCellRenderer * renderer;

	_view_icon_view(browser);
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "pixbuf", BR_COL_PIXBUF_48, NULL);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, "xalign", 0.5,
			"wrap-mode", PANGO_WRAP_WORD_CHAR, "wrap-width", 96,
			NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_48);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview), 96);
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
	gtk_widget_show(browser->iconview);
}

static void _view_icon_view(Browser * browser)
{
	if(browser->iconview != NULL)
	{
#if GTK_CHECK_VERSION(2, 8, 0)
		gtk_cell_layout_clear(GTK_CELL_LAYOUT(browser->iconview));
#endif
		return;
	}
	if(browser->detailview != NULL)
	{
		gtk_widget_destroy(browser->detailview);
		browser->detailview = NULL;
	}
	browser->iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	gtk_icon_view_set_margin(GTK_ICON_VIEW(browser->iconview), 4);
	gtk_icon_view_set_column_spacing(GTK_ICON_VIEW(browser->iconview), 4);
	gtk_icon_view_set_row_spacing(GTK_ICON_VIEW(browser->iconview), 4);
	gtk_icon_view_set_spacing(GTK_ICON_VIEW(browser->iconview), 4);
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(browser->iconview),
			GTK_SELECTION_MULTIPLE);
	g_signal_connect(G_OBJECT(browser->iconview), "item-activated",
			G_CALLBACK(on_icon_default), browser);
	g_signal_connect(G_OBJECT(browser->iconview), "button-press-event",
			G_CALLBACK(on_view_press), browser);
	g_signal_connect(G_OBJECT(browser->iconview), "popup-menu",
			G_CALLBACK(on_view_popup), browser);
	gtk_container_add(GTK_CONTAINER(browser->scrolled), browser->iconview);
}

static void _view_list(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkCellRenderer * renderer;

	_view_icon_view(browser);
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "pixbuf", BR_COL_PIXBUF_24, NULL);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, "xalign", 0.5,
			"wrap-mode", PANGO_WRAP_WORD_CHAR, "wrap-width", 118,
			NULL);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_24);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview), 146);
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_show(browser->iconview);
}

static void _view_thumbnails(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkCellRenderer * renderer;

	_view_icon_view(browser);
	renderer = gtk_cell_renderer_pixbuf_new();
	g_object_set(renderer, "follow-state", TRUE, NULL);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "pixbuf", BR_COL_PIXBUF_96, NULL);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, "xalign", 0.5,
			"wrap-mode", PANGO_WRAP_WORD_CHAR, "wrap-width", 112,
			NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_96);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview), 112);
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
	gtk_widget_show(browser->iconview);
}
#endif


void browser_unselect_all(Browser * browser)
{
	GtkTreeSelection * sel;

#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
	{
		gtk_icon_view_unselect_all(GTK_ICON_VIEW(browser->iconview));
		return;
	}
#endif
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(browser->detailview));
	gtk_tree_selection_unselect_all(sel);
}
