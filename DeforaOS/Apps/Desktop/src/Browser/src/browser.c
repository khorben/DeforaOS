/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2006-2012 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Browser */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";
/* TODO:
 * - re-implement MIME preferences
 * - use the friendly-name for MIME types in the browser view
 * - allow plug-ins to be re-ordered */



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
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

#define COMMON_CONFIG_FILENAME
#include "common.c"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef BINDIR
# define BINDIR		PREFIX "/bin"
#endif
#ifndef LIBDIR
# define LIBDIR		PREFIX "/lib"
#endif

#define IDLE_LOOP_ICON_CNT	16	/* number of icons added in a loop */
#define ICON_NAME		"system-file-manager"


/* Browser */
/* private */
/* types */
typedef enum _BrowserMimeColumn
{
	BMC_ICON,
	BMC_NAME
} BrowserMimeColumn;
#define BMC_LAST BMC_NAME
#define BMC_COUNT (BMC_LAST + 1)

typedef enum _BrowserPluginColumn
{
	BPC_NAME = 0,
	BPC_ENABLED,
	BPC_ICON,
	BPC_NAME_DISPLAY,
	BPC_PLUGIN,
	BPC_BROWSERPLUGINDEFINITION,
	BPC_BROWSERPLUGIN,
	BPC_WIDGET
} BrowserPluginColumn;
#define BPC_LAST BPC_WIDGET
#define BPC_COUNT (BPC_LAST + 1)


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

static const DesktopAccel _browser_accel[] =
{
	{ G_CALLBACK(on_location), GDK_CONTROL_MASK, GDK_KEY_L },
	{ G_CALLBACK(on_properties), GDK_MOD1_MASK, GDK_KEY_Return },
#ifdef EMBEDDED
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(on_copy), GDK_CONTROL_MASK, GDK_KEY_C },
	{ G_CALLBACK(on_cut), GDK_CONTROL_MASK, GDK_KEY_X },
	{ G_CALLBACK(on_new_window), GDK_CONTROL_MASK, GDK_KEY_N },
	{ G_CALLBACK(on_open_file), GDK_CONTROL_MASK, GDK_KEY_O },
	{ G_CALLBACK(on_paste), GDK_CONTROL_MASK, GDK_KEY_V },
	{ G_CALLBACK(on_refresh), GDK_CONTROL_MASK, GDK_KEY_R },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static const DesktopMenu _browser_menu_file[] =
{
	{ N_("_New window"), G_CALLBACK(on_file_new_window), "window-new",
		GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("New _folder"), G_CALLBACK(on_file_new_folder), "folder-new", 0,
		0 },
	{ N_("New _symbolic link..."), G_CALLBACK(on_file_new_symlink), NULL,
		0, 0 },
	{ N_("_Open file..."), G_CALLBACK(on_file_open_file), NULL,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Properties"), G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES,
		GDK_MOD1_MASK, GDK_KEY_Return },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _browser_menu_edit[] =
{
	{ N_("_Cut"), G_CALLBACK(on_edit_cut), GTK_STOCK_CUT, GDK_CONTROL_MASK,
		GDK_KEY_X },
	{ N_("Cop_y"), G_CALLBACK(on_edit_copy), GTK_STOCK_COPY,
		GDK_CONTROL_MASK, GDK_KEY_C },
	{ N_("_Paste"), G_CALLBACK(on_edit_paste), GTK_STOCK_PASTE,
		GDK_CONTROL_MASK, GDK_KEY_V },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Delete"), G_CALLBACK(on_edit_delete), GTK_STOCK_DELETE, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("Select _all"), G_CALLBACK(on_edit_select_all),
#if GTK_CHECK_VERSION(2, 10, 0)
		GTK_STOCK_SELECT_ALL,
#else
		"edit-select-all",
#endif
		GDK_CONTROL_MASK, GDK_KEY_A },
	{ N_("_Unselect all"), G_CALLBACK(on_edit_unselect_all), NULL, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Preferences"), G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _browser_menu_view[] =
{
	{ N_("_Refresh"), G_CALLBACK(on_refresh), GTK_STOCK_REFRESH,
		GDK_CONTROL_MASK, GDK_KEY_R },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Home"), G_CALLBACK(on_view_home), GTK_STOCK_HOME, GDK_MOD1_MASK,
		GDK_KEY_Home },
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Details"), G_CALLBACK(on_view_details), "browser-view-details",
		0, 0 },
	{ N_("_Icons"), G_CALLBACK(on_view_icons), "browser-view-icons", 0, 0 },
	{ N_("_List"), G_CALLBACK(on_view_list), "browser-view-list", 0, 0 },
	{ N_("_Thumbnails"), G_CALLBACK(on_view_thumbnails), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _browser_menu_help[] =
{
	{ N_("_Contents"), G_CALLBACK(on_help_contents), "help-contents", 0,
		GDK_KEY_F1 },
#if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
#else
	{ N_("_About"), G_CALLBACK(on_help_about), NULL, 0, 0 },
#endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenubar _browser_menubar[] =
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
	{ N_("Back"), G_CALLBACK(on_back), GTK_STOCK_GO_BACK, GDK_MOD1_MASK,
		GDK_KEY_Left, NULL },
	{ N_("Up"), G_CALLBACK(on_updir), GTK_STOCK_GO_UP, 0, 0, NULL },
	{ N_("Forward"), G_CALLBACK(on_forward), GTK_STOCK_GO_FORWARD,
		GDK_MOD1_MASK, GDK_KEY_Right, NULL },
	{ N_("Refresh"), G_CALLBACK(on_refresh), GTK_STOCK_REFRESH, 0, 0,
		NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Home"), G_CALLBACK(on_home), GTK_STOCK_HOME, 0, 0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Cut"), G_CALLBACK(on_cut), GTK_STOCK_CUT, 0, 0, NULL },
	{ N_("Copy"), G_CALLBACK(on_copy), GTK_STOCK_COPY, 0, 0, NULL },
	{ N_("Paste"), G_CALLBACK(on_paste), GTK_STOCK_PASTE, 0, 0, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Properties"), G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES, 0,
		0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};


/* variables */
unsigned int browser_cnt = 0;


/* prototypes */
/* accessors */
static gboolean _browser_plugin_is_enabled(Browser * browser,
		char const * plugin);
static Mime * _browser_get_mime(Browser * browser);
static void _browser_set_status(Browser * browser, char const * status);

static DIR * _browser_opendir(char const * pathname, struct stat * st);
static void _browser_plugin_refresh(Browser * browser);
static void _browser_refresh_do(Browser * browser, DIR * dir, struct stat * st);

static int _config_load_boolean(Config * config, char const * variable,
		gboolean * value);
static int _config_load_string(Config * config, char const * variable,
		char ** value);
static int _config_save_boolean(Config * config, char const * variable,
		gboolean value);

/* callbacks */
static void _browser_on_plugin_combo_change(gpointer data);
static void _browser_on_selection_changed(gpointer data);


/* public */
/* functions */
/* browser_new */
static gboolean _new_idle(gpointer data);
static void _idle_load_plugins(Browser * browser);
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
	GtkWidget * hpaned;
	GtkCellRenderer * renderer;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * menu;
	GtkWidget * menuitem;
#endif
	char * p;

	if((browser = malloc(sizeof(*browser))) == NULL)
	{
		browser_error(NULL, directory != NULL ? directory : ".", 1);
		return NULL;
	}
	browser->window = NULL;
	if(_new_pixbufs(browser) != 0)
		browser_error(browser, _("Error while loading default icons"),
				1);

	/* config */
	/* set defaults */
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->prefs.default_view = BV_ICONS;
#endif
	browser->prefs.alternate_rows = TRUE;
	browser->prefs.confirm_before_delete = TRUE;
	browser->prefs.sort_folders_first = TRUE;
	browser->prefs.show_hidden_files = FALSE;
	if((browser->config = config_new()) == NULL
			|| browser_config_load(browser) != 0)
		browser_error(browser, _("Error while loading configuration"),
				1);

	/* mime */
	browser->mime = mime_new(NULL); /* FIXME share MIME instances */

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

	/* plug-ins */
	browser->pl_helper.browser = browser;
	browser->pl_helper.error = browser_error;
	browser->pl_helper.get_mime = _browser_get_mime;
	browser->pl_helper.set_location = browser_set_location;

	/* widgets */
	group = gtk_accel_group_new();
	browser->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(browser->window), group);
	gtk_window_set_default_size(GTK_WINDOW(browser->window), 720, 480);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(browser->window), ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(browser->window), _("File manager"));
	g_signal_connect_swapped(browser->window, "delete-event", G_CALLBACK(
				on_closex), browser);
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
	g_signal_connect_swapped(toolitem, "clicked", G_CALLBACK(on_view_as),
			browser);
	menu = gtk_menu_new();
	menuitem = gtk_image_menu_item_new_with_label(_("Details"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
			gtk_image_new_from_icon_name("browser-view-details",
				GTK_ICON_SIZE_MENU));
	g_signal_connect_swapped(menuitem, "activate",
			G_CALLBACK(on_view_details), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label(_("Icons"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
			gtk_image_new_from_icon_name("browser-view-icons",
				GTK_ICON_SIZE_MENU));
	g_signal_connect_swapped(menuitem, "activate",
			G_CALLBACK(on_view_icons), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label(_("List"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
			gtk_image_new_from_icon_name("browser-view-list",
				GTK_ICON_SIZE_MENU));
	g_signal_connect_swapped(menuitem, "activate", G_CALLBACK(on_view_list),
			browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Thumbnails"));
	g_signal_connect_swapped(menuitem, "activate",
			G_CALLBACK(on_view_thumbnails), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(toolitem), menu);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
#ifdef EMBEDDED
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	g_signal_connect_swapped(toolitem, "clicked",
			G_CALLBACK(on_edit_preferences), browser);
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
#if GTK_CHECK_VERSION(3, 0, 0)
	browser->tb_path = gtk_combo_box_text_new_with_entry();
#else
	browser->tb_path = gtk_combo_box_entry_new_text();
#endif
	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if(directory != NULL)
		gtk_entry_set_text(GTK_ENTRY(widget), directory);
	g_signal_connect_swapped(widget, "activate",
			G_CALLBACK(on_path_activate), browser);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), browser->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect_swapped(toolitem, "clicked",
			G_CALLBACK(on_path_activate), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(2, 18, 0)
	/* infobar */
	browser->infobar = gtk_info_bar_new_with_buttons(GTK_STOCK_CLOSE,
			GTK_RESPONSE_CLOSE, NULL);
	gtk_info_bar_set_message_type(GTK_INFO_BAR(browser->infobar),
			GTK_MESSAGE_ERROR);
	g_signal_connect(browser->infobar, "close", G_CALLBACK(gtk_widget_hide),
			NULL);
	g_signal_connect(browser->infobar, "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	widget = gtk_info_bar_get_content_area(GTK_INFO_BAR(browser->infobar));
	browser->infobar_label = gtk_label_new(NULL);
	gtk_widget_show(browser->infobar_label);
	gtk_box_pack_start(GTK_BOX(widget), browser->infobar_label, TRUE, TRUE,
			0);
	gtk_widget_set_no_show_all(browser->infobar, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), browser->infobar, FALSE, TRUE, 0);
#endif
	/* paned */
	hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 200);
	/* plug-ins */
	browser->pl_view = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(browser->pl_view), 4);
	browser->pl_store = gtk_list_store_new(BPC_COUNT, G_TYPE_STRING,
			G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER,
			G_TYPE_POINTER);
	browser->pl_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(
				browser->pl_store));
	g_signal_connect_swapped(browser->pl_combo, "changed",
			G_CALLBACK(_browser_on_plugin_combo_change), browser);
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->pl_combo),
			renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->pl_combo),
			renderer, "pixbuf", BPC_ICON, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->pl_combo),
			renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->pl_combo),
			renderer, "text", BPC_NAME_DISPLAY, NULL);
	gtk_box_pack_start(GTK_BOX(browser->pl_view), browser->pl_combo, FALSE,
			TRUE, 0);
	browser->pl_box = gtk_vbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(browser->pl_view), browser->pl_box, TRUE,
			TRUE, 0);
	gtk_paned_add1(GTK_PANED(hpaned), browser->pl_view);
	gtk_widget_set_no_show_all(browser->pl_view, TRUE);
	/* view */
	browser->scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(browser->scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_add2(GTK_PANED(hpaned), browser->scrolled);
	gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
	/* statusbar */
	browser->statusbar = gtk_statusbar_new();
	browser->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), browser->statusbar, FALSE, TRUE, 0);
	/* store */
	browser->store = _create_store(browser);
	browser->detailview = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->iconview = NULL;
	browser->view = browser->prefs.default_view;
	browser_set_view(browser, browser->view);
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
	char const * location;

	_idle_load_plugins(browser);
	if((location = browser_get_location(browser)) == NULL)
		browser_go_home(browser);
	else
		browser_set_location(browser, location);
	return FALSE;
}

