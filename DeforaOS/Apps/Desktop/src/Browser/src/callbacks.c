/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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



#include <sys/param.h>
#ifndef __GNU__ /* XXX hurd portability */
# include <sys/mount.h>
# if defined(__linux__) || defined(__CYGWIN__)
#  define unmount(a, b) umount(a)
# endif
# ifndef unmount
#  define unmount unmount
# endif
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include "callbacks.h"
#include "browser.h"
#define _(string) gettext(string)

#define PROGNAME "browser"
#define COMMON_DND
#define COMMON_EXEC
#define COMMON_SYMLINK
#include "common.c"


/* private */
/* prototypes */
static GList * _copy_selection(Browser * browser);
static void _paste_selection(Browser * browser);


/* public */
/* functions */
/* callbacks */
/* window */
gboolean on_closex(gpointer data)
{
	Browser * browser = data;

	browser_delete(browser);
	if(browser_cnt == 0)
		gtk_main_quit();
	return FALSE;
}


/* accelerators */
/* on_close */
gboolean on_close(gpointer data)
{
	on_closex(data);
	return FALSE;
}


/* on_location */
gboolean on_location(gpointer data)
{
	Browser * browser = data;

	browser_focus_location(browser);
	return FALSE;
}


/* on_new */
gboolean on_new_window(gpointer data)
{
	Browser * browser = data;

	browser_new_copy(browser);
	return FALSE;
}


/* on_open_file */
gboolean on_open_file(gpointer data)
{
	Browser * browser = data;

	browser_open(browser, NULL);
	return FALSE;
}


/* file menu */
void on_file_new_window(gpointer data)
{
	on_new_window(data);
}


void on_file_new_folder(gpointer data)
{
	char const * newfolder = _("New folder");
	Browser * browser = data;
	char const * cur = browser->current->data;
	size_t len;
	char * path;

	len = strlen(cur) + strlen(newfolder) + 2;
	if((path = malloc(len)) == NULL)
	{
		browser_error(browser, "malloc", 0);
		return;
	}
	snprintf(path, len, "%s/%s", cur, newfolder);
	if(mkdir(path, 0777) != 0)
		browser_error(browser, path, 0);
	free(path);
}


void on_file_new_symlink(gpointer data)
{
	Browser * browser = data;
	char const * cur = browser->current->data;

	if(_common_symlink(browser->window, cur) != 0)
		browser_error(browser, "symlink", 0);
}


void on_file_close(gpointer data)
{
	on_closex(data);
}


void on_file_open_file(gpointer data)
{
	on_open_file(data);
}


/* edit menu */
/* on_edit_copy */
void on_edit_copy(gpointer data)
{
	on_copy(data);
}


/* on_edit_cut */
void on_edit_cut(gpointer data)
{
	on_cut(data);
}


/* on_edit_delete */
void on_edit_delete(gpointer data)
{
	Browser * browser = data;
	GtkWidget * dialog;
	unsigned long cnt = 0;
	int res = GTK_RESPONSE_YES;
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
#if GTK_CHECK_VERSION(2, 6, 0)
				"%s", _("Warning"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
					dialog),
#endif
				_("Are you sure you want to delete %lu"
					" file(s)?"), cnt);
		gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
		res = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	if(res == GTK_RESPONSE_YES
			&& _common_exec("delete", "-ir", selection) != 0)
		browser_error(browser, "fork", 0);
	g_list_foreach(selection, (GFunc)free, NULL);
	g_list_free(selection);
}


/* on_edit_paste */
void on_edit_paste(gpointer data)
{
	on_paste(data);
}


/* on_edit_select_all */
void on_edit_select_all(gpointer data)
{
	Browser * browser = data;

	browser_select_all(browser);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	Browser * browser = data;

	browser_view_preferences(browser);
}


/* on_edit_unselect_all */
void on_edit_unselect_all(gpointer data)
{
	Browser * browser = data;

	browser_unselect_all(browser);
}


