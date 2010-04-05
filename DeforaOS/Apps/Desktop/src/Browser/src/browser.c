/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <Desktop.h>
#include "callbacks.h"
#include "browser.h"
#define _(string) gettext(string)
#define N_(string) (string)


/* constants */
#define IDLE_LOOP_ICON_CNT	16	/* number of icons added in a loop */
#define ICON_NAME		"system-file-manager"


/* Browser */
/* private */
/* constants */
static DesktopAccel _browser_accel[] =
{
	{ G_CALLBACK(on_location), GDK_CONTROL_MASK, GDK_L },
	{ G_CALLBACK(on_properties), GDK_MOD1_MASK, GDK_Return },
#ifdef EMBEDDED
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_W },
	{ G_CALLBACK(on_copy), GDK_CONTROL_MASK, GDK_C },
	{ G_CALLBACK(on_cut), GDK_CONTROL_MASK, GDK_X },
	{ G_CALLBACK(on_new_window), GDK_CONTROL_MASK, GDK_N },
	{ G_CALLBACK(on_open_file), GDK_CONTROL_MASK, GDK_O },
	{ G_CALLBACK(on_paste), GDK_CONTROL_MASK, GDK_V },
	{ G_CALLBACK(on_refresh), GDK_CONTROL_MASK, GDK_R },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static DesktopMenu _browser_menu_file[] =
{
	{ N_("_New window"), G_CALLBACK(on_file_new_window), "window-new",
		GDK_N },
	{ N_("New _folder"), G_CALLBACK(on_file_new_folder), "folder-new", 0 },
	{ N_("New _symbolic link..."), G_CALLBACK(on_file_new_symlink), NULL,
		0 },
	{ N_("Open file..."), G_CALLBACK(on_file_open_file), NULL, GDK_O },
	{ "", NULL, NULL, 0 },
	{ N_("_Properties"), G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES,
		0 },
	{ "", NULL, NULL, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE, GDK_W },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _browser_menu_edit[] =
{
	{ "_Cut", G_CALLBACK(on_edit_cut), GTK_STOCK_CUT, GDK_X },
	{ "Cop_y", G_CALLBACK(on_edit_copy), GTK_STOCK_COPY, GDK_C },
	{ "_Paste", G_CALLBACK(on_edit_paste), GTK_STOCK_PASTE, GDK_V },
	{ "", NULL, NULL, 0 },
	{ "_Delete", G_CALLBACK(on_edit_delete), GTK_STOCK_DELETE, 0 },
	{ "", NULL, NULL, 0 },
#if GTK_CHECK_VERSION(2, 10, 0)
	{ "_Select all", G_CALLBACK(on_edit_select_all), GTK_STOCK_SELECT_ALL,
#else
	{ "_Select all", G_CALLBACK(on_edit_select_all), "edit-select-all",
#endif
		GDK_A },
	{ "_Unselect all", G_CALLBACK(on_edit_unselect_all), NULL, 0 },
	{ "", NULL, NULL, 0 },
	{ "_Preferences", G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_P },
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _browser_menu_view[] =
{
	{ "_Refresh", G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, GDK_R },
	{ "", NULL, NULL, 0 },
	{ "_Home", G_CALLBACK(on_view_home), GTK_STOCK_HOME, 0 },
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "", NULL, NULL, 0 },
	{ "_Details", G_CALLBACK(on_view_details), "stock_view-details", 0 },
	{ "_Icons", G_CALLBACK(on_view_icons), NULL, 0 },
	{ "_List", G_CALLBACK(on_view_list), NULL, 0 },
	{ "_Thumbnails", G_CALLBACK(on_view_thumbnails), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenu _browser_menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0 },
#else
	{ N_("_About"), G_CALLBACK(on_help_about), NULL, 0 },
#endif
	{ NULL, NULL, NULL, 0 }
};

static DesktopMenubar _browser_menubar[] =
{
	{ N_("_File"), _browser_menu_file },
	{ N_("_Edit"), _browser_menu_edit },
	{ N_("_View"), _browser_menu_view },
	{ N_("_Help"), _browser_menu_help },
	{ NULL, NULL }
};
#endif

/* toolbar */
static DesktopToolbar _browser_toolbar[] =
{
	{ "Back", G_CALLBACK(on_back), GTK_STOCK_GO_BACK, 0, NULL },
	{ "Up", G_CALLBACK(on_updir), GTK_STOCK_GO_UP, 0, NULL },
	{ "Forward", G_CALLBACK(on_forward), GTK_STOCK_GO_FORWARD, 0, NULL },
	{ "Refresh", G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Home", G_CALLBACK(on_home), GTK_STOCK_HOME, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Cut", G_CALLBACK(on_cut), GTK_STOCK_CUT, 0, NULL },
	{ "Copy", G_CALLBACK(on_copy), GTK_STOCK_COPY, 0, NULL },
	{ "Paste", G_CALLBACK(on_paste), GTK_STOCK_PASTE, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Properties", G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES, 0, NULL },
	{ NULL, NULL, NULL, 0, NULL }
};


/* prototypes */
static DIR * _browser_opendir(char const * pathname, struct stat * st);
static void _browser_refresh_do(Browser * browser, DIR * dir, struct stat * st);

static char * _config_get_filename(void);
static void _config_load_boolean(Config * config, char const * variable,
		gboolean * value);
static int _config_load_string(Config * config, char const * variable,
		char ** value);
static int _config_save_boolean(Config * config, char const * variable,
		gboolean value);


/* protected */
/* variables */
unsigned int browser_cnt = 0;


/* public */
/* functions */
/* browser_new */
static gboolean _new_idle(gpointer data);
static int _new_pixbufs(Browser * browser);
static GtkListStore * _create_store(Browser * browser);

Browser * browser_new(char const * directory)
{
	Browser * browser;
	GtkAccelGroup * group;
	GtkWidget * vbox;
#ifndef EMBEDDED
	GtkWidget * tb_menubar;
#endif
	GtkWidget * toolbar;
	GtkWidget * widget;
	GtkToolItem * toolitem;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * menu;
	GtkWidget * menuitem;
#endif
	char * p;

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
	/* set defaults */
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->prefs.default_view = BV_ICONS;
#endif
	browser->prefs.confirm_before_delete = TRUE;
	browser->prefs.sort_folders_first = TRUE;
	browser->prefs.show_hidden_files = FALSE;
	if((browser->config = config_new()) == NULL
			|| browser_config_load(browser) != 0)
		browser_error(browser, "Error while loading configuration", 0);

	/* mime */
	browser->mime = mime_new(); /* FIXME share MIME instances */

	/* history */
	browser->history = NULL;
	browser->current = NULL;

	/* refresh */
	browser->refresh_id = 0;
	browser->refresh_dir = NULL;
	browser->refresh_dev = 0;

	/* selection */
	browser->selection = NULL;
	browser->selection_cut = 0;

	/* widgets */
	group = gtk_accel_group_new();
	browser->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(browser->window), group);
	gtk_window_set_default_size(GTK_WINDOW(browser->window), 640, 480);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(browser->window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(browser->window), _("File browser"));
	g_signal_connect(browser->window, "delete-event", G_CALLBACK(on_closex),
			browser);
	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
#ifndef EMBEDDED
	tb_menubar = desktop_menubar_create(_browser_menubar, browser, group);
	gtk_box_pack_start(GTK_BOX(vbox), tb_menubar, FALSE, FALSE, 0);
#endif
	desktop_accel_create(_browser_accel, browser, group);
	/* toolbar */
	toolbar = desktop_toolbar_create(_browser_toolbar, browser, group);
	browser->tb_back = _browser_toolbar[0].widget;
	browser->tb_updir = _browser_toolbar[1].widget;
	browser->tb_forward = _browser_toolbar[2].widget;
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	toolitem = gtk_menu_tool_button_new(NULL, _("View as..."));
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_view_as), browser);
	menu = gtk_menu_new();
	menuitem = gtk_image_menu_item_new_with_label(_("Details"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
			gtk_image_new_from_icon_name("stock_view-details",
				GTK_ICON_SIZE_MENU));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_details), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Icons"));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_icons), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("List"));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_list), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Thumbnails"));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_view_thumbnails), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolitem), menu);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