static void _idle_load_plugins(Browser * browser)
{
	char const * plugins;
	char * p;
	char * q;
	size_t i;

	if((plugins = config_get(browser->config, NULL, "plugins")) == NULL
			|| strlen(plugins) == 0)
		return;
	if((p = strdup(plugins)) == NULL)
		return; /* XXX report error */
	for(q = p, i = 0;;)
	{
		if(q[i] == '\0')
		{
			browser_load(browser, q);
			break;
		}
		if(q[i++] != ',')
			continue;
		q[i - 1] = '\0';
		browser_load(browser, q);
		q += i;
		i = 0;
	}
	free(p);
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

	gtk_tree_model_get(model, a, BC_IS_DIRECTORY, &is_dir_a,
			BC_DISPLAY_NAME, &name_a, -1);
	gtk_tree_model_get(model, b, BC_IS_DIRECTORY, &is_dir_b,
			BC_DISPLAY_NAME, &name_b, -1);
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

	store = gtk_list_store_new(BC_COUNT, G_TYPE_BOOLEAN, G_TYPE_STRING,
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


/* browser_new_copy */
Browser * browser_new_copy(Browser * browser)
{
	char const * location;

	if(browser == NULL)
		return browser_new(NULL);
	location = browser_get_location(browser);
	return browser_new(location);
}


/* browser_delete */
static void _delete_plugins(Browser * browser);

void browser_delete(Browser * browser)
{
	_delete_plugins(browser);
	if(browser->config != NULL)
		config_delete(browser->config);
	gtk_widget_hide(browser->window);
	if(browser->refresh_id)
		g_source_remove(browser->refresh_id);
	g_list_foreach(browser->history, (GFunc)free, NULL);
	g_list_free(browser->history);
	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	if(browser->detailview != NULL)
		g_object_unref(browser->detailview);
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
		g_object_unref(browser->iconview);
#endif
	g_object_unref(browser->store);
	gtk_widget_destroy(browser->window);
	free(browser);
	browser_cnt--;
}

static void _delete_plugins(Browser * browser)
{
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pl_store);
	GtkTreeIter iter;
	gboolean valid;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	Plugin * plugin;

	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BPC_PLUGIN, &plugin,
				BPC_BROWSERPLUGINDEFINITION, &bpd,
				BPC_BROWSERPLUGIN, &bp, -1);
		if(bpd->destroy != NULL)
			bpd->destroy(bp);
		plugin_delete(plugin);
	}
}