/* view menu */
void on_view_home(gpointer data)
{
	on_home(data);
}


#if GTK_CHECK_VERSION(2, 6, 0)
/* on_view_details */
void on_view_details(gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_DETAILS);
}


/* on_view_icons */
void on_view_icons(gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_ICONS);
}


/* on_view_list */
void on_view_list(gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_LIST);
}


/* on_view_thumbnails */
void on_view_thumbnails(gpointer data)
{
	Browser * browser = data;

	browser_set_view(browser, BV_THUMBNAILS);
}
#endif /* GTK_CHECK_VERSION(2, 6, 0) */


/* help menu */
/* on_help_about */
void on_help_about(gpointer data)
{
	Browser * browser = data;

	browser_about(browser);
}


/* toolbar */
void on_back(gpointer data)
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


/* on_copy */
void on_copy(gpointer data)
{
	Browser * browser = data;
	GtkWidget * entry;

	entry = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if(gtk_window_get_focus(GTK_WINDOW(browser->window)) == entry)
	{
		gtk_editable_copy_clipboard(GTK_EDITABLE(entry));
		return;
	}
	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	browser->selection = _copy_selection(browser);
	browser->selection_cut = 0;
}


/* on_cut */
void on_cut(gpointer data)
{
	Browser * browser = data;
	GtkWidget * entry;

	entry = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if(gtk_window_get_focus(GTK_WINDOW(browser->window)) == entry)
	{
		gtk_editable_cut_clipboard(GTK_EDITABLE(entry));
		return;
	}
	g_list_foreach(browser->selection, (GFunc)free, NULL);
	g_list_free(browser->selection);
	browser->selection = _copy_selection(browser);
	browser->selection_cut = 1;
}


void on_forward(gpointer data)
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


void on_home(gpointer data)
{
	Browser * browser = data;

	browser_go_home(browser);
}


/* on_paste */
void on_paste(gpointer data)
{
	Browser * browser = data;
	GtkWidget * entry;

	entry = gtk_bin_get_child(GTK_BIN(browser->tb_path));
	if(gtk_window_get_focus(GTK_WINDOW(browser->window)) == entry)
	{
		gtk_editable_paste_clipboard(GTK_EDITABLE(entry));
		return;
	}
	_paste_selection(browser);
}


/* properties */
void on_properties(gpointer data)
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


void on_refresh(gpointer data)
{
	Browser * browser = data;

	browser_refresh(browser);
}


void on_updir(gpointer data)
{
	Browser * browser = data;
	char * dir;

	dir = g_path_get_dirname(browser->current->data);
	browser_set_location(browser, dir);
	g_free(dir);
}


#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_as(gpointer data)
{
	Browser * browser = data;

	if(browser->iconview == NULL)
		browser_set_view(browser, BV_ICONS);
	else switch(browser->iconview_as)
	{
		case BV_DETAILS:
			browser_set_view(browser, BV_ICONS);
			break;
		case BV_LIST:
			browser_set_view(browser, BV_THUMBNAILS);
			break;
		case BV_ICONS:
			browser_set_view(browser, BV_LIST);
			break;
		case BV_THUMBNAILS:
			browser_set_view(browser, BV_DETAILS);
			break;
	}
}
#endif


