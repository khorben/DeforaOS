/* $Id$ */
static char const _copyright[] =
"Copyright (c) 2008 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Browser */
static char const _license[] =
"Browser is not free software; you can redistribute it and/or modify it\n"
"under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike\n"
"3.0 Unported as published by the Creative Commons organization.\n"
"\n"
"Browser is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS\n"
"FOR A PARTICULAR PURPOSE. See the Creative Commons Attribution-\n"
"NonCommercial-ShareAlike 3.0 Unported license for more details.\n"
"\n"
"You should have received a copy of the Creative Commons Attribution-\n"
"NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to\n"
"http://creativecommons.org/licenses/by-nc-sa/3.0/";



#include <sys/param.h>
#include <sys/mount.h>
#ifdef __linux__ /* XXX linux portability */
# define unmount(a, b) umount(a)
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "callbacks.h"
#include "browser.h"
#include "../config.h"

#define COMMON_DND
#define COMMON_EXEC
#include "common.c"


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* callbacks */
/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	Browser * browser = data;

	browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
	return FALSE;
}


/* file menu */
void on_file_new_window(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	browser_new(browser->current->data);
}


void on_file_new_folder(GtkMenuItem * menuitem, gpointer data)
{
	static char const * newfolder = "New folder";
	Browser * browser = data;
	char const * cur = browser->current->data;
	char * path;

	if((path = malloc(strlen(cur) + 2 + strlen(newfolder))) == NULL)
	{
		browser_error(browser, strerror(errno), 0);
		return;
	}
	sprintf(path, "%s/%s", cur, newfolder);
	if(mkdir(path, 0777) != 0)
		browser_error(browser, strerror(errno), 0);
	free(path);
}


void on_file_close(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
}


/* edit menu */
/* on_edit_copy */
static GList * _copy_selection(Browser * browser);

void on_edit_copy(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	browser->selection = _copy_selection(browser);
	browser->selection_cut = 0;
}

static GList * _copy_selection(Browser * browser)
{
	GtkTreeSelection * treesel;
	GList * sel;
	GList * p;
	GtkTreeIter iter;
	char * q;

#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
		sel = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(
					browser->iconview));
	else
#endif
	if((treesel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
						browser->detailview))) == NULL)
		return NULL;
	else
		sel = gtk_tree_selection_get_selected_rows(treesel, NULL);
	for(p = NULL; sel != NULL; sel = sel->next)
	{
		if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store),
					&iter, sel->data))
			continue;
		gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
				BR_COL_PATH, &q, -1);
		p = g_list_append(p, q);
	}
	g_list_foreach(sel, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(sel); /* XXX can probably be optimized for re-use */
	return p;
}


/* on_edit_cut */
void on_edit_cut(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	browser->selection = _copy_selection(browser);
	browser->selection_cut = 1;
}


/* on_edit_delete */
void on_edit_delete(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;
	GtkWidget * dialog;
	unsigned long cnt = 0;
	int ret = GTK_RESPONSE_YES;
	GList * selection;
	GList * p;

	if((selection = _copy_selection(browser)) == NULL)
		return;
	for(p = selection; p != NULL; p = p->next)
		if(p->data != NULL)
			cnt++;
	if(cnt == 0)
		return;
	if(browser->prefs.confirm_before_delete == TRUE)
	{
		dialog = gtk_message_dialog_new(GTK_WINDOW(browser->window),
				GTK_DIALOG_MODAL
				| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
				"%s%lu%s", "Are you sure you want to delete ",
				cnt, " file(s)?");
		gtk_window_set_title(GTK_WINDOW(dialog), "Question");
		ret = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}
	if(ret == GTK_RESPONSE_YES
			&& _common_exec("delete", "-ir", selection) != 0)
		browser_error(browser, "fork", 0);
	g_list_foreach(selection, (GFunc)free, NULL);
	g_list_free(selection);
}