#ifdef EMBEDDED
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
				on_edit_preferences), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
#ifndef EMBEDDED
	widget = gtk_label_new(_(" Location: "));
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	browser->tb_path = gtk_combo_box_entry_new_text();
	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if(directory != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), directory);
	g_signal_connect_swapped(G_OBJECT(widget), "activate", G_CALLBACK(
				on_path_activate), browser);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), browser->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect_swapped(G_OBJECT(toolitem), "clicked", G_CALLBACK(
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
	gtk_box_pack_start(GTK_BOX(vbox), browser->statusbar, FALSE, TRUE, 0);
	/* store */
	browser->store = _create_store(browser);
	browser->detailview = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->iconview = NULL;
	browser->iconview_as = browser->prefs.default_view;
	browser_set_view(browser, browser->iconview_as);
	if(browser->iconview != NULL)
		gtk_widget_grab_focus(browser->iconview);
#else
	browser_set_view(browser, BV_DETAILS);
	gtk_widget_grab_focus(browser->detailview);
#endif

	/* preferences */
	browser->pr_window = NULL;

	/* about */
	browser->ab_window = NULL;

	/* open directory */
	if(directory != NULL && (p = strdup(directory)) != NULL)
	{
		browser->history = g_list_append(browser->history, p);
		browser->current = browser->history;
	}
	g_idle_add(_new_idle, browser);

	gtk_container_add(GTK_CONTAINER(browser->window), vbox);
	gtk_widget_show_all(browser->window);
	browser_cnt++;
	return browser;
}

static gboolean _new_idle(gpointer data)
{
	Browser * browser = data;

	if(browser->current == NULL)
		browser_go_home(browser);
	else
		browser_set_location(browser, browser->current->data);
	return FALSE;
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
	browser->pb_file_96 = NULL;
	for(p = file; *p != NULL && browser->pb_file_96 == NULL; p++)
		browser->pb_file_96 = gtk_icon_theme_load_icon(browser->theme,
				*p, 96, 0, NULL);
	browser->pb_folder_96 = NULL;
	for(p = folder; *p != NULL && browser->pb_folder_96 == NULL; p++)
		browser->pb_folder_96 = gtk_icon_theme_load_icon(browser->theme,
				*p, 96, 0, NULL);
	return browser->pb_file_24 == NULL || browser->pb_folder_24 == NULL
		|| browser->pb_file_48 == NULL || browser->pb_folder_48 == NULL
		|| browser->pb_file_96 == NULL || browser->pb_folder_96 == NULL;
#endif
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
			G_TYPE_UINT64, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
			G_TYPE_BOOLEAN, G_TYPE_UINT64, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT,
			G_TYPE_STRING, G_TYPE_STRING);
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
	if(browser->config != NULL)
		config_delete(browser->config);
	gtk_widget_hide(browser->window);
	if(browser->refresh_id)
		g_source_remove(browser->refresh_id);
	g_list_foreach(browser->history, (GFunc)free, NULL);
	g_list_free(browser->history);
	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	g_object_unref(browser->store);
	gtk_widget_destroy(browser->window);
	free(browser);
	browser_cnt--;
}


