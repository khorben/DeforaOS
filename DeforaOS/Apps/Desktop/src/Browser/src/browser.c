/* browser.c */
/* TODO:
 * - hitting refresh unaligns the icons (factorize initialization code with
 *   _fill_store()) */



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "browser.h"

#define min(a, b) ((a) > (b) ? (b) : (a))


/* Browser */
static int _new_pixbufs(Browser * browser);
static GtkWidget * _new_menubar(Browser * browser);
static GtkListStore * _create_store(void);
static void _fill_store(Browser * browser);
/* callbacks */
static void _browser_on_back(GtkWidget * widget, gpointer data);
static void _browser_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _browser_on_edit_copy(GtkWidget * widget, gpointer data);
static void _browser_on_edit_cut(GtkWidget * widget, gpointer data);
static void _browser_on_edit_paste(GtkWidget * widget, gpointer data);
static void _browser_on_edit_preferences(GtkWidget * widget, gpointer data);
static void _browser_on_edit_select_all(GtkWidget * widget, gpointer data);
static void _browser_on_edit_unselect_all(GtkWidget * widget, gpointer data);
static void _browser_on_file_new_window(GtkWidget * widget, gpointer data);
static void _browser_on_file_close(GtkWidget * widget, gpointer data);
static void _browser_on_forward(GtkWidget * widget, gpointer data);
static void _browser_on_help_about(GtkWidget * widget, gpointer data);
static void _browser_on_home(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
static void _browser_on_icon_default(GtkIconView * iconview,
		GtkTreePath *tree_path, gpointer data);
#else
static void _browser_on_icon_default(GtkTreeView * view,
		GtkTreePath * tree_path, GtkTreeViewColumn * column,
		gpointer data);
#endif
static void _browser_on_path_activate(GtkWidget * widget, gpointer data);
static void _browser_on_refresh(GtkWidget * widget, gpointer data);
static void _browser_on_updir(GtkWidget * widget, gpointer data);
struct _menu
{
	char * name;
	GtkSignalFunc callback;
	char * stock;
};
struct _menubar
{
	char * name;
	struct _menu * menu;
};
struct _menu _menu_file[] =
{
	{ "_New window", G_CALLBACK(_browser_on_file_new_window),
		GTK_STOCK_NEW }, /* FIXME */
	{ "", NULL, NULL },
	{ "_Close", G_CALLBACK(_browser_on_file_close), GTK_STOCK_CLOSE },
	{ NULL, NULL, NULL }
};
static struct _menu _menu_edit[] =
{
	{ "_Cut", G_CALLBACK(_browser_on_edit_cut), GTK_STOCK_CUT },
	{ "Cop_y", G_CALLBACK(_browser_on_edit_copy), GTK_STOCK_COPY },
	{ "_Paste", G_CALLBACK(_browser_on_edit_paste), GTK_STOCK_PASTE },
	{ "", NULL, NULL },
	{ "_Select all", G_CALLBACK(_browser_on_edit_select_all), NULL },
	{ "_Unselect all", G_CALLBACK(_browser_on_edit_unselect_all), NULL },
	{ "", NULL, NULL },
	{ "_Preferences", G_CALLBACK(_browser_on_edit_preferences),
		GTK_STOCK_PREFERENCES },
	{ NULL, NULL, NULL }
};
static struct _menu _menu_help[] =
{
#if GTK_CHECK_VERSION(2, 6, 0)
	{ "_About", G_CALLBACK(_browser_on_help_about), GTK_STOCK_ABOUT },
#else
	{ "_About", G_CALLBACK(_browser_on_help_about), NULL },
#endif
	{ NULL, NULL, NULL }
};
static struct _menubar _menubar[] =
{
	{ "_File", _menu_file },
	{ "_Edit", _menu_edit },
	{ "_Help", _menu_help },
	{ NULL, NULL }
};
Browser * browser_new(char const * directory)
{
	Browser * browser;
	GtkWidget * sw;
	GtkWidget * vbox;
	GtkWidget * tb_menubar;
	GtkWidget * toolbar;
	GtkWidget * widget;
	GtkToolItem * toolitem;
	GtkToolItem * tb_button;

	if((browser = malloc(sizeof(*browser))) == NULL)
		return NULL;
	if(!_new_pixbufs(browser))
	{
		browser_error(browser, "Error while loading default icons", 0);
		free(browser);
		return NULL;
	}

	/* config */
	/* FIXME */

	/* mime */
	browser->mime = mime_new();

	/* history */
	browser->history = g_list_append(NULL, strdup(directory == NULL
				? g_get_home_dir() : directory));
	browser->current = browser->history;

	browser->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(browser->window), 640, 480);
	/* FIXME */
	gtk_window_set_title(GTK_WINDOW(browser->window), "File browser");
	/* g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroyed),
		&window); */
	g_signal_connect(browser->window, "delete_event",
			G_CALLBACK(_browser_on_closex), NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	/* menubar */
	tb_menubar = _new_menubar(browser);
	gtk_box_pack_start(GTK_BOX(vbox), tb_menubar, FALSE, FALSE, 0);
	/* toolbar */
	toolbar = gtk_toolbar_new();
	browser->tb_back = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	g_signal_connect(browser->tb_back, "clicked",
			G_CALLBACK(_browser_on_back), browser);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_back, -1);

	browser->tb_updir = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
			strcmp(browser->current->data, "/") != 0);
	g_signal_connect(browser->tb_updir, "clicked",
			G_CALLBACK(_browser_on_updir), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_updir, -1);
	browser->tb_forward = gtk_tool_button_new_from_stock(
			GTK_STOCK_GO_FORWARD);
	g_signal_connect(browser->tb_forward, "clicked",
			G_CALLBACK(_browser_on_forward), browser);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), browser->tb_forward, -1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect(tb_button, "clicked", G_CALLBACK(_browser_on_refresh),
			browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(),
			-1);
	tb_button = gtk_tool_button_new_from_stock(GTK_STOCK_HOME);
	g_signal_connect(tb_button, "clicked", G_CALLBACK(_browser_on_home),
			browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tb_button, -1);
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
	browser->tb_path = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(browser->tb_path), browser->current->data);
	g_signal_connect(G_OBJECT(browser->tb_path), "activate",
			G_CALLBACK(_browser_on_path_activate), browser);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_container_add(GTK_CONTAINER(toolitem), browser->tb_path);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_JUMP_TO);
	g_signal_connect(G_OBJECT(toolitem), "clicked",
			G_CALLBACK(_browser_on_path_activate), browser);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* icon view */
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
	/* statusbar */
	browser->statusbar = gtk_statusbar_new();
	browser->statusbar_id = 0;
	gtk_box_pack_start(GTK_BOX(vbox), browser->statusbar, FALSE, FALSE, 0);
	/* store */
	browser->store = _create_store();
	_fill_store(browser);