/* address bar */
void on_path_activate(gpointer data)
{
	Browser * browser = data;
	GtkWidget * widget;
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

	if(GTK_TREE_VIEW(browser->detailview) != view)
		return;
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
void on_icon_default(GtkIconView * view, GtkTreePath * path, gpointer data)
{
	Browser * browser = data;

	if(GTK_ICON_VIEW(browser->iconview) != view)
		return;
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
		if(rename(path, (q != NULL) ? q : p) != 0)
			browser_error(browser, path, 0);
		else
			gtk_list_store_set(browser->store, &iter, BR_COL_PATH,
					p, BR_COL_DISPLAY_NAME, arg2, -1);
		free(q);
	}
	else if(lstat(path, &st) == 0 && S_ISLNK(st.st_mode))
	{
		if(rename(path, p) != 0)
			browser_error(browser, path, 0);
	}
	else if(link(path, p) != 0 || unlink(path) != 0)
		browser_error(browser, path, 0);
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
#endif /* GTK_CHECK_VERSION(2, 8, 0) */


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
	int isexec;
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
static void _on_icon_delete(gpointer data);
static void _on_icon_open(gpointer data);
static void _on_icon_open_new_window(gpointer data);
static void _on_icon_edit(gpointer data);
static void _on_icon_open_with(gpointer data);
static void _on_icon_run(gpointer data);
static void _on_icon_paste(gpointer data);
static void _on_icon_unmount(gpointer data);

gboolean on_view_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	static IconCallback ic;
	Browser * browser = data;
	GtkTreePath * path = NULL;
	GtkTreeIter iter;
	GtkTreeSelection * sel;
	GtkWidget * menuitem;
	char * mimetype = NULL;

	if(event->type != GDK_BUTTON_PRESS
			|| (event->button != 3 && event->button != 0))
		return FALSE;
	widget = gtk_menu_new();
	/* FIXME prevents actions to be called but probably leaks memory
	g_signal_connect(G_OBJECT(widget), "deactivate", G_CALLBACK(
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
	ic.isexec = 0;
	ic.path = NULL;
	if(path == NULL)
		return _press_context(browser, event, widget, &ic);
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
			BR_COL_IS_EXECUTABLE, &ic.isexec,
			BR_COL_IS_MOUNT_POINT, &ic.ismnt, BR_COL_MIME_TYPE,
			&mimetype, -1);
	if(ic.isdir == TRUE)
		_press_directory(widget, &ic);
	else
		_press_file(browser, widget, mimetype, &ic);
	g_free(mimetype);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(widget), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_properties), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(widget), menuitem);
#if !GTK_CHECK_VERSION(2, 6, 0)
	gtk_tree_path_free(path);
#endif
	return _press_show(browser, event, widget);
}

static void _on_popup_new_text_file(gpointer data);
static void _on_popup_new_folder(gpointer data);
static void _on_popup_new_symlink(gpointer data);
static gboolean _press_context(Browser * browser, GdkEventButton * event,
		GtkWidget * menu, IconCallback * ic)
{
	GtkWidget * menuitem;
	GtkWidget * submenu;
#if GTK_CHECK_VERSION(2, 8, 0)
	GtkWidget * image;
#endif

	browser_unselect_all(browser);
	/* new submenu */
	menuitem = gtk_menu_item_new_with_label(_("New"));
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
#if GTK_CHECK_VERSION(2, 8, 0) /* XXX actually depends on the icon theme */
	menuitem = gtk_image_menu_item_new_with_label(_("Folder"));
	image = gtk_image_new_from_icon_name("folder-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
#else
	menuitem = gtk_menu_item_new_with_label(_("Folder"));
#endif
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_folder), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_menu_item_new_with_label(_("Symbolic link..."));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_symlink), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label(_("Text file"));
	image = gtk_image_new_from_icon_name("stock_new-text",
			GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_text_file), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	/* cut/copy/paste */
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
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_paste), ic);
	else
		gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_properties), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	return _press_show(browser, event, menu);
}

static void _on_popup_new_text_file(gpointer data)
{
	char const * newtext = _("New text file.txt");
	IconCallback * ic = data;
	Browser * browser = ic->browser;
	char const * cur = browser->current->data;
	size_t len;
	char * path;
	int fd;

	len = strlen(cur) + strlen(newtext) + 2;
	if((path = malloc(len)) == NULL)
	{
		browser_error(browser, "malloc", 0);
		return;
	}
	snprintf(path, len, "%s/%s", cur, newtext);
	if((fd = creat(path, 0666)) < 0)
		browser_error(browser, path, 0);
	else
		close(fd);
	free(path);
}

