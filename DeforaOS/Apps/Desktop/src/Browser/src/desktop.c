/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Browser; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */
/* FIXME:
 * - enable drag and drop as a source
 * - factorize code with browser (drag and drop, exec) */



#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "mime.h"
#include "desktop.h"
#define PACKAGE "desktop"
#include "common.c"


/* constants */
#define DESKTOP ".desktop"


/* DesktopIcon */
/* types */
struct _Desktop
{
	DesktopIcon ** icon;
	size_t icon_cnt;
	Mime * mime;
	char * path;
	size_t path_cnt;
	DIR * refresh_dir;
	time_t refresh_mti;

	GtkIconTheme * theme;
	GdkPixbuf * file;
	GdkPixbuf * folder;
	GdkPixbuf * executable;
};

struct _DesktopIcon
{
	Desktop * desktop;
	char * path;
	int isdir;
	char const * mimetype;

	gboolean selected;
	gboolean updated; /* XXX for desktop refresh */

	GtkWidget * window;
	GtkWidget * image;
	GtkWidget * event;
	GtkWidget * label;
};

/* constants */
#define DESKTOPICON_ICON_SIZE	48
#define DESKTOPICON_MAX_HEIGHT	100
#define DESKTOPICON_MAX_WIDTH	100
#define DESKTOPICON_MIN_HEIGHT	(DESKTOPICON_ICON_SIZE << 1)
#define DESKTOPICON_MIN_WIDTH	DESKTOPICON_MAX_WIDTH


/* functions */
/* private */
static void _desktopicon_update_transparency(DesktopIcon * desktopicon,
		GdkPixbuf * icon);


static void _desktopicon_update_transparency(DesktopIcon * desktopicon,
		GdkPixbuf * icon)
{
	int width;
	int height;
	GdkBitmap * mask;
	GdkBitmap * iconmask;
	GdkGC * gc;
	GdkColor black = { 0, 0, 0, 0 };
	GdkColor white = { 0xffffffff, 0xffff, 0xffff, 0xffff };
	GtkRequisition req;
	int offset;

	gtk_window_get_size(GTK_WINDOW(desktopicon->window), &width, &height);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: window is %dx%d\n", width, height);
#endif
	mask = gdk_pixmap_new(NULL, width, height, 1);
	gdk_pixbuf_render_pixmap_and_mask(icon, NULL, &iconmask, 255);
	gc = gdk_gc_new(mask);
	gdk_gc_set_foreground(gc, &black);
	gdk_draw_rectangle(mask, gc, TRUE, 0, 0, width, height);
	gdk_draw_drawable(mask, gc, iconmask, 0, 0,
			(width - DESKTOPICON_ICON_SIZE) / 2, 4, -1, -1);
	gdk_gc_set_foreground(gc, &white);
	gtk_widget_size_request(desktopicon->label, &req);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: label is %dx%d\n", req.width, req.height);
#endif
	offset = DESKTOPICON_ICON_SIZE + 8;
	gdk_draw_rectangle(mask, gc, TRUE, (width - req.width - 8) / 2,
			offset + ((height - offset - req.height - 8)
				/ 2), req.width + 8, req.height + 8);
	gtk_widget_shape_combine_mask(desktopicon->window, mask, 0, 0);
	g_object_unref(gc);
	g_object_unref(iconmask);
	g_object_unref(mask);
}


/* desktopicon_new */
/* callbacks */
static gboolean _on_desktopicon_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _on_icon_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data);
static gboolean _on_icon_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data);
static void _on_icon_drag_data_received(GtkWidget * widget,
		GdkDragContext * context, gint x, gint y,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data);