/* on_edit_paste */
void on_edit_paste(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;
	char * p = browser->current->data;

	if(browser->selection == NULL)
		return;
	browser->selection = g_list_append(browser->selection, p);
	if(browser->selection_cut != 1)
	{
		if(_common_exec("copy", "-ir", browser->selection) != 0)
			browser_error(browser, "fork", 0);
		browser->selection = g_list_remove(browser->selection, p);
		return;
	}
	if(_common_exec("move", "-i", browser->selection) != 0)
		browser_error(browser, "fork", 0);
	browser->selection = g_list_remove(browser->selection, p);
	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	browser->selection = NULL;
}


/* on_edit_select_all */
void on_edit_select_all(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	browser_select_all(browser);
}


void on_edit_unselect_all(GtkMenuItem * menuitem, gpointer data)
{
	Browser * browser = data;

	browser_unselect_all(browser);
}


/* on_edit_preferences */
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
	g_signal_connect(G_OBJECT(browser->pr_window), "delete-event",
			G_CALLBACK(_preferences_on_closex), browser);
	vbox = gtk_vbox_new(FALSE, 0);
	browser->pr_confirm = gtk_check_button_new_with_mnemonic(
			"_Confirm before delete");
	gtk_box_pack_start(GTK_BOX(vbox), browser->pr_confirm, FALSE, FALSE, 4);
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
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(browser->pr_confirm),
			browser->prefs.confirm_before_delete);
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
	browser->prefs.confirm_before_delete = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_confirm));
	browser->prefs.sort_folders_first = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_sort));
	browser->prefs.show_hidden_files = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(browser->pr_hidden));
	browser_config_save(browser);
	browser_refresh(browser);
}


/* view menu */
void on_view_home(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_location(browser, g_get_home_dir());
}


#if GTK_CHECK_VERSION(2, 6, 0)
/* on_view_details */
void on_view_details(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_DETAILS);
}


/* on_view_icons */
void on_view_icons(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_ICONS);
}


/* on_view_list */
void on_view_list(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_LIST);
}


/* on_view_thumbnails */
void on_view_thumbnails(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_THUMBNAILS);
}
#endif /* GTK_CHECK_VERSION(2, 6, 0) */


/* help menu */
/* on_help_about */
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
	GtkWidget * window;
#if GTK_CHECK_VERSION(2, 6, 0)

	if(browser->ab_window != NULL)
	{
		gtk_widget_show(browser->ab_window);
		return;
	}
	browser->ab_window = gtk_about_dialog_new();
	window = browser->ab_window;
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(window), PACKAGE);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(window), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(window), _copyright);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(window), _authors);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(window), _license);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	gtk_widget_show(window);
}
#else /* !GTK_CHECK_VERSION(2, 6, 0) */
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * button;

	if(browser->ab_window != NULL)
	{
		gtk_widget_show(browser->ab_window);
		return;
	}
	browser->ab_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = browser->ab_window;
	gtk_container_set_border_width(GTK_CONTAINER(window), 4);
	gtk_window_set_title(GTK_WINDOW(window), "About Browser");
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(
				browser->window));
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), window);
	vbox = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(PACKAGE " " VERSION),
			FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(_copyright), FALSE,
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
	static GtkWidget * window = NULL; /* XXX probably no longer adapted */
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
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
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
			GTK_SHADOW_IN);
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
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	vbox = gtk_vbox_new(FALSE, 0);
	textview = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
	tbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_set_text(tbuf, _license, strlen(_license));
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(widget),
			GTK_SHADOW_IN);
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
	browser_refresh(browser); /* FIXME if it fails history is wrong */
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
		selection = g_list_append(NULL, strdup(browser->current->data));
	if(_common_exec("properties", NULL, selection) != 0)
		browser_error(browser, "fork", 0);
	g_list_foreach(selection, (GFunc)free, NULL);
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
			== GTK_ORIENTATION_HORIZONTAL)
		browser_set_view(browser, BV_THUMBNAILS);
	else if(gtk_icon_view_get_item_width(GTK_ICON_VIEW(browser->iconview))
			!= 112)
		browser_set_view(browser, BV_LIST);
	else
		browser_set_view(browser, BV_DETAILS);
}
#endif