/* private */
static void _browser_set_status(Browser * browser, char const * status)
{
	GtkStatusbar * sb;

	sb = GTK_STATUSBAR(browser->statusbar);
	if(browser->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				browser->statusbar_id);
	browser->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* useful */
/* browser_error */
static int _browser_error(char const * message, int ret);
/* callbacks */
static void _error_response(gpointer data);

int browser_error(Browser * browser, char const * message, int ret)
{
	GtkWidget * dialog;
	char const * error;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %d) errno=%d\n", __func__, message,
			ret, errno);
#endif
	if(browser == NULL)
		return _browser_error(message, ret);
	error = strerror(errno);
	dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message, error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	if(ret < 0)
	{
		g_signal_connect_swapped(G_OBJECT(dialog), "response",
				G_CALLBACK(_error_response), browser);
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
	fputs("browser: ", stderr);
	perror(message);
	return ret;
}

static void _error_response(gpointer data)
{
	Browser * browser = data;

	browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
}


/* browser_config_load */
int browser_config_load(Browser * browser)
{
	char * filename;
#if GTK_CHECK_VERSION(2, 6, 0)
	char * p = NULL;
#endif

	if(browser->config == NULL)
		return 0; /* XXX ignore error */
	if((filename = _config_get_filename()) == NULL)
		return 1;
	config_load(browser->config, filename); /* XXX ignore errors */
	free(filename);
#if GTK_CHECK_VERSION(2, 6, 0)
	/* XXX deserves a rework (enum) */
	if(_config_load_string(browser->config, "default_view", &p) == 0
			&& p != NULL)
	{
		if(strcmp(p, "details") == 0)
			browser->prefs.default_view = BV_DETAILS;
		else if(strcmp(p, "icons") == 0)
			browser->prefs.default_view = BV_ICONS;
		else if(strcmp(p, "list") == 0)
			browser->prefs.default_view = BV_LIST;
		else if(strcmp(p, "thumbnails") == 0)
			browser->prefs.default_view = BV_THUMBNAILS;
		free(p);
	}
#endif
	_config_load_boolean(browser->config, "confirm_before_delete",
			&browser->prefs.confirm_before_delete);
	_config_load_boolean(browser->config, "sort_folders_first",
			&browser->prefs.sort_folders_first);
	_config_load_boolean(browser->config, "show_hidden_files",
			&browser->prefs.show_hidden_files);
	return 0;
}


/* browser_config_save */
int browser_config_save(Browser * browser)
{
	int ret = 0;
	char * filename;
#if GTK_CHECK_VERSION(2, 6, 0)
	char * str[BV_COUNT] = { "details", "icons", "list", "thumbnails" };
#endif

	if(browser->config == NULL)
		return 0; /* XXX ignore error */
	if((filename = _config_get_filename()) == NULL)
		return 1;
#if GTK_CHECK_VERSION(2, 6, 0)
	/* XXX deserves a rework (enum) */
	if(browser->prefs.default_view >= BV_FIRST
			&& browser->prefs.default_view <= BV_LAST)
		ret |= config_set(browser->config, "", "default_view",
				str[browser->prefs.default_view]);
#endif
	ret |= _config_save_boolean(browser->config, "confirm_before_delete",
			browser->prefs.confirm_before_delete);
	ret |= _config_save_boolean(browser->config, "sort_folders_first",
			browser->prefs.sort_folders_first);
	ret |= _config_save_boolean(browser->config, "show_hidden_files",
			browser->prefs.show_hidden_files);
	if(ret == 0)
		ret |= config_save(browser->config, filename);
	free(filename);
	return ret;
}


/* browser_focus_location */
void browser_focus_location(Browser * browser)
{
	gtk_widget_grab_focus(browser->tb_path);
}


/* browser_go_home */
void browser_go_home(Browser * browser)
{
	char const * home;

	if((home = getenv("HOME")) == NULL)
		home = g_get_home_dir();
	/* XXX use open while set_location should only update the toolbar? */
	browser_set_location(browser, home != NULL ? home : "/");
}