#if GTK_CHECK_VERSION(2, 6, 0)
	browser->iconview = gtk_icon_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(browser->iconview),
			GTK_SELECTION_MULTIPLE);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_DISPLAY_NAME);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(browser->iconview),
			BR_COL_PIXBUF);
	g_signal_connect(browser->iconview, "item-activated",
			G_CALLBACK(_browser_on_icon_default), browser);
#else
	browser->iconview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				browser->store));
	gtk_tree_view_append_column(GTK_TREE_VIEW(browser->iconview),
			gtk_tree_view_column_new_with_attributes("Filename",
				gtk_cell_renderer_text_new(), "text", NULL,
				NULL));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(browser->iconview),
			FALSE);
	g_signal_connect(G_OBJECT(browser->iconview), "row-activated",
			G_CALLBACK(_browser_on_icon_default), browser);
#endif
	g_object_unref(browser->store);
	gtk_container_add(GTK_CONTAINER(sw), browser->iconview);
	gtk_widget_grab_focus(browser->iconview);

	gtk_container_add(GTK_CONTAINER(browser->window), vbox);
	gtk_widget_show_all(browser->window);
	return browser;
}

static int _new_pixbufs(Browser * browser)
{
	browser->theme = gtk_icon_theme_new();
	gtk_icon_theme_set_custom_theme(browser->theme, "gnome");
	if((browser->pb_file = gtk_icon_theme_load_icon(browser->theme,
			"gnome-fs-regular", 48, 0, NULL)) == NULL)
		return FALSE;
	browser->pb_folder = gtk_icon_theme_load_icon(browser->theme,
			"gnome-fs-directory", 48, 0, NULL);
	return browser->pb_folder != NULL;
}