DesktopIcon * desktopicon_new(Desktop * desktop, char const * name,
		char const * path)
{
	DesktopIcon * desktopicon;
	GtkWindow * window;
	struct stat st;
	GdkGeometry geometry;
	GtkWidget * vbox;
	GtkWidget * eventbox;
	GtkTargetEntry targets[] = { { "deforaos_browser_dnd", 0, 0 } };
	size_t targets_cnt = sizeof(targets) / sizeof(*targets);
	GdkPixbuf * icon = NULL;
	char * p;
	GtkLabel * label;

	if((desktopicon = malloc(sizeof(*desktopicon))) == NULL)
		return NULL;
	if((desktopicon->path = strdup(path)) == NULL)
	{
		free(desktopicon);
		return NULL;
	}
	desktopicon->desktop = desktop;
	desktopicon->isdir = 0;
	desktopicon->mimetype = NULL;
	desktopicon->selected = 0;
	desktopicon->updated = 1;
	desktopicon->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = GTK_WINDOW(desktopicon->window);
	gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_set_resizable(window, FALSE);
	gtk_window_set_decorated(window, FALSE);
	gtk_window_set_keep_below(window, TRUE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(window, FALSE);
#endif
	g_signal_connect(G_OBJECT(desktopicon->window), "delete-event",
			G_CALLBACK(_on_desktopicon_closex), desktopicon);
	vbox = gtk_vbox_new(FALSE, 4);
	geometry.min_width = DESKTOPICON_MIN_WIDTH;
	geometry.min_height = DESKTOPICON_MIN_HEIGHT;
	geometry.max_width = DESKTOPICON_MAX_WIDTH;
	geometry.max_height = DESKTOPICON_MAX_HEIGHT;
	geometry.base_width = DESKTOPICON_MIN_WIDTH;
	geometry.base_height = DESKTOPICON_MIN_HEIGHT;
	gtk_window_set_geometry_hints(window, vbox, &geometry,
			GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE
			| GDK_HINT_BASE_SIZE);
	/* icon */
	if(stat(path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
		{
			desktopicon->isdir = 1;
			icon = desktop->folder;
		}
		else if(st.st_mode & S_IXUSR)
			icon = desktop->executable;
		else if((desktopicon->mimetype = mime_type(desktop->mime, path))
				!= NULL)
			mime_icons(desktop->mime, desktop->theme,
					desktopicon->mimetype,
					DESKTOPICON_ICON_SIZE, &icon, -1);
	}
	if(icon == NULL)
		icon = desktop->file;
	eventbox = gtk_event_box_new();
	gtk_drag_dest_set(eventbox, GTK_DEST_DEFAULT_ALL, targets, targets_cnt,
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(_on_icon_button_press), desktopicon);
	g_signal_connect(G_OBJECT(eventbox), "key-press-event",
			G_CALLBACK(_on_icon_key_press), desktopicon);
	g_signal_connect(G_OBJECT(eventbox), "drag-data-received",
			G_CALLBACK(_on_icon_drag_data_received), desktopicon);
	desktopicon->event = eventbox;
	desktopicon->image = gtk_image_new_from_pixbuf(icon);
	gtk_widget_set_size_request(desktopicon->image, DESKTOPICON_MIN_WIDTH,
			DESKTOPICON_ICON_SIZE);
	gtk_box_pack_start(GTK_BOX(vbox), desktopicon->image, FALSE, TRUE, 4);
	if((p = g_filename_to_utf8(name, -1, NULL, NULL, NULL)) != NULL)
		name = p;
	desktopicon->label = gtk_label_new(name);
	label = GTK_LABEL(desktopicon->label);
	gtk_label_set_justify(label, GTK_JUSTIFY_CENTER);
#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_label_set_line_wrap_mode(label, PANGO_WRAP_WORD_CHAR);
#endif
	gtk_label_set_line_wrap(label, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), desktopicon->label, TRUE, FALSE, 4);
	gtk_container_add(GTK_CONTAINER(eventbox), vbox);
	gtk_container_add(GTK_CONTAINER(desktopicon->window), eventbox);
	_desktopicon_update_transparency(desktopicon, icon);
	return desktopicon;
}

/* callbacks */
static gboolean _on_desktopicon_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	DesktopIcon * di = data;

	gtk_widget_hide(widget);
	desktopicon_delete(di);
	return TRUE;
}