/* browser_open */
void browser_open(Browser * browser, char const * path)
{
	GtkWidget * dialog;

	if(path == NULL)
	{
		dialog = gtk_file_chooser_dialog_new(_("Open file..."),
				GTK_WINDOW(browser->window),
				GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
				GTK_RESPONSE_ACCEPT, NULL);
		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
			path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
						dialog));
		gtk_widget_destroy(dialog);
	}
	if(browser->mime != NULL && path != NULL)
		mime_action(browser->mime, "open", path);
}


/* browser_open_with */
void browser_open_with(Browser * browser, char const * path)
{
	GtkWidget * dialog;
	char * filename = NULL;
	pid_t pid;

	dialog = gtk_file_chooser_dialog_new(_("Open with..."),
			GTK_WINDOW(browser->window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	if((pid = fork()) == -1)
		browser_error(browser, "fork", 0);
	else if(pid == 0)
	{
		if(close(0) != 0)
			browser_error(NULL, "stdin", 0);
		execlp(filename, filename, path, NULL);
		browser_error(NULL, filename, 0);
		exit(2);
	}
	g_free(filename);
}


/* browser_refresh */
void browser_refresh(Browser * browser)
{
	DIR * dir;
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__, browser->current != NULL
			? (char*)browser->current->data : "NULL");
#endif
	if(browser->current == NULL)
		return;
	if((dir = _browser_opendir(browser->current->data, &st)) == NULL)
		browser_error(browser, browser->current->data, 0);
	else
		_browser_refresh_do(browser, dir, &st);
}


/* _refresh_new */
static int _refresh_new_loop(Browser * browser);
static gboolean _refresh_new_idle(gpointer data);
static void _refresh_done(Browser * browser);

static void _refresh_new(Browser * browser)
{
	unsigned int i;

	gtk_list_store_clear(browser->store);
	for(i = 0; i < IDLE_LOOP_ICON_CNT
			&& _refresh_new_loop(browser) == 0; i++);
	if(i == IDLE_LOOP_ICON_CNT)
		browser->refresh_id = g_idle_add(_refresh_new_idle, browser);
	else
		_refresh_done(browser);
}


/* _refresh_new_loop */
static int _loop_status(Browser * browser);
static void _loop_insert(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * lst,
		struct stat * st, gboolean updated);

static int _refresh_new_loop(Browser * browser)
{
	struct dirent * de;
	GtkTreeIter iter;
	char * path;
	struct stat lst;
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
			== NULL || lstat(path, &lst) != 0)
	{
		browser_error(NULL, de->d_name, 0);
		if(path != NULL)
			g_free(path);
		return 0;
	}
	if(S_ISLNK(lst.st_mode) && stat(path, &st) == 0)
		_loop_insert(browser, &iter, path, de->d_name, &lst, &st, 0);
	else
		_loop_insert(browser, &iter, path, de->d_name, &lst, &lst, 0);
	g_free(path);
	return 0;
}

static int _loop_status(Browser * browser)
{
	char status[36];

	snprintf(status, sizeof(status), _("%u file%c (%u hidden)"),
			browser->refresh_cnt, browser->refresh_cnt <= 1
			? '\0' : 's', browser->refresh_hid); /* XXX translate */
	_browser_set_status(browser, status);
	return 1;
}


/* _loop_insert */
static void _insert_all(Browser * browser, struct stat * lst, struct stat * st,
		char const ** display, uint64_t * inode, size_t * size,
		char const ** dsize, struct passwd ** pw, struct group ** gr,
		char const ** ddate, char const ** type, char const * path,
		GdkPixbuf ** icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
		GdkPixbuf ** icon_48, GdkPixbuf ** icon_96
#endif
		);

static void _loop_insert(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * lst,
		struct stat * st, gboolean updated)
{
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	uint64_t inode = 0;
	uint64_t size = 0;
	char const * dsize = "";
	char const * ddate = "";
	char const * type = NULL;
	GdkPixbuf * icon_24 = browser->pb_file_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48 = browser->pb_file_48;
	GdkPixbuf * icon_96 = browser->pb_file_96;
#endif

#ifdef DEBUG
	fprintf(stderr, "%s%s(\"%s\")\n", "DEBUG: ", __func__, display);
#endif
	_insert_all(browser, lst, st, &display, &inode, &size, &dsize, &pw, &gr,
			&ddate, &type, path, &icon_24
#if GTK_CHECK_VERSION(2, 6, 0)
			, &icon_48, &icon_96);
	gtk_list_store_insert_with_values(browser->store, iter, -1,
#else
		);
	gtk_list_store_insert_after(browser->store, iter, NULL);
	gtk_list_store_set(browser->store, iter,
#endif
			BR_COL_UPDATED, updated, BR_COL_PATH, path,
			BR_COL_DISPLAY_NAME, display, BR_COL_INODE, inode,
			BR_COL_IS_DIRECTORY, S_ISDIR(st->st_mode),
			BR_COL_IS_EXECUTABLE, st->st_mode & S_IXUSR,
			BR_COL_IS_MOUNT_POINT,
			st->st_dev != browser->refresh_dev,
			BR_COL_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BR_COL_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
			BR_COL_PIXBUF_96, icon_96 != NULL ? icon_96
			: browser->pb_file_96,
#endif
			BR_COL_SIZE, size, BR_COL_DISPLAY_SIZE, dsize,
			BR_COL_OWNER, pw != NULL ? pw->pw_name : "",
			BR_COL_GROUP, gr != NULL ? gr->gr_name : "",
			BR_COL_DATE, lst->st_mtime, BR_COL_DISPLAY_DATE, ddate,
			BR_COL_MIME_TYPE, type != NULL ? type : "", -1);
}