/* useful */
/* browser_about */
static gboolean _about_on_closex(gpointer data);

void browser_about(Browser * browser)
{
	if(browser->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(browser->ab_window));
		return;
	}
	browser->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(browser->ab_window), GTK_WINDOW(
				browser->window));
	desktop_about_dialog_set_authors(browser->ab_window, _authors);
	desktop_about_dialog_set_comments(browser->ab_window,
			_("File manager for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(browser->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(browser->ab_window,
			"system-file-manager");
	desktop_about_dialog_set_license(browser->ab_window, _license);
	desktop_about_dialog_set_name(browser->ab_window, PACKAGE);
	desktop_about_dialog_set_translator_credits(browser->ab_window,
			_("translator-credits"));
	desktop_about_dialog_set_version(browser->ab_window, VERSION);
	desktop_about_dialog_set_website(browser->ab_window,
			"http://www.defora.org/");
	g_signal_connect_swapped(browser->ab_window, "delete-event",
			G_CALLBACK(_about_on_closex), browser);
	gtk_widget_show(browser->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Browser * browser = data;

	gtk_widget_hide(browser->ab_window);
	return TRUE;
}


/* browser_error */
static int _browser_error(char const * message, int ret);
/* callbacks */
#if !GTK_CHECK_VERSION(2, 18, 0)
static void _error_response(gpointer data);
#endif

int browser_error(Browser * browser, char const * message, int ret)
{
#if !GTK_CHECK_VERSION(2, 18, 0)
	GtkWidget * dialog;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", %d) errno=%d\n", __func__, message,
			ret, errno);
#endif
	if(browser == NULL)
		return _browser_error(message, ret);
#if GTK_CHECK_VERSION(2, 18, 0)
	gtk_label_set_text(GTK_LABEL(browser->infobar_label), message);
	gtk_widget_show(browser->infobar);
#else
	dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
# if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
# endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	if(ret < 0)
	{
		g_signal_connect_swapped(dialog, "response",
				G_CALLBACK(_error_response), browser);
		ret = -ret;
	}
	else
		g_signal_connect(dialog, "response", G_CALLBACK(
					gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
#endif
	return ret;
}

static int _browser_error(char const * message, int ret)
{
	fprintf(stderr, "%s: %s\n", "browser", message);
	return ret;
}

#if !GTK_CHECK_VERSION(2, 18, 0)
static void _error_response(gpointer data)
{
	Browser * browser = data;

	if(browser_cnt > 0) /* XXX ugly */
		browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
}
#endif


/* browser_config_load */
int browser_config_load(Browser * browser)
{
	char * filename;
#if GTK_CHECK_VERSION(2, 6, 0)
	char * p = NULL;
#endif

	if(browser->config == NULL)
		return 0; /* XXX ignore error */
	if((filename = _common_config_filename(BROWSER_CONFIG_FILE)) == NULL)
		return 1;
	if(config_load(browser->config, filename) != 0)
		browser_error(NULL, error_get(), 1);
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
	_config_load_boolean(browser->config, "alternate_rows",
			&browser->prefs.alternate_rows);
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
	if((filename = _common_config_filename(BROWSER_CONFIG_FILE)) == NULL)
		return 1;
#if GTK_CHECK_VERSION(2, 6, 0)
	/* XXX deserves a rework (enum) */
	if(browser->prefs.default_view >= BV_FIRST
			&& browser->prefs.default_view <= BV_LAST)
		ret |= config_set(browser->config, NULL, "default_view",
				str[browser->prefs.default_view]);
#endif
	ret |= _config_save_boolean(browser->config, "alternate_rows",
			browser->prefs.alternate_rows);
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


/* browser_get_location */
char const * browser_get_location(Browser * browser)
{
	if(browser->current == NULL)
		return NULL;
	return browser->current->data;
}


/* browser_get_view */
BrowserView browser_get_view(Browser * browser)
{
	return browser->view;
}


/* browser_go_home */
void browser_go_home(Browser * browser)
{
	char const * home;

	if((home = getenv("HOME")) == NULL)
		home = g_get_home_dir();
	/* XXX use open while set_location should only update the toolbar? */
	browser_set_location(browser, (home != NULL) ? home : "/");
}


/* browser_load */
int browser_load(Browser * browser, char const * plugin)
{
	Plugin * p;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	GtkWidget * widget;
	GtkTreeIter iter;
	GtkIconTheme * theme;
	GdkPixbuf * icon = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, plugin);
#endif
	if(_browser_plugin_is_enabled(browser, plugin))
		return 0;
	if((p = plugin_new(LIBDIR, PACKAGE, "plugins", plugin)) == NULL)
		return -browser_error(NULL, error_get(), 1);
	if((bpd = plugin_lookup(p, "plugin")) == NULL)
	{
		plugin_delete(p);
		return -browser_error(NULL, error_get(), 1);
	}
	if(bpd->init == NULL || bpd->destroy == NULL || bpd->get_widget == NULL
			|| (bp = bpd->init(&browser->pl_helper)) == NULL)
	{
		plugin_delete(p);
		return -browser_error(NULL, error_get(), 1);
	}
	widget = bpd->get_widget(bp);
	gtk_widget_hide(widget);
	theme = gtk_icon_theme_get_default();
	if(bpd->icon != NULL)
		icon = gtk_icon_theme_load_icon(theme, bpd->icon, 24, 0, NULL);
	if(icon == NULL)
		icon = gtk_icon_theme_load_icon(theme, "gnome-settings", 24, 0,
				NULL);
	gtk_list_store_append(browser->pl_store, &iter);
	gtk_list_store_set(browser->pl_store, &iter, BPC_NAME, plugin,
			BPC_ICON, icon, BPC_NAME_DISPLAY, _(bpd->name),
			BPC_PLUGIN, p, BPC_BROWSERPLUGINDEFINITION, bpd,
			BPC_BROWSERPLUGIN, bp, BPC_WIDGET, widget, -1);
	gtk_box_pack_start(GTK_BOX(browser->pl_box), widget, TRUE, TRUE, 0);
	if(gtk_widget_get_no_show_all(browser->pl_view) == TRUE)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(browser->pl_combo), 0);
		gtk_widget_set_no_show_all(browser->pl_view, FALSE);
		gtk_widget_show_all(browser->pl_view);
	}
	return 0;
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
	GtkFileFilter * filter;
	char * filename = NULL;
	pid_t pid;

	dialog = gtk_file_chooser_dialog_new(_("Open with..."),
			GTK_WINDOW(browser->window),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT, NULL);
	/* set the default folder to BINDIR */
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), BINDIR);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Executable files"));
	gtk_file_filter_add_mime_type(filter, "application/x-executable");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Shell scripts"));
	gtk_file_filter_add_mime_type(filter, "application/x-shellscript");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return;
	if((pid = fork()) == -1)
		browser_error(browser, strerror(errno), 1);
	else if(pid == 0)
	{
		if(close(0) != 0)
			browser_error(NULL, strerror(errno), 0);
		execlp(filename, filename, path, NULL);
		browser_error(NULL, strerror(errno), 0);
		exit(2);
	}
	g_free(filename);
}


/* browser_refresh */
void browser_refresh(Browser * browser)
{
	char const * location;
	DIR * dir;
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %s\n", __func__, (browser->current != NULL
				&& browser->current->data != NULL)
			? (char *)browser->current->data : "NULL");