static GtkWidget * _new_menubar(Browser * browser)
{
	GtkWidget * tb_menubar;
	GtkWidget * menu;
	GtkWidget * menubar;
	GtkWidget * menuitem;
	unsigned int i;
	unsigned int j;

	tb_menubar = gtk_menu_bar_new();
	for(i = 0; _menubar[i].name != NULL; i++)
	{
		menubar = gtk_menu_item_new_with_mnemonic(_menubar[i].name);
		menu = gtk_menu_new();
		for(j = 0; _menubar[i].menu[j].name != NULL; j++)
		{
			if(_menubar[i].menu[j].name[0] == '\0')
				menuitem = gtk_separator_menu_item_new();
			else if(_menubar[i].menu[j].stock == 0)
				menuitem = gtk_menu_item_new_with_mnemonic(
						_menubar[i].menu[j].name);
			else
				menuitem = gtk_image_menu_item_new_from_stock(
						_menubar[i].menu[j].stock,
						NULL);
			if(_menubar[i].menu[j].callback != NULL)
				g_signal_connect(G_OBJECT(menuitem), "activate",
						G_CALLBACK(_menubar[i].menu[j].callback), browser);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		}
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar), menu);
		gtk_menu_bar_append(GTK_MENU_BAR(tb_menubar), menubar);
	}
	return tb_menubar;
}

static void _fill_store(Browser * browser)
{
	GDir * dir;
	char const * name;
	GtkTreeIter iter;
	unsigned int cnt;
	unsigned int hidden_cnt;
	char status[36];

	gtk_list_store_clear(browser->store);
	if((dir = g_dir_open(browser->current->data, 0, NULL)) == NULL)
		return;
	gtk_entry_set_text(GTK_ENTRY(browser->tb_path), browser->current->data);
	for(cnt = 0, hidden_cnt = 0; (name = g_dir_read_name(dir)) != NULL;)
	{
		gchar * path, * display_name;
		char const * type;
		gboolean is_dir;
		GdkPixbuf * icon;

		if(name[0] == '.') /* FIXME optional */
		{
			hidden_cnt++;
			continue;
		}
		path = g_build_filename(browser->current->data, name, NULL);
		is_dir = g_file_test(path, G_FILE_TEST_IS_DIR);
		display_name = g_filename_to_utf8(name, -1, NULL, NULL, NULL);
		gtk_list_store_append(browser->store, &iter);
		if(is_dir)
			icon = browser->pb_folder;
		else if(!is_dir && browser->mime != NULL
				&& (type = mime_type(browser->mime, name))
				!= NULL)
		{
			if((icon = mime_icon(browser->mime, browser->theme,
							type)) == NULL)
				icon = browser->pb_file;
		}
		else
			icon = browser->pb_file;
		gtk_list_store_set(browser->store, &iter, BR_COL_PATH, path,
				BR_COL_DISPLAY_NAME, display_name,
				BR_COL_IS_DIRECTORY, is_dir, BR_COL_PIXBUF,
				icon, -1);
		g_free(path);
		g_free(display_name);
		cnt++;
	}
	if(browser->statusbar_id)
		gtk_statusbar_remove(GTK_STATUSBAR(browser->statusbar),
				gtk_statusbar_get_context_id(
					GTK_STATUSBAR(browser->statusbar), ""),
				browser->statusbar_id);
	snprintf(status, sizeof(status), "%u file%c (%u hidden)", cnt, cnt <= 1
			? '\0' : 's', hidden_cnt);
	browser->statusbar_id = gtk_statusbar_push(GTK_STATUSBAR(
				browser->statusbar),
			gtk_statusbar_get_context_id(GTK_STATUSBAR(
					browser->statusbar), ""), status);
}