/* FIXME some code is duplicated from callback.c */
/* types */
static void _popup_directory(GtkWidget * menu, DesktopIcon * desktopicon);
static void _popup_file(GtkWidget * menu, DesktopIcon * desktopicon);
static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, DesktopIcon * icon,
		GtkWidget * menu);
/* callbacks */
static void _on_icon_open(GtkWidget * widget, gpointer data);
static void _on_icon_edit(GtkWidget * widget, gpointer data);
static void _on_icon_open_with(GtkWidget * widget, gpointer data);
static void _on_icon_delete(GtkWidget * widget, gpointer data);
static void _on_icon_properties(GtkWidget * widget, gpointer data);

static gboolean _on_icon_button_press(GtkWidget * widget,
		GdkEventButton * event, gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * menu;
	GtkWidget * menuitem;

	if(event->state & GDK_CONTROL_MASK)
		desktopicon_set_selected(desktopicon, !desktopicon_get_selected(
					desktopicon));
	else
	{
		desktop_unselect_all(desktopicon->desktop);
		desktopicon_set_selected(desktopicon, TRUE);
	}
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
	{
		_on_icon_open(widget, desktopicon);
		return FALSE;
	}
	if(event->type != GDK_BUTTON_PRESS || event->button != 3)
		return FALSE;
	menu = gtk_menu_new();
	if(desktopicon->isdir)
		_popup_directory(menu, desktopicon);
	else
		_popup_file(menu, desktopicon);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_delete), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_properties), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, event->time);
	return TRUE;
}

static void _popup_directory(GtkWidget * menu, DesktopIcon * desktopicon)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_file(GtkWidget * menu, DesktopIcon * desktopicon)
{
	GtkWidget * menuitem;

	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "open",
			GTK_STOCK_OPEN, G_CALLBACK(_on_icon_open), desktopicon,
			menu);
	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "edit",
#if GTK_CHECK_VERSION(2, 6, 0)
			GTK_STOCK_EDIT,
#else
			"_Edit",