#endif
	if((location = browser_get_location(browser)) == NULL)
		return;
	if((dir = _browser_opendir(location, &st)) == NULL) /* XXX */
		browser_error(browser, strerror(errno), 1);
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
static int _loop_status(Browser * browser, char const * prefix);
static void _loop_insert(Browser * browser, GtkTreeIter * iter,
		char const * path, char const * display, struct stat * lst,
		struct stat * st, gboolean updated);

static int _refresh_new_loop(Browser * browser)
{
	struct dirent * de;
	char const * location;
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
		return _loop_status(browser, NULL);
	_loop_status(browser, _("Refreshing folder: "));
	location = browser_get_location(browser);
	if((path = g_build_filename(location, de->d_name, NULL)) == NULL
			|| lstat(path, &lst) != 0)
	{
		browser_error(NULL, strerror(errno), 1);
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

static int _loop_status(Browser * browser, char const * prefix)
{
	char status[64];

	snprintf(status, sizeof(status), _("%s%u file%c (%u hidden)"),
			(prefix != NULL) ? prefix : "",
			browser->refresh_cnt, browser->refresh_cnt <= 1
			? '\0' : 's', browser->refresh_hid); /* XXX translate */
	_browser_set_status(browser, status);
	return 1;
}


/* _loop_insert */
static void _insert_all(Browser * browser, struct stat * lst, struct stat * st,
		char const ** display, uint64_t * inode, uint64_t * size,
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
			BC_UPDATED, updated, BC_PATH, path,
			BC_DISPLAY_NAME, display, BC_INODE, inode,
			BC_IS_DIRECTORY, S_ISDIR(st->st_mode),
			BC_IS_EXECUTABLE, st->st_mode & S_IXUSR,
			BC_IS_MOUNT_POINT,
			(st->st_dev != browser->refresh_dev) ? TRUE : FALSE,
			BC_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BC_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
			BC_PIXBUF_96, icon_96 != NULL ? icon_96
			: browser->pb_file_96,
#endif
			BC_SIZE, size, BC_DISPLAY_SIZE, dsize,
			BC_OWNER, pw != NULL ? pw->pw_name : "",
			BC_GROUP, gr != NULL ? gr->gr_name : "",
			BC_DATE, lst->st_mtime, BC_DISPLAY_DATE, ddate,
			BC_MIME_TYPE, type != NULL ? type : "", -1);
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
		char const ** display, uint64_t * inode, uint64_t * size,
		char const ** dsize, struct passwd ** pw, struct group ** gr,
		char const ** ddate, char const ** type, char const * path,
		GdkPixbuf ** icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
		GdkPixbuf ** icon_48, GdkPixbuf ** icon_96
#endif
		)
{
	char const * p;
	GError * error = NULL;

	if((p = g_filename_to_utf8(*display, -1, NULL, NULL, &error)) == NULL)
	{
		browser_error(NULL, error->message, 1);
		g_error_free(error);
	}
	else
		*display = p; /* XXX memory leak */
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
		mime_icons(browser->mime, "application/x-executable", 24,
				icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
				48, icon_48, 96, icon_96,
#endif
				-1);
	else if(browser->mime != NULL && *type == NULL
			&& (*type = mime_type(browser->mime, path)) != NULL)
		mime_icons(browser->mime, *type, 24, icon_24,
#if GTK_CHECK_VERSION(2, 6, 0)
				48, icon_48, 96, icon_96,
#endif
				-1);
}

static char const * _insert_size(off_t size)
{
	static char buf[11];
	double sz = size;
	char * unit;

	if(sz < 1024)
	{
		snprintf(buf, sizeof(buf), "%.0f %s", sz, _("bytes"));
		return buf;
	}
	else if((sz /= 1024) < 1024)
		unit = N_("kB");
	else if((sz /= 1024) < 1024)
		unit = N_("MB");
	else if((sz /= 1024) < 1024)
		unit = N_("GB");
	else
	{
		sz /= 1024;
		unit = N_("TB");
	}
	snprintf(buf, sizeof(buf), "%.1f %s", sz, _(unit));
	return buf;
}

static char const * _insert_date(time_t date)
{
	static char buf[16];
	time_t sixmonths; /* XXX set it per refresh */
	struct tm tm;
	size_t len;

	sixmonths = time(NULL) - 15552000;
	localtime_r(&date, &tm);
	if(date < sixmonths)
		len = strftime(buf, sizeof(buf), "%b %e %Y", &tm);
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
	char * rmt = "mount-point";

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

#if GTK_CHECK_VERSION(2, 6, 0)
static gboolean _done_thumbnails(gpointer data);
#endif
static gboolean _done_timeout(gpointer data);
static void _refresh_done(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter * iter = &browser->refresh_iter;
#endif

	closedir(browser->refresh_dir);
	browser->refresh_dir = NULL;
#if GTK_CHECK_VERSION(2, 6, 0)
	if(gtk_tree_model_get_iter_first(model, iter) == TRUE)
		browser->refresh_id = g_idle_add(_done_thumbnails, browser);
	else
#endif
		browser->refresh_id = g_timeout_add(1000, _done_timeout,
				browser);
}

#if GTK_CHECK_VERSION(2, 6, 0)
static gboolean _done_thumbnails(gpointer data)
{
	Browser * browser = data;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter * iter = &browser->refresh_iter;
	size_t i;
	char * type;
	char * path;
	GdkPixbuf * icon;
	GError * error = NULL;

	for(i = 0; i < IDLE_LOOP_ICON_CNT; i++)
	{
		gtk_tree_model_get(model, iter, BC_MIME_TYPE, &type,
				BC_PATH, &path, -1);
		if(type != NULL && path != NULL
				&& strncmp(type, "image/", 6) == 0)
		{
			if((icon = gdk_pixbuf_new_from_file_at_size(path, 96,
							96, &error)) == NULL)
			{
				browser_error(NULL, error->message, 1);
				g_error_free(error);
				error = NULL;
			}
			else
				gtk_list_store_set(browser->store, iter,
						BC_PIXBUF_96, icon, -1);
		}
		free(type);
		free(path);
		if(gtk_tree_model_iter_next(model, iter) != TRUE)
			break;
	}
	if(i == IDLE_LOOP_ICON_CNT)
		return TRUE;
	browser->refresh_id = g_timeout_add(1000, _done_timeout, browser);
	return FALSE;
}
#endif

static gboolean _done_timeout(gpointer data)
{
	Browser * browser = data;
	char const * location;
	struct stat st;

	if((location = browser_get_location(browser)) == NULL)
	{
		browser->refresh_id = 0;
		return FALSE;
	}
	if(stat(location, &st) != 0)
	{
		browser->refresh_id = 0;
		browser_error(NULL, strerror(errno), 1);
		return FALSE;
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
	char const * location;
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
		return _loop_status(browser, NULL);
	_loop_status(browser, _("Refreshing folder: "));
	location = browser_get_location(browser);
	if((path = g_build_filename(location, de->d_name, NULL)) == NULL
			|| lstat(path, &lst) != 0)
	{
		browser_error(NULL, strerror(errno), 1);
		if(path != NULL)
			g_free(path);
		return 0;
	}
	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BC_INODE, &inode, -1);
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
	gtk_list_store_set(browser->store, iter, BC_UPDATED, 1, BC_PATH, path,
			BC_DISPLAY_NAME, display, BC_INODE, inode,
			BC_IS_DIRECTORY, S_ISDIR(st->st_mode),
			BC_IS_EXECUTABLE, st->st_mode & S_IXUSR,
			BC_IS_MOUNT_POINT, (st->st_dev != browser->refresh_dev)
			? TRUE : FALSE,
			BC_PIXBUF_24, icon_24 != NULL ? icon_24
			: browser->pb_file_24,
#if GTK_CHECK_VERSION(2, 6, 0)
			BC_PIXBUF_48, icon_48 != NULL ? icon_48
			: browser->pb_file_48,
#endif
			BC_SIZE, size, BC_DISPLAY_SIZE, dsize,
			BC_OWNER, (pw != NULL) ? pw->pw_name : "",
			BC_GROUP, (gr != NULL) ? gr->gr_name : "",
			BC_DATE, lst->st_mtime, BC_DISPLAY_DATE, ddate,
			BC_MIME_TYPE, type != NULL ? type : "", -1);
	/* FIXME refresh the plug-in if the icon is currently selected */
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
		gtk_tree_model_get(model, &iter, BC_UPDATED, &updated, -1);
		gtk_list_store_set(browser->store, &iter, BC_UPDATED, FALSE,
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
	if(browser_get_view(browser) != BV_DETAILS)
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

int browser_set_location(Browser * browser, char const * path)
{
	int ret = 0;
	char * realpath = NULL;
	DIR * dir;
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, path);
#endif
	if((realpath = _location_real_path(path)) == NULL)
		return -1;
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
		ret = -browser_error(browser, strerror(errno), 1);
	free(realpath);
	return ret;
}

static char * _location_real_path(char const * path)
{
	char * p;
	char * cur;
	size_t i;

	if(path == NULL)
		return NULL;
	if(path[0] == '/')
	{
		if((p = strdup(path)) == NULL)
			return NULL;
	}
	else
	{
		cur = g_get_current_dir();
		p = g_build_filename(cur, path, NULL);
		g_free(cur);
	}
	/* replace "/./" by "/" */
	for(i = strlen(p); (cur = strstr(p, "/./")) != NULL; i = strlen(p))
		memmove(cur, &cur[2], (p + i) - (cur + 1));
	/* replace "//" by "/" */
	for(i = strlen(p); (cur = strstr(p, "//")) != NULL; i = strlen(p))
		memmove(cur, &cur[1], (p + i) - (cur));
	/* remove single dots at the end of the address */
	i = strlen(p);
	if(i >= 2 && strcmp(&p[i - 2], "/.") == 0)
		p[i - 1] = '\0';
	/* trim slashes in the end */
	for(i = strlen(p); i > 1 && p[--i] == '/'; p[i] = '\0');
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
static void _details_column_text(GtkTreeView * view, GtkCellRenderer * renderer,
		char const * title, int id, int sort);
#if GTK_CHECK_VERSION(2, 6, 0)
static void _view_icons(Browser * browser);
static void _view_list(Browser * browser);
static void _view_thumbnails(Browser * browser);
#endif

void browser_set_view(Browser * browser, BrowserView view)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, view);
#endif
#if GTK_CHECK_VERSION(2, 6, 0)
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
	browser->view = view;
}

static void _view_details(Browser * browser)
{
	GtkTreeSelection * treesel;
	GtkTreeViewColumn * column;
	GtkCellRenderer * renderer;
	GtkTreeView * view;
#if GTK_CHECK_VERSION(2, 6, 0)
	GList * sel = NULL;
	GList * p;
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, browser->view);
#endif
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->view != BV_DETAILS)
	{
		sel = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
					browser->iconview));
		if(browser->iconview != NULL)
			gtk_container_remove(GTK_CONTAINER(browser->scrolled),
					browser->iconview);
		if(browser->detailview != NULL)
			gtk_container_add(GTK_CONTAINER(browser->scrolled),
					browser->detailview);
	}