/* insert_all */
static char const * _insert_size(off_t size);
static char const * _insert_date(time_t date);
static char const * _insert_mode(mode_t mode, dev_t parent, dev_t dev);
static void _insert_dir(Browser * browser, GdkPixbuf ** icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
		GdkPixbuf ** icon_48, GdkPixbuf ** icon_96,
#endif
		dev_t dev);

static void _insert_all(Browser * browser, struct stat * lst, struct stat * st,
		char const ** display, uint64_t * inode, size_t * size,
		char const ** dsize, struct passwd ** pw, struct group ** gr,
		char const ** ddate, char const ** type, char const * path,
		GdkPixbuf ** icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
		GdkPixbuf ** icon_48, GdkPixbuf ** icon_96
#endif
		)
{
	char const * p;

	if((p = g_filename_to_utf8(*display, -1, NULL, NULL, NULL)) != NULL)
		*display = p; /* XXX should display be free'd? */
	*inode = lst->st_ino;
	*size = lst->st_size;
	*dsize = _insert_size(lst->st_size);
	*pw = getpwuid(lst->st_uid);
	*gr = getgrgid(lst->st_gid);
	*ddate = _insert_date(lst->st_mtime);
	*type = _insert_mode(lst->st_mode, browser->refresh_dev, lst->st_dev);
	if(S_ISDIR(st->st_mode))
		_insert_dir(browser, icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
				icon_48, icon_96,
#endif
				st->st_dev);
	else if(st->st_mode & S_IXUSR)
		mime_icons(browser->mime, browser->theme,
				"application/x-executable", 24, icon_24,
#if !GTK_CHECK_VERSION(2, 6, 0)
				-1);
#else
				48, icon_48, 96, icon_96, -1);
#endif
	else if(browser->mime != NULL && *type == NULL
			&& (*type = mime_type(browser->mime, path)) != NULL)
	{
		mime_icons(browser->mime, browser->theme, *type, 24, icon_24,
#if !GTK_CHECK_VERSION(2, 6, 0)
				-1);
#else
				48, icon_48, 96, icon_96, -1);
		if(strncmp(*type, "image/", 6) == 0)
			*icon_96 = gdk_pixbuf_new_from_file_at_size(path, 96,
					96, NULL);
#endif
	}
}

static char const * _insert_size(off_t size)
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

static char const * _insert_date(time_t date)
{
	static char buf[16];
	static time_t sixmonths = -1;
	struct tm tm;
	size_t len;

	if(sixmonths == -1)
		sixmonths = time(NULL) - 15552000;
	localtime_r(&date, &tm);
	if(date < sixmonths)
		len = strftime(buf, sizeof(buf), "%b %e  %Y", &tm);
	else
		len = strftime(buf, sizeof(buf), "%b %e %H:%M", &tm);
	buf[len] = '\0';
	return buf;
}

static char const * _insert_mode(mode_t mode, dev_t parent, dev_t dev)
{
	if(S_ISDIR(mode))
	{
		if(parent != dev)
			return "inode/mountpoint";
		return "inode/directory";
	}
	else if(S_ISBLK(mode))
		return "inode/blockdevice";
	else if(S_ISCHR(mode))
		return "inode/chardevice";
	else if(S_ISFIFO(mode))
		return "inode/fifo";
	else if(S_ISLNK(mode))
		return "inode/symlink";
#ifdef S_ISSOCK
	else if(S_ISSOCK(mode))
		return "inode/socket";
#endif
	return NULL;
}

static void _insert_dir(Browser * browser, GdkPixbuf ** icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
		GdkPixbuf ** icon_48, GdkPixbuf ** icon_96,
#endif
		dev_t dev)
{
	char * rmt = "folder-remote";

	if(browser->refresh_dev == dev)
	{
		*icon_24 = browser->pb_folder_24;
#if GTK_CHECK_VERSION(2, 6, 0)
		*icon_48 = browser->pb_folder_48;
		*icon_96 = browser->pb_folder_96;
#endif
		return;
	}
	if((*icon_24 = gtk_icon_theme_load_icon(browser->theme, rmt, 24, 0,
					NULL)) == NULL)
		*icon_24 = browser->pb_folder_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	if((*icon_48 = gtk_icon_theme_load_icon(browser->theme, rmt, 48, 0,
					NULL)) == NULL)
		*icon_48 = browser->pb_folder_48;
	if((*icon_96 = gtk_icon_theme_load_icon(browser->theme, rmt, 96, 0,
					NULL)) == NULL)
		*icon_96 = browser->pb_folder_96;
#endif
}