static void _on_popup_new_folder(gpointer data)
{
	IconCallback * ic = data;
	Browser * browser = ic->browser;

	on_file_new_folder(browser);
}

static void _on_popup_new_symlink(gpointer data)
{
	IconCallback * ic = data;
	Browser * browser = ic->browser;

	on_file_new_symlink(browser);
}

static void _press_directory(GtkWidget * menu, IconCallback * ic)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_with_mnemonic(
			_("Open in new _window"));
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem),
			gtk_image_new_from_icon_name("window-new",
				GTK_ICON_SIZE_MENU));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open_new_window), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_cut), ic->browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_copy), ic->browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	if(ic->browser->selection == NULL)
		gtk_widget_set_sensitive(menuitem, FALSE);
	else /* FIXME only if just this one is selected */
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_paste), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	if(ic->ismnt)
	{
		menuitem = gtk_menu_item_new_with_mnemonic(_("_Unmount"));
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_unmount), ic);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
		menuitem = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
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
	if(ic->isexec)
	{
		menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_EXECUTE,
				NULL);
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
				G_CALLBACK(_on_icon_run), ic);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	menuitem = gtk_menu_item_new_with_mnemonic(_("Open _with..."));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open_with), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_cut), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				on_edit_copy), browser);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	gtk_widget_set_sensitive(menuitem, FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
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
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", callback, ic);
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

static void _on_icon_delete(gpointer data)
{
	IconCallback * cb = data;

	/* FIXME not selected => cursor */
	on_edit_delete(cb->browser);
}

static void _on_icon_open(gpointer data)
{
	IconCallback * cb = data;

	if(cb->isdir)
		browser_set_location(cb->browser, cb->path);
	else if(cb->browser->mime != NULL)
		mime_action(cb->browser->mime, "open", cb->path);
}

static void _on_icon_open_new_window(gpointer data)
{
	IconCallback * cb = data;

	if(!cb->isdir)
		return;
	browser_new(cb->path);
}

static void _on_icon_edit(gpointer data)
{
	IconCallback * cb = data;

	if(cb->browser->mime != NULL)
		mime_action(cb->browser->mime, "edit", cb->path);
}

static void _on_icon_run(gpointer data)
	/* FIXME does not work with scripts */
{
	IconCallback * cb = data;
	GtkWidget * dialog;
	int res;
	GError * error = NULL;
	char * argv[2];

	dialog = gtk_message_dialog_new(GTK_WINDOW(cb->browser->window),
			GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
			GTK_BUTTONS_YES_NO, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Warning"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s",
#endif
			_("Are you sure you want to execute this file?"));
	gtk_window_set_title(GTK_WINDOW(dialog), _("Warning"));
	res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if(res != GTK_RESPONSE_YES)
		return;
	argv[0] = cb->path;
	if(g_spawn_async(NULL, argv, NULL, 0, NULL, NULL, NULL, &error) != TRUE)
		browser_error(cb->browser, cb->path, 1);
}

static void _on_icon_open_with(gpointer data)
{
	IconCallback * cb = data;

	browser_open_with(cb->browser, cb->path);
}

static void _on_icon_paste(gpointer data)
{
	IconCallback * cb = data;
	char * p;

	/* XXX this is totally ugly */
	p = cb->browser->current->data;
	if(cb->path != NULL)
		cb->browser->current->data = cb->path;
	_paste_selection(cb->browser);
	cb->browser->current->data = p;
}

static void _on_icon_unmount(gpointer data)
{
	IconCallback * cb = data;

#ifndef unmount
	errno = ENOSYS;
#else
	if(unmount(cb->path, 0) != 0)
#endif
		browser_error(cb->browser, cb->path, 0);
}


/* private */
/* functions */
/* copy_selection */
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


/* paste_selection */
static void _paste_selection(Browser * browser)
{
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