#endif
			G_CALLBACK(_on_icon_edit), desktopicon, menu);
	menuitem = gtk_menu_item_new_with_mnemonic("Open _with...");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open_with), desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback,
		DesktopIcon * desktopicon, GtkWidget * menu)
{
	GtkWidget * menuitem;

	if(mime_get_handler(mime, mimetype, action) == NULL)
		return;
	if(strncmp(label, "gtk-", 4) == 0)
		menuitem = gtk_image_menu_item_new_from_stock(label, NULL);
	else
		menuitem = gtk_menu_item_new_with_mnemonic(label);
	g_signal_connect(G_OBJECT(menuitem), "activate", callback, desktopicon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _on_icon_open(GtkWidget * widget, gpointer data)
{
	DesktopIcon * desktopicon = data;
	pid_t pid;

	if(desktopicon->isdir == 0)
	{
		if(desktopicon->desktop->mime != NULL) /* XXX ugly */
			if(mime_action(desktopicon->desktop->mime, "open",
					desktopicon->path) != 0)
				_on_icon_open_with(widget, desktopicon);
		return;
	}
	if((pid = fork()) == -1)
	{
		desktop_error(desktopicon->desktop, strerror(errno), 0);
		return;
	}
	if(pid != 0)
		return;
	execlp("browser", "browser", "--", desktopicon->path, NULL);
	fprintf(stderr, "%s%s\n", "desktop: browser: ", strerror(errno));
	exit(2);
}

static void _on_icon_edit(GtkWidget * widget, gpointer data)
{
	DesktopIcon * desktopicon = data;

	mime_action(desktopicon->desktop->mime, "edit", desktopicon->path);
}

static void _on_icon_open_with(GtkWidget * widget, gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * dialog;
	char * filename = NULL;
	pid_t pid;

	dialog = gtk_file_chooser_dialog_new("Open with...",
			GTK_WINDOW(desktopicon->window),
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
		desktop_error(desktopicon->desktop, "fork", 0);
	else if(pid == 0)
	{
		execlp(filename, filename, desktopicon->path, NULL);
		desktop_error(NULL, filename, 0);
		exit(2);
	}
	g_free(filename);
}

static void _on_icon_delete(GtkWidget * widget, gpointer data)
{
	DesktopIcon * desktopicon = data;

	/* FIXME actually delete the file, and wait for the refresh */
	desktop_icon_remove(desktopicon->desktop, desktopicon);
}

static void _on_icon_properties(GtkWidget * widget, gpointer data)
{
	DesktopIcon * desktopicon = data;
	pid_t pid;

	if((pid = fork()) == -1)
	{
		desktop_error(desktopicon->desktop, "fork", 0);
		return;
	}
	else if(pid != 0)
		return;
	execlp("properties", "properties", "--", desktopicon->path, NULL);
	desktop_error(NULL, "properties", 0);
	exit(2);
}

static gboolean _on_icon_key_press(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
	/* FIXME handle shift and control */
{
	DesktopIcon * desktopicon = data;

	if(event->type != GDK_KEY_PRESS)
		return FALSE;
	if(event->keyval == GDK_uparrow)
	{
		desktop_unselect_all(desktopicon->desktop);
		desktop_select_above(desktopicon->desktop, desktopicon);
	}
	else if(event->keyval == GDK_downarrow)
	{
		desktop_unselect_all(desktopicon->desktop);
		desktop_select_under(desktopicon->desktop, desktopicon);
	}
	else /* not handling it */
		return FALSE;
	return TRUE;
}

static void _on_icon_drag_data_received(GtkWidget * widget,
		GdkDragContext * context, gint x, gint y,
		GtkSelectionData * seldata, guint info, guint time,
		gpointer data)
{
	DesktopIcon * desktopicon = data;
	size_t len;
	size_t i;
	GList * selection = NULL;
#ifdef DEBUG
	GList * s;
#else
	int ret = 0;
#endif

	if(seldata->length <= 0 || seldata->data == NULL)
		return;
	len = seldata->length;
	for(i = 0; i < len; i += strlen((char*)&seldata->data[i]) + 1)
		selection = g_list_append(selection, &seldata->data[i]);
#ifdef DEBUG
	fprintf(stderr, "%s%s%s%s%s", "DEBUG: ",
			context->suggested_action == GDK_ACTION_COPY ? "copying"
			: "moving", " to \"", desktopicon->path, "\":\n");
	for(s = selection; s != NULL; s = s->next)
		fprintf(stderr, "DEBUG: \"%s\" to \"%s\"\n", seldata->data,
				desktopicon->path);
#else
	selection = g_list_append(selection, desktopicon->path);
	if(context->suggested_action == GDK_ACTION_COPY)
		ret = _common_exec("copy", "-ir", selection);
	else if(context->suggested_action == GDK_ACTION_MOVE)
		ret = _common_exec("move", "-i", selection);
	if(ret != 0)
		desktop_error(desktopicon->desktop, "fork", 0);
#endif
	g_list_free(selection);
}


/* desktopicon_delete */
void desktopicon_delete(DesktopIcon * desktopicon)
{
	free(desktopicon->path);
	gtk_widget_destroy(desktopicon->window);
	free(desktopicon);
}


/* accessors */
char const * desktopicon_get_path(DesktopIcon * desktopicon)
{
	return desktopicon->path;
}


gboolean desktopicon_get_selected(DesktopIcon * desktopicon)
{
	return desktopicon->selected;
}


void desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon)
{
	gtk_image_set_from_pixbuf(GTK_IMAGE(desktopicon->image), icon);
	_desktopicon_update_transparency(desktopicon, icon);
}


void desktopicon_set_selected(DesktopIcon * desktopicon, gboolean selected)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %p is %s\n", desktopicon, selected ? "selected"
			: "deselected");
#endif
	desktopicon->selected = selected;
	gtk_widget_set_state(desktopicon->event, selected
			? GTK_STATE_SELECTED : GTK_STATE_NORMAL);
}


/* useful */
/* desktopicon_move */
void desktopicon_move(DesktopIcon * desktopicon, int x, int y)
{
	gtk_window_move(GTK_WINDOW(desktopicon->window), x, y);
}


/* desktopicon_show */
void desktopicon_show(DesktopIcon * desktopicon)
{
	gtk_widget_show_all(desktopicon->window);
}


/* Desktop */
/* functions */
/* desktop_new */
static Desktop * _new_error(Desktop * desktop, char const * message);
static int _new_create_desktop(Desktop * desktop, char const * home);
static void _new_add_home(Desktop * desktop, char const * home);

/* callbacks */
/* FIXME implement desktop resizing callback */

Desktop * desktop_new(void)
{
	Desktop * desktop;
	char * home;
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

	if((desktop = malloc(sizeof(*desktop))) == NULL)
		return NULL;
	desktop->icon = NULL;
	desktop->icon_cnt = 0;
	desktop->path = NULL;
	if((desktop->mime = mime_new()) == NULL
			|| (desktop->icon = malloc(sizeof(*(desktop->icon))
					* desktop->icon_cnt)) == NULL)
	{
		desktop_delete(desktop);
		return NULL;
	}
	desktop->theme = gtk_icon_theme_get_default();
	desktop->file = NULL;
	for(p = file; *p != NULL && desktop->file == NULL; p++)
		desktop->file = gtk_icon_theme_load_icon(desktop->theme,
				*p, DESKTOPICON_ICON_SIZE, 0, NULL);
	desktop->folder = NULL;
	for(p = folder; *p != NULL && desktop->folder == NULL; p++)
		desktop->folder = gtk_icon_theme_load_icon(desktop->theme,
				*p, DESKTOPICON_ICON_SIZE, 0, NULL);
	desktop->executable = NULL;
	for(p = executable; *p != NULL && desktop->executable == NULL; p++)
		desktop->executable = gtk_icon_theme_load_icon(desktop->theme,
				*p, DESKTOPICON_ICON_SIZE, 0, NULL);
	if((home = getenv("HOME")) == NULL)
		return _new_error(desktop, "HOME");
	if(_new_create_desktop(desktop, home) != 0)
		return _new_error(desktop, "Creating desktop");
	_new_add_home(desktop, home);
	desktop_refresh(desktop);
	return desktop;
}

static Desktop * _new_error(Desktop * desktop, char const * message)
{
	desktop_error(desktop, message, -1);
	desktop_delete(desktop);
	return NULL;
}

static int _new_create_desktop(Desktop * desktop, char const * home)
{
	struct stat st;

	desktop->path_cnt = strlen(home) + strlen("/" DESKTOP) + 1;
	if((desktop->path = malloc(desktop->path_cnt)) == NULL)
		return 1;
	sprintf(desktop->path, "%s%s", home, "/" DESKTOP);
	if(lstat(desktop->path, &st) == 0)
	{
		if(!S_ISDIR(st.st_mode))
		{
			errno = ENOTDIR;
			return 1;
		}
	}
	else if(mkdir(desktop->path, 0777) != 0)
		return 1;
	return 0;
}

static void _new_add_home(Desktop * desktop, char const * home)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	if((desktopicon = desktopicon_new(desktop, "Home", home)) == NULL)
		return;
	desktop_icon_add(desktop, desktopicon);
	icon = gtk_icon_theme_load_icon(desktop->theme, "gnome-home",
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon == NULL)
		icon = gtk_icon_theme_load_icon(desktop->theme,
				"gnome-fs-home", DESKTOPICON_ICON_SIZE,
				0, NULL);
	if(icon != NULL)
		desktopicon_set_icon(desktopicon, icon);
}


