/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
/* Browser is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Browser is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Browser; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA */



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
#include "mime.h"
#include "desktop.h"


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

	int updated; /* XXX for desktop refresh */

	GtkWidget * window;
	GtkWidget * image;
	GtkWidget * label;
};

/* constants */
#define DESKTOPICON_MAX_HEIGHT	100
#define DESKTOPICON_MAX_WIDTH	100
#define DESKTOPICON_MIN_HEIGHT	56			/* image and borders */
#define DESKTOPICON_MIN_WIDTH	DESKTOPICON_MAX_WIDTH	/* constant width */

#define DESKTOPICON_ICON_SIZE	48


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

	gtk_window_get_size(GTK_WINDOW(desktopicon->window), &width, &height);
	mask = gdk_pixmap_new(NULL, width, height, 1);
	gdk_pixbuf_render_pixmap_and_mask(icon, NULL, &iconmask, 255);
	gc = gdk_gc_new(mask);
	gdk_gc_set_foreground(gc, &black);
	gdk_draw_rectangle(mask, gc, TRUE, 0, 0, width, 56);
	gdk_draw_drawable(mask, gc, iconmask, 0, 0, 26, 4, -1, -1);
	gdk_gc_set_foreground(gc, &white);
	gdk_draw_rectangle(mask, gc, TRUE, 0, 56, width, height - 56);
	gtk_widget_shape_combine_mask(desktopicon->window, mask, 0, 0);
	g_object_unref(gc);
	g_object_unref(iconmask);
	g_object_unref(mask);
}

/* desktopicon_new */
/* callbacks */
static gboolean _on_desktopicon_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static gboolean _on_icon_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data);

DesktopIcon * desktopicon_new(Desktop * desktop, char const * name,
		char const * path)
{
	DesktopIcon * desktopicon;
	struct stat st;
	GdkGeometry geometry;
	GtkWidget * vbox;
	GtkWidget * eventbox;
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
	desktopicon->updated = 1;
	desktopicon->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_type_hint(GTK_WINDOW(desktopicon->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_set_resizable(GTK_WINDOW(desktopicon->window), FALSE);
	gtk_window_set_decorated(GTK_WINDOW(desktopicon->window), FALSE);
	gtk_window_set_keep_below(GTK_WINDOW(desktopicon->window), TRUE);
	gtk_window_set_accept_focus(GTK_WINDOW(desktopicon->window), FALSE);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_focus_on_map(GTK_WINDOW(desktopicon->window), FALSE);
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
	gtk_window_set_geometry_hints(GTK_WINDOW(desktopicon->window), vbox,
		&geometry, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE
			| GDK_HINT_BASE_SIZE);
	/* icon */
	if(lstat(path, &st) == 0)
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
	desktopicon->image = gtk_image_new_from_pixbuf(icon);
	gtk_widget_set_size_request(desktopicon->image, DESKTOPICON_MIN_WIDTH,
			DESKTOPICON_ICON_SIZE);
	eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(eventbox), desktopicon->image);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(_on_icon_press), desktopicon);
	gtk_box_pack_start(GTK_BOX(vbox), eventbox, FALSE, TRUE, 4);
	if((p = g_filename_to_utf8(name, -1, NULL, NULL, NULL)) != NULL)
		name = p;
	eventbox = gtk_event_box_new();
	desktopicon->label = gtk_label_new(name);
	gtk_container_add(GTK_CONTAINER(eventbox), desktopicon->label);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(_on_icon_press), desktopicon);
	label = GTK_LABEL(desktopicon->label);
	gtk_label_set_justify(label, GTK_JUSTIFY_CENTER);
#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_label_set_line_wrap_mode(label, PANGO_WRAP_WORD_CHAR);
#endif
	gtk_label_set_line_wrap(label, TRUE);
	gtk_widget_set_size_request(desktopicon->label, DESKTOPICON_MIN_WIDTH,
			-1);
	gtk_box_pack_start(GTK_BOX(vbox), eventbox, TRUE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(desktopicon->window), vbox);
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

static gboolean _on_icon_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	DesktopIcon * desktopicon = data;
	GtkWidget * menu;
	GtkWidget * menuitem;

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
#if GTK_CHECK_VERSION(2, 6, 0)
	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "edit",
			GTK_STOCK_EDIT, G_CALLBACK(_on_icon_edit), desktopicon,
			menu);
#else
	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "edit",
			"_Edit", G_CALLBACK(_on_icon_edit), desktopicon, menu);
#endif
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
			mime_action(desktopicon->desktop->mime, "open",
					desktopicon->path);
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


void desktopicon_set_icon(DesktopIcon * desktopicon, GdkPixbuf * icon)
{
	gtk_image_set_from_pixbuf(GTK_IMAGE(desktopicon->image), icon);
	_desktopicon_update_transparency(desktopicon, icon);
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