static int _sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
		gpointer data)
{
	gboolean is_dir_a, is_dir_b;
	gchar * name_a, * name_b;
	int ret;

	/* FIXME sorts folders before files => optional */
	gtk_tree_model_get(model, a, BR_COL_IS_DIRECTORY, &is_dir_a,
			BR_COL_DISPLAY_NAME, &name_a, -1);
	gtk_tree_model_get(model, b, BR_COL_IS_DIRECTORY, &is_dir_b,
			BR_COL_DISPLAY_NAME, &name_b, -1);
	if(!is_dir_a && is_dir_b)
		ret = 1;
	else if(is_dir_a && !is_dir_b)
		ret = -1;
	else
		ret = g_utf8_collate(name_a, name_b);
	g_free(name_a);
	g_free(name_b);
	return ret;
}

static GtkListStore * _create_store(void)
{
	GtkListStore * store;

	store = gtk_list_store_new(BR_NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, 
			GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store),
			_sort_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store),
			GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
			GTK_SORT_ASCENDING); /* FIXME resettable */
	return store;
}

/* callbacks */
static void _browser_on_back(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	if(browser->current->prev == NULL)
		return;
	browser->current = g_list_previous(browser->current);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back),
			browser->current->prev != NULL);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
			strcmp(browser->current->data, "/") != 0);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward),
			TRUE);
	_fill_store(browser);
}

static void _browser_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
}

static void _copy_foreach(gpointer data, Browser * browser);
static void _browser_on_edit_copy(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	GList * selection;

	/* FIXME free previous buffer */
	selection = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
				browser->iconview));
	g_list_foreach(selection, (GFunc)_copy_foreach, browser);
	/* FIXME differentiate cut from paste */
}

static void _copy_foreach(gpointer data, Browser * browser)
{
	printf("%p\n", data); /* FIXME */
}

static void _browser_on_edit_cut(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	GList * selection;

	/* FIXME free previous buffer */
	selection = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
				browser->iconview));
	/* FIXME differentiate cut from paste */
}

static void _browser_on_edit_paste(GtkWidget * widget, gpointer data)
{
}

static void _browser_on_edit_select_all(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	gtk_icon_view_select_all(GTK_ICON_VIEW(browser->iconview));
}

static void _browser_on_edit_unselect_all(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	gtk_icon_view_unselect_all(GTK_ICON_VIEW(browser->iconview));
}

static void _browser_on_preferences_close(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _browser_on_edit_preferences(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	Browser * browser = data;
	GtkWidget * vbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "File browser preferences");
	g_signal_connect(G_OBJECT(window), "delete_event",
			G_CALLBACK(_browser_on_preferences_close), browser);
	vbox = gtk_vbox_new(FALSE, 0);
	/* FIXME */
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _browser_on_preferences_close(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
}

static void _browser_on_file_new_window(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	pid_t pid;

	if((pid = fork()) == -1)
	{
		browser_error(browser, strerror(errno), 0);
		return;
	}
	if(pid != 0)
		return;
	execlp("browser", "browser", browser->current->data, NULL);
	fprintf(stderr, "%s%s", "browser: browser: ", strerror(errno));
	exit(2);
}

static void _browser_on_file_close(GtkWidget * widget, gpointer data)
{
	gtk_main_quit();
}

static void _browser_on_forward(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	if(browser->current->next == NULL)
		return;
	browser->current = browser->current->next;
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
			strcmp(browser->current->data, "/") != 0);
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward),
			browser->current->next != NULL);
	_fill_store(browser);
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_close(GtkWidget * widget, gpointer * data);
static void _about_credits(GtkWidget * widget, gpointer * data);
static void _about_license(GtkWidget * widget, gpointer * data);
#endif
static void _browser_on_help_about(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	char const * authors[] = { "Pierre 'khorben' Pronchery", NULL };
	char const copyright[] = "Copyright (c) 2006 khorben";

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
#if GTK_CHECK_VERSION(2, 6, 0)
	window = gtk_about_dialog_new();
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), "File browser");
	/* FIXME automatic version */
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), "0.0.0");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), authors);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), "GPLv2");
	gtk_widget_show(window);