#endif
	if(browser->detailview != NULL)
	{
		gtk_widget_show(browser->detailview);
		return;
	}
	browser->detailview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	g_object_ref(browser->detailview);
	view = GTK_TREE_VIEW(browser->detailview);
	gtk_tree_view_set_rules_hint(view, browser->prefs.alternate_rows);
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
		g_signal_connect_swapped(treesel, "changed",
				G_CALLBACK(_browser_on_selection_changed),
				browser);
	}
	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer,
			"pixbuf", BC_PIXBUF_24, NULL);
	gtk_tree_view_append_column(view, column);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, "ellipsize",
			PANGO_ELLIPSIZE_END, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(on_filename_edited),
			browser);
	_details_column_text(view, renderer, _("Filename"), BC_DISPLAY_NAME,
			BC_DISPLAY_NAME);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	_details_column_text(view, renderer, _("Size"), BC_DISPLAY_SIZE,
			BC_SIZE);
	_details_column_text(view, NULL, _("Owner"), BC_OWNER, BC_OWNER);
	_details_column_text(view, NULL, _("Group"), BC_GROUP, BC_GROUP);
	_details_column_text(view, NULL, _("Date"), BC_DISPLAY_DATE, BC_DATE);
	_details_column_text(view, NULL, _("MIME type"), BC_MIME_TYPE,
			BC_MIME_TYPE);
	gtk_tree_view_set_headers_visible(view, TRUE);
	g_signal_connect(view, "row-activated", G_CALLBACK(on_detail_default),
			browser);
	g_signal_connect(view, "button-press-event", G_CALLBACK(on_view_press),
			browser);
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
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_tree_view_column_set_expand(column, TRUE);
#endif
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, sort);
	gtk_tree_view_append_column(view, column);
}

#if GTK_CHECK_VERSION(2, 6, 0)
static void _view_icon_view(Browser * browser);
static void _view_icon_selection(Browser * browser, GList * sel);

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
			renderer, "pixbuf", BC_PIXBUF_48, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 16, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.5, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(on_filename_edited),
			browser);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BC_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BC_PIXBUF_48);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BC_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_ICON_WRAP_WIDTH);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_icon_view_set_item_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
#else
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
#endif
}

static void _view_icon_view(Browser * browser)
{
	GtkTreeSelection * treesel;
	GList * sel = NULL;
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkTargetEntry targets[] = { { "deforaos_browser_dnd", 0, 0 } };
	size_t targets_cnt = sizeof(targets) / sizeof(*targets);
#endif

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %u\n", __func__, browser->view);
#endif
	if(browser->view == BV_DETAILS)
	{
		if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							browser->detailview)))
				!= NULL)
			sel = gtk_tree_selection_get_selected_rows(treesel,
					NULL);
		if(browser->detailview != NULL)
			gtk_container_remove(GTK_CONTAINER(browser->scrolled),
					browser->detailview);
		if(browser->iconview != NULL)
			gtk_container_add(GTK_CONTAINER(browser->scrolled),
					browser->iconview);
	}
	if(browser->iconview != NULL)
	{
#if GTK_CHECK_VERSION(2, 8, 0)
		gtk_cell_layout_clear(GTK_CELL_LAYOUT(browser->iconview));
#endif
		_view_icon_selection(browser, sel);
		gtk_widget_show(browser->iconview);
		return;
	}
	browser->iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	g_object_ref(browser->iconview);
	/* this needs to be done now */
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(browser->iconview),
			GTK_SELECTION_MULTIPLE);
	g_signal_connect_swapped(browser->iconview, "selection-changed",
			G_CALLBACK(_browser_on_selection_changed), browser);
	_view_icon_selection(browser, sel);