static gboolean _refresh_new_idle(gpointer data)
{
	Browser * browser = data;
	unsigned int i;

	for(i = 0; i < IDLE_LOOP_ICON_CNT
			&& _refresh_new_loop(browser) == 0; i++);
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
		return browser_error(NULL, browser->current->data, FALSE);
	}
	if(st.st_mtime == browser->refresh_mti)
		return TRUE;
	browser_refresh(browser);
	return FALSE;
}

/* refresh_current */
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
		char const * path, char const * display, struct stat * lst,
		struct stat * st);
static int _current_loop(Browser * browser)
{
	struct dirent * de;
	char * path;
	struct stat lst;
	struct stat st;
	struct stat * p = &lst;
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
			== NULL || lstat(path, &lst) != 0)
	{
		browser_error(NULL, de->d_name, 0);
		if(path != NULL)
			g_free(path);
		return 0;
	}
	valid = gtk_tree_model_get_iter_first(model, &iter);
	for(; valid == TRUE; valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BR_COL_INODE, &inode, -1);
		if(inode == lst.st_ino)
			break;
	}
	if(S_ISLNK(lst.st_mode) && stat(path, &st) == 0)
		p = &st;
	if(valid != TRUE)
		_loop_insert(browser, &iter, path, de->d_name, &lst, p, 1);
	else
		_loop_update(browser, &iter, path, de->d_name, &lst, p);
	g_free(path);
	return 0;
}

static void _loop_update(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * lst,
		struct stat * st)
{
	struct passwd * pw = NULL;
	struct group * gr = NULL;
	uint64_t inode = 0;
	uint64_t size = 0;
	char const * dsize = "";
	char const * ddate = "";
	char const * type = NULL;
	GdkPixbuf * icon_24 = browser->pb_file_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * icon_48 = browser->pb_file_48;
	GdkPixbuf * icon_96 = browser->pb_file_96;
#endif

#ifdef DEBUG
	fprintf(stderr, "%s%s(\"%s\")\n", "DEBUG: ", __func__, display);
#endif
	_insert_all(browser, lst, st, &display, &inode, &size, &dsize, &pw, &gr,
			&ddate, &type, path, &icon_24
#if GTK_CHECK_VERSION(2, 6, 0)
			, &icon_48, &icon_96
#endif
		   );
	gtk_list_store_set(browser->store, iter, BR_COL_UPDATED, 1,
			BR_COL_PATH, path, BR_COL_DISPLAY_NAME, display,
			BR_COL_INODE, inode, BR_COL_IS_DIRECTORY,
			S_ISDIR(st->st_mode), BR_COL_IS_EXECUTABLE,
			st->st_mode & S_IXUSR, BR_COL_IS_MOUNT_POINT,
			st->st_dev != browser->refresh_dev,
			BR_COL_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BR_COL_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
#endif
			BR_COL_SIZE, size, BR_COL_DISPLAY_SIZE, dsize,
			BR_COL_OWNER, pw != NULL ? pw->pw_name : "",
			BR_COL_GROUP, gr != NULL ? gr->gr_name : "",
			BR_COL_DATE, lst->st_mtime, BR_COL_DISPLAY_DATE, ddate,
			BR_COL_MIME_TYPE, type != NULL ? type : "", -1);
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


/* browser_select_all */
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


/* browser_set_location */
static char * _location_real_path(char const * path);
static int _location_directory(Browser * browser, char const * path, DIR * dir,
		struct stat * st);

void browser_set_location(Browser * browser, char const * path)
{
	char * realpath = NULL;
	DIR * dir;
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, path);
#endif
	if((realpath = _location_real_path(path)) == NULL)
		return;
	/* XXX check browser_cnt to disallow filenames at startup */
	if(browser_cnt && g_file_test(realpath, G_FILE_TEST_IS_REGULAR))
	{
		if(browser->mime != NULL)
			mime_action(browser->mime, "open", realpath);
	}
	else if(g_file_test(realpath, G_FILE_TEST_IS_DIR)
			&& (dir = _browser_opendir(realpath, &st)) != NULL)
	{
		if(_location_directory(browser, realpath, dir, &st) == 0)
			gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
					strcmp(browser->current->data, "/"));
		else
			closedir(dir);
	}
	else
		/* XXX errno may not be set */
		browser_error(browser, realpath, 0);
	free(realpath);
}

static char * _location_real_path(char const * path)
{
	char * p;
	char * cur;
	size_t i;

	if(path == NULL)
		return NULL;
	if(path[0] == '/')
		p = strdup(path);
	else
	{
		cur = g_get_current_dir();
		p = g_build_filename(cur, path, NULL);
		g_free(cur);
	}
	/* trim slashes in the end */
	for(i = strlen(p); i > 1 && p[--i] == '/'; p[i] = '\0');
	/* FIXME replace "/./" and "/"+ by "/" */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\") => \"%s\"\n", __func__, path, p);
#endif
	return p;
}

static int _location_directory(Browser * browser, char const * path, DIR * dir,
		struct stat * st)
{
	char * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, path);