/* desktop_delete */
void desktop_delete(Desktop * desktop)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
		desktopicon_delete(desktop->icon[i]);
	free(desktop->icon);
	if(desktop->mime != NULL)
		mime_delete(desktop->mime);
	free(desktop->path);
	free(desktop);
}


/* useful */
/* desktop_error */
static int _error_text(char const * message, int ret);
int desktop_error(Desktop * desktop, char const * message, int ret)
{
	GtkWidget * dialog;

	if(desktop == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s: %s", message, strerror(errno));
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	if(ret < 0)
	{
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
					gtk_main_quit), NULL);
		ret = -ret;
	}
	else
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(
					gtk_widget_destroy), NULL);
	gtk_widget_show(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs("desktop: ", stderr);
	perror(message);
	return ret;
}


static void _refresh_current(Desktop * desktop);
void desktop_refresh(Desktop * desktop)
{
	int fd;
	struct stat st;

#ifdef __sun__
	if((fd = open(desktop->path, O_RDONLY)) < 0
			|| fstat(fd, &st) != 0
			|| (desktop->refresh_dir = fdopendir(fd)) == NULL)
	{
		desktop_error(desktop, desktop->path, 0);
		if(fd >= 0)
			close(fd);
		return;
	}
#else
	if((desktop->refresh_dir = opendir(desktop->path)) == NULL)
	{
		desktop_error(desktop, desktop->path, 0);
		return;
	}
	fd = dirfd(desktop->refresh_dir);
	if(fstat(fd, &st) != 0)
	{
		desktop_error(desktop, desktop->path, 0);
		closedir(desktop->refresh_dir);
		return;
	}
#endif
	desktop->refresh_mti = st.st_mtime;
	_refresh_current(desktop);
}