#if GTK_CHECK_VERSION(2, 8, 0)
	gtk_icon_view_enable_model_drag_source(GTK_ICON_VIEW(browser->iconview),
			GDK_BUTTON1_MASK, targets, targets_cnt,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
	gtk_icon_view_enable_model_drag_dest(GTK_ICON_VIEW(browser->iconview),
			targets, targets_cnt,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
#endif
	g_signal_connect(browser->iconview, "item-activated",
			G_CALLBACK(on_icon_default), browser);
	g_signal_connect(browser->iconview, "button-press-event",
			G_CALLBACK(on_view_press), browser);
	g_signal_connect(browser->iconview, "popup-menu",
			G_CALLBACK(on_view_popup), browser);
#if GTK_CHECK_VERSION(2, 8, 0)
	g_signal_connect(browser->iconview, "drag-data-get",
			G_CALLBACK(on_view_drag_data_get), browser);
	g_signal_connect(browser->iconview, "drag-data-received",
			G_CALLBACK(on_view_drag_data_received), browser);
#endif
	gtk_container_add(GTK_CONTAINER(browser->scrolled), browser->iconview);
	gtk_widget_show(browser->iconview);
}

static void _view_icon_selection(Browser * browser, GList * sel)
{
	GList * p;

	if(sel == NULL)
		return;
	gtk_icon_view_unselect_all(GTK_ICON_VIEW(browser->iconview));
	for(p = sel; p != NULL; p = p->next)
		gtk_icon_view_select_path(GTK_ICON_VIEW(browser->iconview),
				p->data);
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
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
			renderer, "pixbuf", BC_PIXBUF_24, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 20, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.0, NULL);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BC_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BC_PIXBUF_24);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BC_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_LIST_WRAP_WIDTH + 24);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_icon_view_set_item_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
#else
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_HORIZONTAL);
#endif
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
			renderer, "pixbuf", BC_PIXBUF_96, NULL);
	renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(browser->iconview), renderer,
			TRUE);
	g_object_set(renderer, "editable", TRUE,
			"ellipsize", PANGO_ELLIPSIZE_END,
			"width-chars", 22, "wrap-mode", PANGO_WRAP_WORD_CHAR,
			"xalign", 0.5, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(on_filename_edited),
			browser);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(browser->iconview),
			renderer, "text", BC_DISPLAY_NAME, NULL);
#else
	_view_icon_view(browser);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BC_PIXBUF_96);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BC_DISPLAY_NAME);
	gtk_icon_view_set_item_width(GTK_ICON_VIEW(browser->iconview),
			BROWSER_THUMBNAIL_WRAP_WIDTH);
#endif /* !GTK_CHECK_VERSION(2, 8, 0) */
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_icon_view_set_item_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
#else
	gtk_icon_view_set_orientation(GTK_ICON_VIEW(browser->iconview),
			GTK_ORIENTATION_VERTICAL);
#endif
}
#endif


/* browser_show_preferences */
static void _preferences_set(Browser * browser);
static void _preferences_set_plugins(Browser * browser);
/* callbacks */
static void _preferences_on_mime_edit(gpointer data);
static void _preferences_on_mime_foreach(void * data, char const * name,
		GdkPixbuf * icon_24, GdkPixbuf * icon_48, GdkPixbuf * icon_96);
static void _preferences_on_plugin_toggled(GtkCellRendererToggle * renderer,
		char * path, gpointer data);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void browser_show_preferences(Browser * browser)
{
	GtkWidget * widget;
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * hbox;
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;

	if(browser->pr_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(browser->pr_window));
		return;
	}
	browser->pr_window = gtk_dialog_new_with_buttons(_("Preferences"),
			GTK_WINDOW(browser->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(browser->pr_window, "delete-event",
			G_CALLBACK(_preferences_on_closex), browser);
	g_signal_connect(browser->pr_window, "response",
			G_CALLBACK(_preferences_on_response), browser);
	/* notebook */
	notebook = gtk_notebook_new();
	/* appearance tab */
	vbox = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
#if GTK_CHECK_VERSION(2, 6, 0)
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Default view:"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
# if GTK_CHECK_VERSION(3, 0, 0)
	widget = gtk_combo_box_text_new();
	browser->pr_view = widget;
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL,
			_("Details"));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, _("Icons"));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL, _("List"));
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(widget), NULL,
			_("Thumbnails"));
# else
	widget = gtk_combo_box_new_text();
	browser->pr_view = widget;
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("Details"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("Icons"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("List"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(widget), _("Thumbnails"));
# endif
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), 1);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
#endif
	browser->pr_alternate = gtk_check_button_new_with_mnemonic(
			_("_Alternate row colors in detailed view"));
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_alternate, FALSE, FALSE,
			0);
	browser->pr_confirm = gtk_check_button_new_with_mnemonic(
			_("_Confirm before deletion"));
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_confirm, FALSE, FALSE, 0);
	browser->pr_sort = gtk_check_button_new_with_mnemonic(
			_("Sort _folders first"));
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_sort, FALSE, FALSE, 0);
	browser->pr_hidden = gtk_check_button_new_with_mnemonic(
			_("Show _hidden files"));
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_hidden, FALSE, FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox,
			gtk_label_new_with_mnemonic(_("_Appearance")));
	/* file associations tab */
	hbox = gtk_hbox_new(FALSE, 4);
	browser->pr_mime_store = gtk_list_store_new(BMC_COUNT, GDK_TYPE_PIXBUF,
			G_TYPE_STRING);
	browser->pr_mime_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->pr_mime_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(browser->pr_mime_view),
			FALSE);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_pixbuf_new(), "pixbuf", BMC_ICON,
			NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->pr_mime_view),
			column);
	column = gtk_tree_view_column_new_with_attributes(_("Type"),
			gtk_cell_renderer_text_new(), "text", BMC_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->pr_mime_view),
			column);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(
				browser->pr_mime_store), BMC_NAME,
			GTK_SORT_ASCENDING);
	mime_foreach(browser->mime, _preferences_on_mime_foreach, browser);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), browser->pr_mime_view);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	vbox = gtk_vbox_new(FALSE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	g_signal_connect_swapped(widget, "clicked",
			G_CALLBACK(_preferences_on_mime_edit), browser);
	gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox,
			gtk_label_new_with_mnemonic(_("_File associations")));
	/* plug-ins tab */
	browser->pr_plugin_store = gtk_list_store_new(BPC_COUNT, G_TYPE_STRING,
			G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING,
			G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER,
			G_TYPE_POINTER);
	browser->pr_plugin_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->pr_plugin_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(
				browser->pr_plugin_view), FALSE);
	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer, "toggled", G_CALLBACK(
				_preferences_on_plugin_toggled), browser);
	column = gtk_tree_view_column_new_with_attributes(_("Enabled"),
			renderer, "active", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->pr_plugin_view),
			column);
	column = gtk_tree_view_column_new_with_attributes(NULL,
			gtk_cell_renderer_pixbuf_new(), "pixbuf", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->pr_plugin_view),
			column);
	column = gtk_tree_view_column_new_with_attributes(_("Name"),
			gtk_cell_renderer_text_new(), "text", 3, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 3);
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->pr_plugin_view),
			column);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(
				browser->pr_plugin_store), BPC_NAME_DISPLAY,
			GTK_SORT_ASCENDING);
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), browser->pr_plugin_view);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new_with_mnemonic(_("_Plug-ins")));
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(browser->pr_window));
#else
	vbox = GTK_DIALOG(browser->pr_window)->vbox;
#endif
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	_preferences_set(browser);
	gtk_widget_show_all(browser->pr_window);
}

static void _preferences_set(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_combo_box_set_active(GTK_COMBO_BOX(browser->pr_view),
			browser->prefs.default_view);
#endif
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_alternate),
			browser->prefs.alternate_rows);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_confirm),
			browser->prefs.confirm_before_delete);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_sort),
			browser->prefs.sort_folders_first);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_hidden),
			browser->prefs.show_hidden_files);
	_preferences_set_plugins(browser);
}