/* address bar */
void on_path_activate(GtkWidget * widget, gpointer data)
{
	Browser * browser = data;
	gchar const * p;

	widget = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	p = gtk_entry_get_text(GTK_ENTRY(widget));
	browser_set_location(browser, p);
}


/* view */
static void _default_do(Browser * browser, GtkTreePath * path);
void on_detail_default(GtkTreeView * view, GtkTreePath * path,
		GtkTreeViewColumn * column, gpointer data)
{
	Browser * browser = data;

	_default_do(browser, path);
}

static void _default_do(Browser * browser, GtkTreePath * path)
{
	char * location;
	GtkTreeIter iter;
	gboolean is_dir;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter, BR_COL_PATH,
			&location, BR_COL_IS_DIRECTORY, &is_dir, -1);
	if(is_dir)
		browser_set_location(browser, location);
	else if(browser->mime == NULL
			|| mime_action(browser->mime, "open", location) != 0)
		browser_open_with(browser, location);
	g_free(location);
}


#if GTK_CHECK_VERSION(2, 6, 0)
void on_icon_default(GtkIconView * view,
		GtkTreePath * path, gpointer data)
{
	Browser * browser = data;

	_default_do(browser, path);
}
#endif


void on_filename_edited(GtkCellRendererText * renderer, gchar * arg1,
		gchar * arg2, gpointer data)
{
	Browser * browser = data;
	GtkTreeModel * model = GTK_TREE_MODEL(browser->store);
	GtkTreeIter iter;
	int isdir = 0;
	char * path = NULL;
	ssize_t len;
	char * p = NULL;
	char * q;
	struct stat st;

	if(gtk_tree_model_get_iter_from_string(model, &iter, arg1) == TRUE)
	{
		gtk_tree_model_get(model, &iter, BR_COL_IS_DIRECTORY, &isdir,
				BR_COL_PATH, &path, -1);
		if(path != NULL && (len = strrchr(path, '/') - path) > 0
				&& strcmp(&path[len + 1], arg2) != 0)
			p = malloc(len + strlen(arg2) + 2);
	}
	if(p == NULL)
	{
		free(path);
		return;
	}
	strncpy(p, path, len);
	sprintf(&p[len], "/%s", arg2);
	if(isdir && lstat(p, &st) == -1 && errno == ENOENT) /* XXX TOCTOU */
	{
		q = g_filename_from_utf8(p, -1, NULL, NULL, NULL);
		if(rename(path, q != NULL ? q : p) != 0)
			browser_error(browser, strerror(errno), 0);
		else
			gtk_list_store_set(browser->store, &iter, BR_COL_PATH,
					p, BR_COL_DISPLAY_NAME, arg2, -1);
		free(q);
	}
	else if(link(path, p) != 0 || unlink(path) != 0)
		browser_error(browser, strerror(errno), 0);
	else
		gtk_list_store_set(browser->store, &iter, BR_COL_PATH, p,
				BR_COL_DISPLAY_NAME, arg2, -1);
	free(p);
	free(path);
}


#if GTK_CHECK_VERSION(2, 8, 0)
/* on_view_drag_data_get */
void on_view_drag_data_get(GtkWidget * widget, GdkDragContext * context,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data)
	/* XXX could be more optimal */
{
	Browser * browser = data;
	GList * selection;
	GList * s;
	size_t len;
	unsigned char * p;

	selection = _copy_selection(browser);
	seldata->format = 8;
	seldata->data = NULL;
	seldata->length = 0;
	for(s = selection; s != NULL; s = s->next)
	{
		len = strlen(s->data) + 1;
		if((p = realloc(seldata->data, seldata->length + len)) == NULL)
			continue; /* XXX report error */
		seldata->data = p;
		memcpy(&p[seldata->length], s->data, len);
		seldata->length += len;
	}
	g_list_foreach(selection, (GFunc)free, NULL);
	g_list_free(selection);
}


