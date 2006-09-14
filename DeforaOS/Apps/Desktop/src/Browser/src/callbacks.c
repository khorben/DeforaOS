/* callbacks.c */



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "callbacks.h"
#include "browser.h"
#include "../config.h"


/* constants */
static char const * _authors[] =
{
	"Pierre 'khorben' Pronchery",
	NULL
};

/* FIXME */
static char const _license[] = "GPLv2";


/* callbacks */
/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_widget_hide(widget);
	gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_new_window(GtkMenuItem * menuitem, gpointer data)
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
	fprintf(stderr, "%s%s\n", "browser: browser: ", strerror(errno));
	exit(2);
}


void on_file_close(GtkMenuItem * menuitem, gpointer data)
{
	gtk_main_quit();
}


/* edit menu */
static GList * _copy_selection(Browser * browser);
void on_edit_copy(GtkMenuItem * menuitem, gpointer data)
	/* FIXME */
{
	Browser * browser = data;
	GtkTreeIter iter;
	GList * sel;
	GList * p;
	gchar * q;

	if((sel = _copy_selection(browser)) == NULL)
		return;
	for(p = sel; p->next != NULL; p = p->next)
	{
		if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store),
					&iter, p->data))
			continue;
		gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
				BR_COL_PATH, &q, -1);
		printf("%s\n", q);
		g_free(q);
	}
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
}

static GList * _copy_selection(Browser * browser)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
		return gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
				browser->iconview));
	else
#endif
	{
		GtkTreeSelection * treesel;

		if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
							browser->detailview)))
				== NULL)
			return NULL;
		return gtk_tree_selection_get_selected_rows(treesel, NULL);
	}
}


void on_edit_cut(GtkMenuItem * menuitem, gpointer data)
	/* FIXME */
{
	Browser * browser = data;
	GtkTreeIter iter;
	GList * sel;
	GList * p;
	gchar * q;

	if((sel = _copy_selection(browser)) == NULL)
		return;
	for(p = sel; p->next != NULL; p = p->next)
	{
		if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store),
					&iter, p->data))
			continue;
		gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
				BR_COL_PATH, &q, -1);
		printf("%s\n", q);
		g_free(q);
	}
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel);
}


static void _delete_do(Browser * browser, GList * selection, unsigned long cnt);
void on_edit_delete(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;
	GtkWidget * dialog;
	unsigned long cnt = 0;
	int ret;
	GtkTreeIter iter;
	GList * selection;
	GList * p;

	if((selection = _copy_selection(browser)) == NULL)
		return;
	for(p = selection; p->next != NULL; p = p->next)
		if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store),
					&iter, p->data))
			continue;
		else
			cnt++;
	if(cnt == 0)
		return;
	dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s%lu%s",
			"Are you sure you want to delete ", cnt, " file(s)?");
	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(GTK_WIDGET(dialog));
	if(ret == GTK_RESPONSE_YES)
		_delete_do(browser, selection, cnt);
	g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(selection);
}

static void _delete_do(Browser * browser, GList * selection, unsigned long cnt)
{
	unsigned long i = 1;
	char ** argv;
	pid_t pid;
	GtkTreeIter iter;
	GList * p;
	gchar * q;

	if((pid = fork()) == -1)
	{
		browser_error(browser, "fork", 0);
		return;
	}
	else if(pid != 0)
		return;
	if((argv = malloc(sizeof(char*) * (cnt+2))) == NULL)
	{
		fprintf(stderr, "%s%s\n", "browser: malloc: ", strerror(errno));
		exit(2);
	}
	argv[0] = "delete";
	argv[cnt+1] = NULL;
	for(p = selection; p->next != NULL; p = p->next)
	{
		if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store),
					&iter, p->data))
			continue;
		gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
				BR_COL_PATH, &q, -1);
		argv[i++] = q;
	}
	execvp(argv[0], argv);
	fprintf(stderr, "%s%s%s%s\n", "browser: ", argv[0], ": ",
			strerror(errno));
	exit(2);
}


void on_edit_select_all(GtkMenuItem * menuitem, gpointer data)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	Browser * browser = data;

	gtk_icon_view_select_all(GTK_ICON_VIEW(browser->iconview));
#endif
}


void on_edit_unselect_all(GtkMenuItem * menuitem, gpointer data)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	Browser * browser = data;

	gtk_icon_view_unselect_all(GTK_ICON_VIEW(browser->iconview));
#endif
}