#endif
	if((p = strdup(path)) == NULL)
		return -1;
	if(browser->history == NULL)
	{
		if((browser->history = g_list_alloc()) == NULL)
			return 1;
		browser->history->data = p;
		browser->current = browser->history;
	}
	else if(strcmp(browser->current->data, p) != 0)
	{
		g_list_foreach(browser->current->next, (GFunc)free, NULL);
		g_list_free(browser->current->next);
		browser->current->next = NULL;
		browser->history = g_list_append(browser->history, p);
		browser->current = g_list_last(browser->history);
		gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back),
				browser->current->prev != NULL ? TRUE : FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward),
				FALSE);
	}
	else
		free(p);
	_browser_refresh_do(browser, dir, st);
	return 0;
}


/* browser_set_view */
static void _view_details(Browser * browser);
#if GTK_CHECK_VERSION(2, 6, 0)
static void _view_icons(Browser * browser);
static void _view_list(Browser * browser);
static void _view_thumbnails(Browser * browser);
#endif

void browser_set_view(Browser * browser, BrowserView view)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->iconview_as = view;
	switch(view)
	{
		case BV_DETAILS:
			_view_details(browser);
			break;
		case BV_ICONS:
			_view_icons(browser);
			break;
		case BV_LIST:
			_view_list(browser);
			break;
		case BV_THUMBNAILS:
			_view_thumbnails(browser);
			break;
	}
#else
	_view_details(browser);
#endif
}

static void _details_column_text(GtkTreeView * view, GtkCellRenderer * renderer,
		char const * title, int id, int sort);

static void _view_details(Browser * browser)
{
	GtkTreeSelection * treesel;
	GtkCellRenderer * renderer;
	GtkTreeView * view;
#if GTK_CHECK_VERSION(2, 6, 0)
	GList * sel = NULL;
	GList * p;
#endif

	if(browser->detailview != NULL)
		return;
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
	{
		sel = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
					browser->iconview));
		gtk_widget_destroy(browser->iconview);
		browser->iconview = NULL;
	}
#endif
	browser->detailview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	view = GTK_TREE_VIEW(browser->detailview);
	if((treesel = gtk_tree_view_get_selection(view)) != NULL)
	{
		gtk_tree_selection_set_mode(treesel, GTK_SELECTION_MULTIPLE);
#if GTK_CHECK_VERSION(2, 6, 0)
		if(sel != NULL)
		{
			for(p = sel; p != NULL; p = p->next)
				gtk_tree_selection_select_path(treesel,
						p->data);
			g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(sel);
		}
#endif
	}
	gtk_tree_view_append_column(view,
			gtk_tree_view_column_new_with_attributes("",
				gtk_cell_renderer_pixbuf_new(), "pixbuf",
				BR_COL_PIXBUF_24, NULL));
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	_details_column_text(view, renderer, _("Filename"), BR_COL_DISPLAY_NAME,
			BR_COL_DISPLAY_NAME);
	_details_column_text(view, NULL, _("Size"), BR_COL_DISPLAY_SIZE,
			BR_COL_SIZE);
	_details_column_text(view, NULL, _("Owner"), BR_COL_OWNER,
			BR_COL_OWNER);
	_details_column_text(view, NULL, _("Group"), BR_COL_GROUP,
			BR_COL_GROUP);
	_details_column_text(view, NULL, _("Date"), BR_COL_DISPLAY_DATE,
			BR_COL_DATE);
	_details_column_text(view, NULL, _("MIME type"), BR_COL_MIME_TYPE,
			BR_COL_MIME_TYPE);
	gtk_tree_view_set_headers_visible(view, TRUE);
	g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(
				on_detail_default), browser);
	g_signal_connect(G_OBJECT(view), "button-press-event", G_CALLBACK(
				on_view_press), browser);
	gtk_container_add(GTK_CONTAINER(browser->scrolled),
			browser->detailview);
	gtk_widget_show(browser->detailview);
}

static void _details_column_text(GtkTreeView * view, GtkCellRenderer * renderer,
		char const * title, int id, int sort)
{
	GtkTreeViewColumn * column;

	if(renderer == NULL)
		renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(title, renderer,
			"text", id, NULL);
	gtk_tree_view_column_set_sort_column_id(column, sort);
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
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 16, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.5, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_48);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_ICON_WRAP_WIDTH);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
	gtk_widget_show(browser->iconview);
}

static void _view_icon_view(Browser * browser)
{
	GtkTreeSelection * treesel;
	GList * sel = NULL;
	GList * p;
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkTargetEntry targets[] = { { "deforaos_browser_dnd", 0, 0 } };
	size_t targets_cnt = sizeof(targets) / sizeof(*targets);
#endif

	if(browser->iconview != NULL)
	{
#if GTK_CHECK_VERSION(2, 8, 0)
		gtk_cell_layout_clear(GTK_CELL_LAYOUT(browser->iconview));
#endif
		return;
	}
	if(browser->detailview != NULL)
	{
		if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							browser->detailview)))
				!= NULL)
			sel = gtk_tree_selection_get_selected_rows(treesel,
					NULL);
		gtk_widget_destroy(browser->detailview);
		browser->detailview = NULL;
	}
	browser->iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(browser->iconview),
			GTK_SELECTION_MULTIPLE); /* needs to be done now */
	if(sel != NULL)
	{
		for(p = sel; p != NULL; p = p->next)
			gtk_icon_view_select_path(GTK_ICON_VIEW(
						browser->iconview), p->data);
		g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
		g_list_free(sel);

	}