/* on_view_drag_data_received */
void on_view_drag_data_received(GtkWidget * widget, GdkDragContext * context,
		gint x, gint y, GtkSelectionData * seldata, guint info,
		guint time, gpointer data)
	/* FIXME - may not be an icon view
	 *       - icon view may not be supported (< 2.6)
	 *       - not fully checking if the source matches */
{
	Browser * browser = data;
	GtkTreePath * path;
	GtkTreeIter iter;
	char * dest;

	path = gtk_icon_view_get_path_at_pos(GTK_ICON_VIEW(browser->iconview),
			x, y);
	if(path == NULL)
		dest = browser->current->data;
	else
	{
		gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter,
				path);
		gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter,
				BR_COL_PATH, &dest, -1);
	}
	if(_common_drag_data_received(context, seldata, dest) != 0)
		browser_error(browser, "fork", 0);
}
#endif


/* on_view_popup */
gboolean on_view_popup(GtkWidget * widget, gpointer data)
{
	GdkEventButton event;

	memset(&event, 0, sizeof(event));
	event.type = GDK_BUTTON_PRESS;
	event.button = 0;
	event.time = gtk_get_current_event_time();
	return on_view_press(widget, &event, data);
}


/* on_view_press */
/* types */
typedef struct _IconCallback
{
	Browser * browser;
	int isdir;
	int ismnt;
	char * path;
} IconCallback;

/* sub-functions */
static gboolean _press_context(Browser * browser, GdkEventButton * event,
		GtkWidget * menu, IconCallback * ic);
static void _press_directory(GtkWidget * menu, IconCallback * ic);
static void _press_file(Browser * browser, GtkWidget * menu, char * mimetype,
		IconCallback * ic);
static void _press_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, IconCallback * ic,
		GtkWidget * menu);
static gboolean _press_show(Browser * browser, GdkEventButton * event,
		GtkWidget * menu);

/* callbacks */
static void _on_icon_delete(GtkWidget * widget, gpointer data);
static void _on_icon_open(GtkWidget * widget, gpointer data);
static void _on_icon_edit(GtkWidget * widget, gpointer data);
static void _on_icon_open_with(GtkWidget * widget, gpointer data);
static void _on_icon_paste(GtkWidget * widget, gpointer data);
static void _on_icon_unmount(GtkWidget * widget, gpointer data);

gboolean on_view_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	static IconCallback ic;
	Browser * browser = data;
	GtkWidget * menu;
	GtkTreePath * path = NULL;
	GtkTreeIter iter;
	GtkTreeSelection * sel;
	GtkWidget * menuitem;
	char * mimetype = NULL;

	if(event->type != GDK_BUTTON_PRESS
			|| (event->button != 3 && event->button != 0))
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
	ic.browser = browser;
	ic.isdir = 0;
	ic.path = NULL;
	if(path == NULL)
		return _press_context(browser, event, menu, &ic);
	/* FIXME error checking + sub-functions */
	gtk_tree_model_get_iter(GTK_TREE_MODEL(browser->store), &iter, path);
#if GTK_CHECK_VERSION(2, 6, 0)
	if(browser->iconview != NULL)
	{
		if(gtk_icon_view_path_is_selected(GTK_ICON_VIEW(
						browser->iconview), path)
				== FALSE)
		{
			browser_unselect_all(browser);
			gtk_icon_view_select_path(GTK_ICON_VIEW(
						browser->iconview), path);
		}
	}
	else
#endif
	{
		sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(
					browser->detailview));
		if(!gtk_tree_selection_iter_is_selected(sel, &iter))
		{
			browser_unselect_all(browser);
			gtk_tree_selection_select_iter(sel, &iter);
		}
	}
	gtk_tree_model_get(GTK_TREE_MODEL(browser->store), &iter, BR_COL_PATH,
			&ic.path, BR_COL_IS_DIRECTORY, &ic.isdir,
			BR_COL_IS_MOUNT_POINT, &ic.ismnt, BR_COL_MIME_TYPE,
			&mimetype, -1);
	if(ic.isdir == TRUE)
		_press_directory(menu, &ic);
	else
		_press_file(browser, menu, mimetype, &ic);
	g_free(mimetype);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_properties), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
#if !GTK_CHECK_VERSION(2, 6, 0)
	gtk_tree_path_free(path);