static void _preferences_set(Browser * browser);
/* callbacks */
static gboolean _preferences_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _preferences_on_cancel(GtkWidget * widget, gpointer data);
static void _preferences_on_ok(GtkWidget * widget, gpointer data);
void on_edit_preferences(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * widget;
	GtkSizeGroup * group;

	if(browser->pr_window != NULL)
	{
		gtk_widget_show(browser->pr_window);
		return;
	}
	browser->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(browser->pr_window), FALSE);
	gtk_window_set_title(GTK_WINDOW(browser->pr_window),
			"File browser preferences");
	gtk_window_set_transient_for(GTK_WINDOW(browser->pr_window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(browser->pr_window), "delete_event",
			G_CALLBACK(_preferences_on_closex), browser);
	vbox = gtk_vbox_new(FALSE, 0);
	browser->pr_sort = gtk_check_button_new_with_mnemonic(
			"Sort _folders first");
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_sort, FALSE, FALSE, 4);
	browser->pr_hidden = gtk_check_button_new_with_mnemonic(
			"Show _hidden files");
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_hidden, FALSE, FALSE, 4);
	/* dialog */
	hbox = gtk_hbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_ok), browser);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_preferences_on_cancel), browser);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(browser->pr_window), vbox);
	_preferences_set(browser);
	gtk_widget_show_all(browser->pr_window);
}

static void _preferences_set(Browser * browser)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_sort),
			browser->prefs.sort_folders_first);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_hidden),
			browser->prefs.show_hidden_files);
}

static gboolean _preferences_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	Browser * browser = data;

	_preferences_on_cancel(widget, browser);
	return TRUE;
}

static void _preferences_on_cancel(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	gtk_widget_hide(browser->pr_window);
	_preferences_set(browser);
}

static void _preferences_on_ok(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	gtk_widget_hide(browser->pr_window);
	browser->prefs.sort_folders_first = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_sort));
	browser->prefs.show_hidden_files = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_hidden));
	browser_refresh(browser);
}


/* view menu */
void on_view_home(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_location(browser, g_get_home_dir());
}


#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_details(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_DETAILS);
}


void on_view_icons(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_ICONS);
}


void on_view_list(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_LIST);
}
#endif /* GTK_CHECK_VERSION(2, 6, 0) */


/* help menu */
static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data);
static void _about_on_credits(GtkWidget * widget, gpointer data);
static void _about_on_license(GtkWidget * widget, gpointer data);
#endif
void on_help_about(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	static GtkWidget * window = NULL;
	char const copyright[] = "Copyright (c) 2006 khorben";
#if GTK_CHECK_VERSION(2, 6, 0)
	gsize cnt = 65536;
	gchar * buf;
	
	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	if((buf = malloc(sizeof(*buf) * cnt)) == NULL)
	{
		browser_error(browser, "malloc", 0);
		return;
	}
	window = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), window);
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	if(g_file_get_contents("/usr/share/common-licenses/GPL-2", &buf, &cnt,
				NULL) == TRUE)
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), buf);
	else
		gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window),
				_license);
	free(buf);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Browser");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(copyright), FALSE,
			FALSE, 2);
	hbox = gtk_hbox_new(TRUE, 4);
	button = gtk_button_new_with_mnemonic("C_redits");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_credits), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_with_mnemonic("_License");
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_license), window);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(
				_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */

static gboolean _about_on_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	return TRUE;
}

#if !GTK_CHECK_VERSION(2, 6, 0)
static void _about_on_close(GtkWidget * widget, gpointer data)
{
	GtkWidget * window = data;

	gtk_widget_hide(window);
}

static void _about_on_credits(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * notebook;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkTextIter iter;
	GtkWidget * hbox;
	size_t i;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "Credits");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, "", 0);
	for(i = 0; _authors[i] != NULL; i++)
	{
		gtk_text_buffer_get_end_iter(tbuf, &iter);
		gtk_text_buffer_insert(tbuf, &iter, _authors[i], strlen(
					_authors[i]));
	}
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	notebook = gtk_notebook_new();
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget,
			gtk_label_new("Written by"));
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}

static void _about_on_license(GtkWidget * widget, gpointer data)
{
	static GtkWidget * window = NULL;
	GtkWidget * about = data;
	GtkWidget * vbox;
	GtkWidget * textview;
	GtkTextBuffer * tbuf;
	GtkWidget * hbox;

	if(window != NULL)
	{
		gtk_widget_show(window);
		return;
	}
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "License");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(about));
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(widget), textview);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 4);
	hbox = gtk_hbox_new(FALSE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(widget), "clicked",
			G_CALLBACK(_about_on_close), window);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show_all(window);
}
#endif /* !GTK_CHECK_VERSION(2, 6, 0) */


/* toolbar */
void on_back(GtkWidget * widget, gpointer data)
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
	browser_refresh(browser);
}