#if GTK_CHECK_VERSION(2, 8, 0)
	gtk_icon_view_enable_model_drag_source(GTK_ICON_VIEW(browser->iconview),
			GDK_BUTTON1_MASK, targets, targets_cnt,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_icon_view_enable_model_drag_dest(GTK_ICON_VIEW(browser->iconview),
			targets, targets_cnt,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
#endif
	g_signal_connect(G_OBJECT(browser->iconview), "item-activated",
			G_CALLBACK(on_icon_default), browser);
	g_signal_connect(G_OBJECT(browser->iconview), "button-press-event",
			G_CALLBACK(on_view_press), browser);
	g_signal_connect(G_OBJECT(browser->iconview), "popup-menu",
			G_CALLBACK(on_view_popup), browser);
#if GTK_CHECK_VERSION(2, 8, 0)
	g_signal_connect(G_OBJECT(browser->iconview), "drag-data-get",
			G_CALLBACK(on_view_drag_data_get), browser);
	g_signal_connect(G_OBJECT(browser->iconview), "drag-data-received",
			G_CALLBACK(on_view_drag_data_received), browser);
#endif
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
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 20, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.0, NULL);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_24);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_LIST_WRAP_WIDTH + 24);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
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
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 22, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.5, NULL);
	g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(
				on_filename_edited), browser);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BR_COL_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF_96);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_THUMBNAIL_WRAP_WIDTH);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
	gtk_widget_show(browser->iconview);
}
#endif


/* browser_unselect_all */
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


/* private */
/* functions */
static DIR * _browser_opendir(char const * pathname, struct stat * st)
{
	DIR * dir;
	int fd;

#ifdef DEBUG
	/* FIXME errno may be modified before reaching this point */
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, pathname);
#endif
#if defined(__sun__)
	if((fd = open(pathname, O_RDONLY)) < 0
			|| (dir = fdopendir(fd)) == NULL)
	{
		if(fd >= 0)
			close(fd);
		return NULL;
	}
#else
	if((dir = opendir(pathname)) == NULL)
		return NULL;
	fd = dirfd(dir);
#endif
	if(st != NULL && fstat(fd, st) != 0)
	{
		closedir(dir);
		return NULL;
	}
	return dir;
}


/* browser_refresh_do */
static void _refresh_title(Browser * browser);
static void _refresh_path(Browser * browser);
static void _refresh_new(Browser * browser);
static void _refresh_current(Browser * browser);

static void _browser_refresh_do(Browser * browser, DIR * dir, struct stat * st)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__,
			(char*)browser->current->data);
#endif
	if(browser->refresh_id != 0)
		g_source_remove(browser->refresh_id);
	browser->refresh_id = 0;
	if(browser->refresh_dir != NULL)
		closedir(browser->refresh_dir);
	browser->refresh_dir = dir;
	browser->refresh_mti = st->st_mtime;
	browser->refresh_cnt = 0;
	browser->refresh_hid = 0;
	_refresh_title(browser);
	_refresh_path(browser);
	_browser_set_status(browser, _("Refreshing folder..."));
	if(st->st_dev != browser->refresh_dev
			|| st->st_ino != browser->refresh_ino)
	{
		browser->refresh_dev = st->st_dev;
		browser->refresh_ino = st->st_ino;
		_refresh_new(browser);
	}
	else
		_refresh_current(browser);
}

static void _refresh_title(Browser * browser)
{
	char buf[256];
	char * p;

	p = g_filename_to_utf8(browser->current->data, -1, NULL, NULL, NULL);
	snprintf(buf, sizeof(buf), "%s%s", "File browser - ", p != NULL ? p
			: (char*)browser->current->data);
	free(p);
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
	p = g_filename_to_utf8(browser->current->data, -1, NULL, NULL, NULL);
	gtk_entry_set_text(GTK_ENTRY(widget), p != NULL ? p
			: browser->current->data);
	free(p);
	for(i = 0; i < cnt; i++)
		gtk_combo_box_remove_text(GTK_COMBO_BOX(browser->tb_path), 0);
	if((p = g_path_get_dirname(browser->current->data)) == NULL)
		return;
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


static char * _config_get_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(BROWSER_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, BROWSER_CONFIG_FILE);
	return filename;
}


static void _config_load_boolean(Config * config, char const * variable,
		gboolean * value)
{
	char const * str;

	if((str = config_get(config, "", variable)) == NULL)
		return; /* XXX default to something? */
	if(strcmp(str, "0") == 0)
		*value = FALSE;
	else if(strcmp(str, "1") == 0)
		*value = TRUE;
}


static int _config_load_string(Config * config, char const * variable,
		char ** value)
{
	char const * str;
	char * p;

	if((str = config_get(config, "", variable)) == NULL)
		return 0;
	if((p = strdup(str)) == NULL)
		return 1;
	free(*value);
	*value = p;
	return 0;
}


static int _config_save_boolean(Config * config, char const * variable,
		gboolean value)
{
	return config_set(config, "", variable, value ? "1" : "0");
}