static void _preferences_set_plugins(Browser * browser)
{
	DIR * dir;
	struct dirent * de;
	GtkIconTheme * theme;
#ifdef __APPLE__
	char const ext[] = ".dylib";
#else
	char const ext[] = ".so";
#endif
	size_t len;
	Plugin * p;
	BrowserPluginDefinition * bpd;
	GtkTreeIter iter;
	gboolean enabled;
	GdkPixbuf * icon;

	gtk_list_store_clear(browser->pr_plugin_store);
	if((dir = opendir(LIBDIR "/" PACKAGE "/plugins")) == NULL)
		return;
	theme = gtk_icon_theme_get_default();
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name)) < sizeof(ext))
			continue;
		if(strcmp(&de->d_name[len - sizeof(ext) + 1], ext) != 0)
			continue;
		de->d_name[len - sizeof(ext) + 1] = '\0';
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, de->d_name);
#endif
		if((p = plugin_new(LIBDIR, PACKAGE, "plugins", de->d_name))
				== NULL)
			continue;
		if((bpd = plugin_lookup(p, "plugin")) == NULL)
		{
			plugin_delete(p);
			continue;
		}
		enabled = _browser_plugin_is_enabled(browser, de->d_name);
		icon = NULL;
		if(bpd->icon != NULL)
			icon = gtk_icon_theme_load_icon(theme, bpd->icon, 24,
					0, NULL);
		if(icon == NULL)
			icon = gtk_icon_theme_load_icon(theme, "gnome-settings",
					24, 0, NULL);
		gtk_list_store_append(browser->pr_plugin_store, &iter);
		gtk_list_store_set(browser->pr_plugin_store, &iter,
				BPC_NAME, de->d_name, BPC_ENABLED, enabled,
				BPC_ICON, icon, BPC_NAME_DISPLAY, _(bpd->name),
				-1);
		plugin_delete(p);
	}
	closedir(dir);
}

static void _preferences_on_mime_edit(gpointer data)
{
	Browser * browser = data;
	GtkTreeSelection * selection;
	GtkTreeModel * model;
	GtkTreeIter iter;
	char * type;
	GtkWidget * dialog;
	int flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkWidget * open;
	GtkWidget * view;
	GtkWidget * edit;
	char const * p;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
				browser->pr_mime_view));
	if(gtk_tree_selection_get_selected(selection, &model, &iter) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, BMC_NAME, &type, -1);
	dialog = gtk_dialog_new_with_buttons(_("Edit file association"),
			GTK_WINDOW(browser->pr_window), flags, GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY,
			GTK_RESPONSE_APPLY, NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(vbox), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	/* type */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Type:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(type);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* open */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Open with:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	open = gtk_entry_new();
	p = mime_get_handler(browser->mime, type, "open");
	gtk_entry_set_text(GTK_ENTRY(open), (p != NULL) ? p : "");
	gtk_box_pack_start(GTK_BOX(hbox), open, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* view */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("View with:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	view = gtk_entry_new();
	p = mime_get_handler(browser->mime, type, "view");
	gtk_entry_set_text(GTK_ENTRY(view), (p != NULL) ? p : "");
	gtk_box_pack_start(GTK_BOX(hbox), view, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	/* edit */
	hbox = gtk_hbox_new(FALSE, 4);
	widget = gtk_label_new(_("Edit with:"));
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	edit = gtk_entry_new();
	p = mime_get_handler(browser->mime, type, "edit");
	gtk_entry_set_text(GTK_ENTRY(edit), (p != NULL) ? p : "");
	gtk_box_pack_start(GTK_BOX(hbox), edit, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	/* response */
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_APPLY)
	{
		mime_set_handler(browser->mime, type, "open",
				gtk_entry_get_text(GTK_ENTRY(open)));
		mime_set_handler(browser->mime, type, "view",
				gtk_entry_get_text(GTK_ENTRY(view)));
		mime_set_handler(browser->mime, type, "edit",
				gtk_entry_get_text(GTK_ENTRY(edit)));
		mime_save(browser->mime);
	}
	gtk_widget_destroy(dialog);
	free(type);
}

static void _preferences_on_mime_foreach(void * data, char const * name,
		GdkPixbuf * icon_24, GdkPixbuf * icon_48, GdkPixbuf * icon_96)
{
	Browser * browser = data;
	GtkTreeIter iter;

	gtk_list_store_append(browser->pr_mime_store, &iter);
	gtk_list_store_set(browser->pr_mime_store, &iter, BMC_NAME, name,
			BMC_ICON, icon_24, -1);
}

static void _preferences_on_plugin_toggled(GtkCellRendererToggle * renderer,
		char * path, gpointer data)
{
	Browser * browser = data;
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(
				browser->pr_plugin_store), &iter, path);
	gtk_list_store_set(browser->pr_plugin_store, &iter, BPC_ENABLED,
			!gtk_cell_renderer_toggle_get_active(renderer), -1);
}

static gboolean _preferences_on_closex(gpointer data)
{
	Browser * browser = data;

	_preferences_on_cancel(browser);
	return TRUE;
}

static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_OK)
		_preferences_on_ok(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_preferences_on_cancel(data);
}

static void _preferences_on_cancel(gpointer data)
{
	Browser * browser = data;

	gtk_widget_hide(browser->pr_window);
	_preferences_set(browser);
}

static void _preferences_on_ok(gpointer data)
{
	Browser * browser = data;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pr_plugin_store);
	GtkTreeIter iter;
	gboolean valid;
	gchar * p;
	gboolean enabled;
	String * value = string_new("");
	String * sep = "";
	int res = (value != NULL) ? 0 : 1;

	gtk_widget_hide(browser->pr_window);
	/* appearance */
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->prefs.default_view = gtk_combo_box_get_active(GTK_COMBO_BOX(
				browser->pr_view));
#endif
	browser->prefs.alternate_rows = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_alternate));
	if(browser->detailview != NULL)
		gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(browser->detailview),
				browser->prefs.alternate_rows);
	browser->prefs.confirm_before_delete = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_confirm));
	browser->prefs.sort_folders_first = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_sort));
	browser->prefs.show_hidden_files = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_hidden));
	/* plug-ins */
	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BPC_NAME, &p,
				BPC_ENABLED, &enabled, -1);
		if(enabled)
		{
			browser_load(browser, p);
			res |= string_append(&value, sep);
			res |= string_append(&value, p);
			sep = ",";
		}
		else
			browser_unload(browser, p);
		g_free(p);
	}
	if(res == 0)
		config_set(browser->config, NULL, "plugins", value);
	string_delete(value);
	browser_config_save(browser);
	browser_refresh(browser);
}


/* browser_unload */
int browser_unload(Browser * browser, char const * plugin)
{
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pl_store);
	GtkTreeIter iter;
	gboolean valid;
	gchar * p;
	Plugin * pp;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	GtkWidget * widget;
	gboolean enabled = FALSE;

	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BPC_NAME, &p, BPC_PLUGIN, &pp,
				BPC_BROWSERPLUGINDEFINITION, &bpd,
				BPC_BROWSERPLUGIN, &bp, BPC_WIDGET, &widget,
				-1);
		enabled = (strcmp(p, plugin) == 0) ? TRUE : FALSE;
		g_free(p);
		if(enabled)
			break;
	}
	if(enabled != TRUE)
		return 0;
	gtk_list_store_remove(browser->pl_store, &iter);
	gtk_container_remove(GTK_CONTAINER(browser->pl_box), widget);
	if(bpd->destroy != NULL)
		bpd->destroy(bp);
	plugin_delete(pp);
	if(gtk_tree_model_iter_n_children(model, NULL) == 0)
	{
		gtk_widget_set_no_show_all(browser->pl_view, TRUE);
		gtk_widget_hide(browser->pl_view);
	}
	else if(gtk_combo_box_get_active(GTK_COMBO_BOX(browser->pl_combo)) < 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(browser->pl_combo), 0);
	return 0;
}