void on_forward(GtkWidget * widget, gpointer data)
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
	browser_refresh(browser);
}


void on_home(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_location(browser, g_get_home_dir());
}


void on_properties(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	GList * selection;

	if((selection = _copy_selection(browser)) == NULL)
		return;
	/* FIXME */
	g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(selection);
}


void on_refresh(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_refresh(browser);
}


void on_updir(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	char * dir;

	browser = data;
	dir = g_path_get_dirname(browser->current->data);
	browser_set_location(browser, dir);
	g_free(dir);
}


#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_as(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	if(browser->iconview == NULL)
		browser_set_view(browser, BV_ICONS);
	else if(gtk_icon_view_get_orientation(GTK_ICON_VIEW(browser->iconview))
			== GTK_ORIENTATION_VERTICAL)
		browser_set_view(browser, BV_LIST);
	else
		browser_set_view(browser, BV_DETAILS);
}
#endif


/* view */
void on_detail_default(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	Browser * browser = data;
	char * location;
	GtkTreeIter iter;
	gboolean is_dir;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter, BR_COL_PATH,
			&location, BR_COL_IS_DIRECTORY, &is_dir, -1);
	browser_set_location(browser, location);
	g_free(location);
}


#if GTK_CHECK_VERSION(2, 6, 0)
void on_icon_default(GtkIconView * view,
		GtkTreePath * tree_path, gpointer data)
{
	Browser * browser = data;
	char * path;
	GtkTreeIter iter;
	gboolean is_dir;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter,
			tree_path);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter, BR_COL_PATH,
			&path, BR_COL_IS_DIRECTORY, &is_dir, -1);
	browser_set_location(browser, path);
	g_free(path);
}
#endif


/* address bar */
void on_path_activate(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_location(browser, gtk_entry_get_text(GTK_ENTRY(
					browser->tb_path)));
}


/* view */
/* types */
/* FIXME rather ugly, maybe could go directly in Browser */
typedef struct _IconCallback
{
	Browser * browser;
	int isdir;
	char * path;
} IconCallback;

/* variables */
static IconCallback _icon_cb_data;

static gboolean _popup_show(Browser * browser, GdkEventButton * event,
		GtkWidget * menu);
static void on_icon_open(GtkWidget * widget, gpointer data);
static void on_icon_edit(GtkWidget * widget, gpointer data);
gboolean on_view_popup(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	Browser * browser = data;
	GtkWidget * menu;
	GtkTreePath * path = NULL;
	GtkTreeIter iter;
	GtkWidget * menuitem;

	if(event->type != GDK_BUTTON_PRESS || event->button != 3)
		return FALSE;
	menu = gtk_menu_new();
	/* FIXME prevents actions to be called but probably leaks memory
	g_signal_connect(G_OBJECT(menu), "deactivate", G_CALLBACK(
				gtk_widget_destroy), NULL); */
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
		path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(
					browser->iconview), (int)event->x,
				(int)event->y);
	else
#endif
		gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(
					browser->detailview), (int)event->x,
				(int)event->y, &path, NULL, NULL, NULL);
	if(path == NULL)
	{
		menuitem = gtk_image_menu_item_new_from_stock(
				GTK_STOCK_PROPERTIES, NULL);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		return _popup_show(browser, event, menu);
	}
	/* FIXME error checking + sub-functions */
	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
			BR_COL_IS_DIRECTORY, &_icon_cb_data.isdir, -1);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
			BR_COL_PATH, &_icon_cb_data.path, -1);
	_icon_cb_data.browser = browser;
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_icon_open), &_icon_cb_data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	if(_icon_cb_data.isdir != TRUE)
	{
#if GTK_CHECK_VERSION(2, 6, 0)
		menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_EDIT,
				NULL);
#else
		menuitem = gtk_menu_item_new_with_mnemonic("_Edit");
#endif
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					on_icon_edit), &_icon_cb_data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
#if !GTK_CHECK_VERSION(2, 6, 0)
	gtk_tree_path_free(path);
#endif
	return _popup_show(browser, event, menu);
}

static gboolean _popup_show(Browser * browser, GdkEventButton * event,
		GtkWidget * menu)
{
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
		gtk_menu_attach_to_widget(GTK_MENU(menu), browser->iconview,
				NULL);
	else
#endif
		gtk_menu_attach_to_widget(GTK_MENU(menu), browser->detailview,
				NULL);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, event->time);
	return TRUE;
}

static void on_icon_edit(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	mime_action(cb->browser->mime, "edit", cb->path);
}

static void on_icon_open(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	if(cb->isdir)
		browser_set_location(cb->browser, cb->path);
	else
		mime_action(cb->browser->mime, "open", cb->path);
}