static int _current_loop(Desktop * desktop);
static gboolean _current_idle(gpointer data);
static gboolean _current_done(Desktop * desktop);
static void _refresh_current(Desktop * desktop)
{
	unsigned int i;

	for(i = 0; i < 16 && _current_loop(desktop) == 0; i++);
	if(i == 16)
		g_idle_add(_current_idle, desktop);
	else
		_current_done(desktop);
}

static int _loop_lookup(Desktop * desktop, char const * name);
static int _current_loop(Desktop * desktop)
{
	struct dirent * de;
	char * p;
	DesktopIcon * desktopicon;

	while((de = readdir(desktop->refresh_dir)) != NULL)
	{
		if(de->d_name[0] == '.')
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
		if(_loop_lookup(desktop, de->d_name) == 1)
			continue;
		break;
	}
	if(de == NULL)
		return 1;
	if((p = realloc(desktop->path, desktop->path_cnt + strlen(de->d_name)
					+ 1)) == NULL)
	{
		desktop_error(NULL, "realloc", 0);
		return 1;
	} /* FIXME avoid calling realloc() at every pass */
	desktop->path = p;
	sprintf(&desktop->path[desktop->path_cnt - 1], "/%s", de->d_name);
	if((desktopicon = desktopicon_new(desktop, de->d_name, desktop->path))
			!= NULL)
		desktop_icon_add(desktop, desktopicon);
	desktop->path[desktop->path_cnt - 1] = '\0';
	return 0;
}

static int _loop_lookup(Desktop * desktop, char const * name)
{
	size_t i;
	char const * p;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktop->icon[i]->updated == 1) /* XXX internal knowledge */
			continue;
		if((p = desktopicon_get_path(desktop->icon[i])) == NULL
				|| (p = strrchr(p, '/')) == NULL)
			continue;
		if(strcmp(name, ++p) != 0)
			continue;
		desktop->icon[i]->updated = 1; /* XXX here too */
		return 1;
	}
	return 0;
}

static gboolean _current_idle(gpointer data)
{
	Desktop * desktop = data;
	unsigned int i;

	for(i = 0; i < 16 && _current_loop(desktop) == 0; i++);
	if(i == 16)
		return TRUE;
	return _current_done(desktop);
}

static gboolean _done_timeout(gpointer data);
static gboolean _current_done(Desktop * desktop)
{
	size_t i = 1;

	while(i < desktop->icon_cnt)
		if(desktop->icon[i]->updated != 1)
			desktop_icon_remove(desktop, desktop->icon[i]);
		else
			desktop->icon[i++]->updated = 0;
	closedir(desktop->refresh_dir);
	g_timeout_add(1000, _done_timeout, desktop);
	return FALSE;
}