#endif
	return _press_show(browser, event, menu);
}

static void _on_folder_new(GtkWidget * widget, gpointer data);
static gboolean _press_context(Browser * browser, GdkEventButton * event,
		GtkWidget * menu, IconCallback * ic)
{
	GtkWidget * menuitem;
	GtkWidget * submenu;
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkWidget * image;
#endif

	browser_unselect_all(browser);
	menuitem = gtk_menu_item_new_with_label("New");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
#if GTK_CHECK_VERSION(2, 8, 0) /* XXX actually depends on the icon theme */
	image = gtk_image_new_from_icon_name("folder-new", GTK_ICON_SIZE_MENU);
	menuitem = gtk_image_menu_item_new_with_label("Folder");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
#else
	menuitem = gtk_menu_item_new_with_label("Folder");
#endif
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_folder_new), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	if(browser->selection != NULL)
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					on_edit_paste), browser);
	else
		gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_properties), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	return _press_show(browser, event, menu);
}

static void _on_folder_new(GtkWidget * widget, gpointer data)
{
	IconCallback * ic = data;
	Browser * browser = ic->browser;

	on_file_new_folder(NULL, browser); /* XXX ugly */
}

static void _press_directory(GtkWidget * menu, IconCallback * ic)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_cut), ic->browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_copy), ic->browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	if(ic->browser->selection == NULL)
		gtk_widget_set_sensitive(menuitem, FALSE);
	else /* FIXME only if just this one is selected */
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					_on_icon_paste), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	if(ic->ismnt)
	{
		menuitem = gtk_menu_item_new_with_mnemonic("_Unmount");
		g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
					_on_icon_unmount), ic);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_delete), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _press_file(Browser * browser, GtkWidget * menu, char * mimetype,
		IconCallback * ic)
{
	GtkWidget * menuitem;

	_press_mime(browser->mime, mimetype, "open", GTK_STOCK_OPEN, G_CALLBACK(
				_on_icon_open), ic, menu);
	_press_mime(browser->mime, mimetype, "edit",
#if GTK_CHECK_VERSION(2, 6, 0)
			GTK_STOCK_EDIT,
#else
			"_Edit",
#endif
			G_CALLBACK(_on_icon_edit), ic, menu);
	menuitem = gtk_menu_item_new_with_mnemonic("Open _with...");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open_with), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_cut), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_copy), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_delete), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _press_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, IconCallback * ic,
		GtkWidget * menu)
{
	GtkWidget * menuitem;

	if(mime == NULL || mime_get_handler(mime, mimetype, action) == NULL)
		return;
	if(strncmp(label, "gtk-", 4) == 0)
		menuitem = gtk_image_menu_item_new_from_stock(label, NULL);
	else
		menuitem = gtk_menu_item_new_with_mnemonic(label);
	g_signal_connect(G_OBJECT(menuitem), "activate", callback, ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static gboolean _press_show(Browser * browser, GdkEventButton * event,
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
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button,
			event->time);
	return TRUE;
}

static void _on_icon_delete(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	/* FIXME not selected => cursor */
	on_edit_delete(GTK_MENU_ITEM(widget), cb->browser);
}

static void _on_icon_open(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	if(cb->isdir)
		browser_set_location(cb->browser, cb->path);
	else if(cb->browser->mime != NULL)
		mime_action(cb->browser->mime, "open", cb->path);
}

static void _on_icon_edit(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	if(cb->browser->mime != NULL)
		mime_action(cb->browser->mime, "edit", cb->path);
}

static void _on_icon_open_with(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	browser_open_with(cb->browser, cb->path);
}

static void _on_icon_paste(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;
	char * p = cb->browser->current->data;

	cb->browser->current->data = cb->path; /* XXX this is totally ugly */
	on_edit_paste(GTK_MENU_ITEM(widget), cb->browser);
	cb->browser->current->data = p;
}

static void _on_icon_unmount(GtkWidget * widget, gpointer data)
{
	IconCallback * cb = data;

	if(unmount(cb->path, 0) != 0)
		browser_error(cb->browser, strerror(errno), 0);
}