/* browser_unselect_all */
void browser_unselect_all(Browser * browser)
{
	GtkTreeSelection * sel;

#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser_get_view(browser) != BV_DETAILS)
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
/* browser_opendir */
static DIR * _browser_opendir(char const * pathname, struct stat * st)
{
	DIR * dir;
	int fd;

#ifdef DEBUG
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


/* browser_plugin_refresh */
static void _plugin_refresh_do(Browser * browser, char const * path);

static void _browser_plugin_refresh(Browser * browser)
{
	char const * location;
	GtkTreeSelection * treesel;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter iter;
	GList * sel;
	gchar * path = NULL;

	location = browser_get_location(browser);
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser_get_view(browser) != BV_DETAILS)
		sel = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
					browser->iconview));
	else
#endif
	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						browser->detailview))) == NULL)
	{
		_plugin_refresh_do(browser, location);
		return;
	}
	else
		sel = gtk_tree_selection_get_selected_rows(treesel, NULL);
	if(sel != NULL && sel->data != NULL && sel->next == NULL
			&& gtk_tree_model_get_iter(model, &iter, sel->data))
	{
		gtk_tree_model_get(model, &iter, BC_PATH, &path, -1);
		_plugin_refresh_do(browser, (path != NULL) ? path : location);
		g_free(path);
	}
	else if(location != NULL)
		_plugin_refresh_do(browser, location);
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
}

static void _plugin_refresh_do(Browser * browser, char const * path)
{
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pl_store);
	GtkTreeIter iter;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	GList * l;

	if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(browser->pl_combo),
				&iter) != TRUE)
		return;
	gtk_tree_model_get(model, &iter, BPC_BROWSERPLUGINDEFINITION, &bpd,
			BPC_BROWSERPLUGIN, &bp, -1);
	if(bpd->refresh != NULL)
	{
		/* FIXME pass the complete selection instead */
		l = g_list_append(NULL, path);
		bpd->refresh(bp, l);
		g_list_free(l);
	}
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
			(char *)browser->current->data);
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
	_browser_plugin_refresh(browser);
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
	char const * title;
	char * p;
	GError * error = NULL;
	char buf[256];

	title = browser_get_location(browser);
	if((p = g_filename_to_utf8(title, -1, NULL, NULL, &error)) == NULL)
	{
		browser_error(NULL, error->message, 1);
		g_error_free(error);
	}
	else
		title = p;
	snprintf(buf, sizeof(buf), "%s%s%s", _("File manager"), " - ", title);
	free(p);
	gtk_window_set_title(GTK_WINDOW(browser->window), buf);
}

static void _refresh_path(Browser * browser)
{
	static unsigned int cnt = 0;
	char const * location;
	GtkWidget * widget;
	char * p;
	GError * error = NULL;
	unsigned int i;
	char * q;

	location = browser_get_location(browser);
	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if((p = g_filename_to_utf8(location, -1, NULL, NULL, &error)) == NULL)
	{
		browser_error(NULL, error->message, 1);
		g_error_free(error);
	}
	gtk_entry_set_text(GTK_ENTRY(widget), (p != NULL) ? p : location);
	free(p);
	for(i = 0; i < cnt; i++)
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(browser->tb_path),
				0);
#else
		gtk_combo_box_remove_text(GTK_COMBO_BOX(browser->tb_path), 0);
#endif
	if((p = g_path_get_dirname(location)) == NULL)
		return;
	if(strcmp(p, ".") != 0)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(browser->tb_path),
				NULL, p);
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(browser->tb_path), p);
#endif
		for(cnt = 1; strcmp(p, "/") != 0; cnt++)
		{
			q = g_path_get_dirname(p);
			g_free(p);
			p = q;
#if GTK_CHECK_VERSION(3, 0, 0)
			gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(
						browser->tb_path), NULL, p);
#else
			gtk_combo_box_append_text(GTK_COMBO_BOX(
						browser->tb_path), p);
#endif
		}
	}
	g_free(p);
}


/* browser_plugin_is_enabled */
static gboolean _browser_plugin_is_enabled(Browser * browser,
		char const * plugin)
{
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pl_store);
	GtkTreeIter iter;
	gchar * p;
	gboolean valid;
	int res;

	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, BPC_NAME, &p, -1);
		res = strcmp(p, plugin);
		g_free(p);
		if(res == 0)
			return TRUE;
	}
	return FALSE;
}


/* browser_get_mime */
static Mime * _browser_get_mime(Browser * browser)
{
	return browser->mime;
}


/* browser_set_status */
static void _browser_set_status(Browser * browser, char const * status)
{
	GtkStatusbar * sb = GTK_STATUSBAR(browser->statusbar);

	if(browser->statusbar_id != 0)
		gtk_statusbar_remove(sb, gtk_statusbar_get_context_id(sb, ""),
				browser->statusbar_id);
	browser->statusbar_id = gtk_statusbar_push(sb,
			gtk_statusbar_get_context_id(sb, ""), status);
}


/* config_load_boolean */
static int _config_load_boolean(Config * config, char const * variable,
		gboolean * value)
{
	char const * str;

	if((str = config_get(config, NULL, variable)) == NULL)
		return -1;
	if(strcmp(str, "0") == 0)
		*value = FALSE;
	else if(strcmp(str, "1") == 0)
		*value = TRUE;
	else
		return -1;
	return 0;
}


/* config_load_string */
static int _config_load_string(Config * config, char const * variable,
		char ** value)
{
	char const * str;
	char * p;

	if((str = config_get(config, NULL, variable)) == NULL)
		return 0;
	if((p = strdup(str)) == NULL)
		return -1;
	free(*value);
	*value = p;
	return 0;
}


/* config_save_boolean */
static int _config_save_boolean(Config * config, char const * variable,
		gboolean value)
{
	return config_set(config, NULL, variable, value ? "1" : "0");
}


/* callbacks */
/* browser_on_plugin_combo */
static void _browser_on_plugin_combo_change(gpointer data)
{
	Browser * browser = data;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->pl_store);
	GtkTreeIter iter;
	gboolean valid;
	BrowserPluginDefinition * bpd;
	BrowserPlugin * bp;
	GtkWidget * widget;

	for(valid = gtk_tree_model_get_iter_first(model, &iter); valid == TRUE;
			valid = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(browser->pl_store), &iter,
				BPC_BROWSERPLUGINDEFINITION, &bpd,
				BPC_BROWSERPLUGIN, &bp,
				BPC_WIDGET, &widget, -1);
		/* signal the previous plug-in that it is no longer active */
		if(gtk_widget_get_visible(widget) && bpd->refresh != NULL)
			bpd->refresh(bp, NULL);
		gtk_widget_hide(widget);
	}
	if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(browser->pl_combo),
				&iter) != TRUE)
		return;
	gtk_tree_model_get(GTK_TREE_MODEL(browser->pl_store), &iter, BPC_WIDGET,
			&widget, -1);
	gtk_widget_show(widget);
	_browser_plugin_refresh(browser);
}


/* browser_on_selection_changed */
static void _browser_on_selection_changed(gpointer data)
{
	Browser * browser = data;
	
	_browser_plugin_refresh(browser);
}