#else
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Browser");
	{
		GtkWidget * vbox;
		GtkWidget * hbox;
		GtkWidget * button;

		vbox = gtk_vbox_new(FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(
					"Browser 0.0.0"), FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(copyright),
				FALSE, FALSE, 2);
		hbox = gtk_hbox_new(TRUE, 4);
		button = gtk_button_new_with_mnemonic("C_redits");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_credits), window);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		button = gtk_button_new_with_mnemonic("_License");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_license), window);
		gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
					_about_close), window);
		gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
		gtk_container_add(GTK_CONTAINER(window), vbox);
	}
	gtk_widget_show_all(window);
#endif
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_close(GtkWidget * widget, gpointer * data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
}

static void _about_credits(GtkWidget * widget, gpointer * data)
{
}

static void _about_license(GtkWidget * widget, gpointer * data)
{
}
#endif


static void _browser_go(Browser * browser, char const * path);
static void _browser_on_home(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	_browser_go(browser, g_get_home_dir());
}

static void _browser_go(Browser * browser, char const * path)
{
	if(g_file_test(path, G_FILE_TEST_IS_REGULAR))
		return mime_open(browser->mime, path);
	if(!g_file_test(path, G_FILE_TEST_IS_DIR))
		return;
	if(browser->history == NULL)
	{
		if((browser->history = g_list_alloc()) == NULL)
			return;
		browser->history->data = strdup(path);
		browser->current = browser->history;
	}
	else if(strcmp(browser->current->data, path) != 0)
	{
		g_list_foreach(browser->current->next, (GFunc)free, NULL);
		g_list_free(browser->current->next);
		browser->current->next = NULL;
		browser->history = g_list_append(browser->history,
				strdup(path));
		browser->current = g_list_last(browser->history);
		gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_back),
				TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_forward),
				FALSE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(browser->tb_updir),
			strcmp(browser->current->data, "/") != 0);
	_fill_store(browser);
}

#if GTK_CHECK_VERSION(2, 6, 0)
static void _browser_on_icon_default(GtkIconView * iconview,
		GtkTreePath * tree_path, gpointer data)
#else
static void _browser_on_icon_default(GtkTreeView * view,
		GtkTreePath * tree_path, GtkTreeViewColumn * column,
		gpointer data)
#endif
{
	Browser * browser = data;
	char * path;
	GtkTreeIter iter;
	gboolean is_dir;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter,
			tree_path);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter, BR_COL_PATH,
			&path, BR_COL_IS_DIRECTORY, &is_dir, -1);
	_browser_go(browser, path);
	g_free(path);
}

static void _browser_on_path_activate(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	_browser_go(browser, gtk_entry_get_text(GTK_ENTRY(browser->tb_path)));
}

static void _browser_on_refresh(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	_fill_store(browser);
}

static void _browser_on_updir(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	char * dir;

	browser = data;
	dir = g_path_get_dirname(browser->current->data);
	_browser_go(browser, dir);
	g_free(dir);
}


void browser_delete(Browser * browser)
{
	g_list_foreach(browser->history, (GFunc)free, NULL);
	g_list_free(browser->history);
	free(browser);
}


/* useful */
int browser_error(Browser * browser, char const * message, int ret)
{
	GtkWidget * dialog;

	dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", message);
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy),
			NULL);
	gtk_widget_show(dialog);
	return ret;
}