static gboolean _done_timeout(gpointer data)
{
	Desktop * desktop = data;
	struct stat st;

	if(stat(desktop->path, &st) != 0)
		return desktop_error(NULL, desktop->path, FALSE);
	if(st.st_mtime == desktop->refresh_mti)
		return TRUE;
	desktop_refresh(desktop);
	return FALSE;
}


/* desktop_icon_add */
void desktop_icon_add(Desktop * desktop, DesktopIcon * icon)
{
	DesktopIcon ** p;

	if((p = realloc(desktop->icon, sizeof(*p) * (desktop->icon_cnt + 1)))
			== NULL)
	{
		desktop_error(desktop, "Adding icon", 0);
		return;
	}
	desktop->icon = p;
	desktop->icon[desktop->icon_cnt++] = icon;
	desktop_icons_align(desktop);
	desktopicon_show(icon);
}


/* desktop_icon_remove */
void desktop_icon_remove(Desktop * desktop, DesktopIcon * icon)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktop->icon[i] != icon)
			continue;
		desktopicon_delete(icon);
		desktop->icon_cnt--;
		for(; i < desktop->icon_cnt; i++)
			desktop->icon[i] = desktop->icon[i + 1];
	}
	desktop_icons_align(desktop);
}


/* desktop_icons_align */
static int _align_compare(const void * a,  const void * b);
void desktop_icons_align(Desktop * desktop)
{
	GdkScreen * screen;
	int height = INT_MAX;
	size_t i;
	int x = 0;
	int y = 0;

	qsort(desktop->icon, desktop->icon_cnt, sizeof(void*), _align_compare);
	if((screen = gdk_screen_get_default()) != NULL)
		height = gdk_screen_get_height(screen);
	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(y + DESKTOPICON_MAX_HEIGHT > height)
		{
			x += DESKTOPICON_MAX_WIDTH;
			y = 0;
		}
		desktopicon_move(desktop->icon[i], x, y);
		y += DESKTOPICON_MAX_HEIGHT;
	}
}

static int _align_compare(const void * a, const void * b)
{
	DesktopIcon * icona = *(DesktopIcon**)a;
	DesktopIcon * iconb = *(DesktopIcon**)b;

	return strcmp(desktopicon_get_path(icona), desktopicon_get_path(iconb));
}


/* desktop_select_all */
void desktop_select_all(Desktop * desktop)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
		desktopicon_set_selected(desktop->icon[i], TRUE);
}


/* desktop_select_above */
void desktop_select_above(Desktop * desktop, DesktopIcon * icon)
	/* FIXME icons may be wrapped */
{
	size_t i;

	for(i = 1; i < desktop->icon_cnt; i++)
		if(desktop->icon[i] == icon)
		{
			desktopicon_set_selected(desktop->icon[i], TRUE);
			return;
		}
}


/* desktop_select_under */
void desktop_select_under(Desktop * desktop, DesktopIcon * icon)
	/* FIXME icons may be wrapped */
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
		if(desktop->icon[i] == icon && i + 1 < desktop->icon_cnt)
		{
			desktopicon_set_selected(desktop->icon[i], TRUE);
			return;
		}
}


/* desktop_unselect_all */
void desktop_unselect_all(Desktop * desktop)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
		desktopicon_set_selected(desktop->icon[i], FALSE);
}


/* usage */
static int _usage(void)
{
	fputs("Usage: desktop\n", stderr);
	return 1;
}


/* main */
static void _main_sigchld(int signum);

int main(int argc, char * argv[])
{
	int o;
	Desktop * desktop;
	struct sigaction sa;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind < argc)
		return _usage();
	if((desktop = desktop_new()) == NULL)
	{
		gtk_main();
		return 2;
	}
	sa.sa_handler = _main_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		desktop_error(desktop, "signal handling error", 0);
	gtk_main();
	desktop_delete(desktop);
	return 0;
}

static void _main_sigchld(int signum)
{
	wait(NULL);
}
