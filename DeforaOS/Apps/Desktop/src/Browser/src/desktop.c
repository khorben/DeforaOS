/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
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
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
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

	GtkIconTheme * theme;
};

struct _DesktopIcon
{
	Desktop * desktop;
	char * path;
	int isdir;
	char const * mimetype;

	GtkWidget * window;
	GtkWidget * label;
};


/* functions */
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
	GtkWidget * image;

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
	desktopicon->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_move(GTK_WINDOW(desktopicon->window), 0, 0);
	gtk_window_set_type_hint(GTK_WINDOW(desktopicon->window),
			GDK_WINDOW_TYPE_HINT_DOCK);
	gtk_window_set_resizable(GTK_WINDOW(desktopicon->window), FALSE);
	gtk_window_set_deletable(GTK_WINDOW(desktopicon->window), FALSE);
	gtk_window_set_decorated(GTK_WINDOW(desktopicon->window), FALSE);
	gtk_window_set_keep_below(GTK_WINDOW(desktopicon->window), TRUE);
	g_signal_connect(G_OBJECT(desktopicon->window), "delete_event",
			G_CALLBACK(_on_desktopicon_closex), desktopicon);
	vbox = gtk_vbox_new(FALSE, 4);
	geometry.min_width = 100;
	geometry.min_height = 56;
	geometry.max_width = 100;
	geometry.max_height = 100;
	geometry.base_width = 100;
	geometry.base_height = 56;
	gtk_window_set_geometry_hints(GTK_WINDOW(desktopicon->window), vbox,
		&geometry, GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE
			| GDK_HINT_BASE_SIZE);
	/* icon */
	if(lstat(path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
		{
			desktopicon->isdir = 1;
			icon = gtk_icon_theme_load_icon(desktop->theme,
					"gnome-fs-directory", 48, 0, NULL);
		}
		else if((desktopicon->mimetype = mime_type(desktop->mime, path))
				!= NULL)
			mime_icons(desktop->mime, desktop->theme,
					desktopicon->mimetype, &icon);
	}
	if(icon == NULL)
		icon = gtk_icon_theme_load_icon(desktop->theme,
				"gnome-fs-regular", 48, 0, NULL);
	image = gtk_image_new_from_pixbuf(icon);
	gtk_widget_set_size_request(image, 100, 48);
	eventbox = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(eventbox), image);
	g_signal_connect(G_OBJECT(eventbox), "button-press-event",
			G_CALLBACK(_on_icon_press), desktopicon);
	gtk_box_pack_start(GTK_BOX(vbox), eventbox, FALSE, TRUE, 4);
	desktopicon->label = gtk_label_new(name);
	gtk_label_set_justify(GTK_LABEL(desktopicon->label),
			GTK_JUSTIFY_CENTER);
	gtk_label_set_line_wrap_mode(GTK_LABEL(desktopicon->label),
			PANGO_WRAP_WORD_CHAR);
	gtk_label_set_line_wrap(GTK_LABEL(desktopicon->label), TRUE);
	gtk_widget_set_size_request(desktopicon->label, 100, -1);
	g_signal_connect(G_OBJECT(desktopicon->label), "button-press-event",
			G_CALLBACK(_on_icon_press), desktopicon);
	gtk_box_pack_start(GTK_BOX(vbox), desktopicon->label, TRUE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(desktopicon->window), vbox);
	gtk_widget_show_all(desktopicon->window);
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
typedef struct _IconCallback
{
	DesktopIcon * desktopicon;
	char * path;
} IconCallback;
static void _popup_directory(GtkWidget * menu, IconCallback * ic);
static void _popup_file(DesktopIcon * desktopicon, GtkWidget * menu,
		IconCallback * ic);
static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, IconCallback * ic,
		GtkWidget * menu);
/* callbacks */
static void _on_icon_delete(GtkWidget * widget, gpointer data);
static void _on_icon_open(GtkWidget * widget, gpointer data);
static void _on_icon_edit(GtkWidget * widget, gpointer data);
static void _on_icon_open_with(GtkWidget * widget, gpointer data);

static gboolean _on_icon_press(GtkWidget * widget, GdkEventButton * event,
		gpointer data)
{
	static IconCallback ic;
	DesktopIcon * desktopicon = data;
	GtkWidget * menu;
	GtkWidget * menuitem;

	if(event->type != GDK_BUTTON_PRESS || event->button != 3)
		return FALSE;
	menu = gtk_menu_new();
	ic.desktopicon = desktopicon;
	ic.path = desktopicon->path;
	if(desktopicon->isdir)
		_popup_directory(menu, &ic);
	else
		_popup_file(desktopicon, menu, &ic);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_delete), &ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(
			GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, event->time);
	return TRUE;
}

static void _popup_directory(GtkWidget * menu, IconCallback * ic)
{
	GtkWidget * menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_file(DesktopIcon * desktopicon, GtkWidget * menu,
		IconCallback * ic)
{
	GtkWidget * menuitem;

	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "open",
			GTK_STOCK_OPEN, G_CALLBACK(_on_icon_open), ic, menu);
	_popup_mime(desktopicon->desktop->mime, desktopicon->mimetype, "edit",
			GTK_STOCK_EDIT, G_CALLBACK(_on_icon_edit), ic, menu);
	menuitem = gtk_menu_item_new_with_mnemonic("Open _with...");
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_icon_open_with), ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _popup_mime(Mime * mime, char const * mimetype, char const * action,
		char const * label, GCallback callback, IconCallback * ic,
		GtkWidget * menu)
{
	GtkWidget * menuitem;

	if(mime_get_handler(mime, mimetype, action) == NULL)
		return;
	if(strncmp(label, "gtk-", 4) == 0)
		menuitem = gtk_image_menu_item_new_from_stock(label, NULL);
	else
		menuitem = gtk_menu_item_new_with_mnemonic(label);
	g_signal_connect(G_OBJECT(menuitem), "activate", callback, ic);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
}

static void _on_icon_delete(GtkWidget * widget, gpointer data)
{
	IconCallback * ic = data;

	/* FIXME actually delete the file, and wait for the refresh */
	desktop_icons_remove(ic->desktopicon->desktop, ic->desktopicon);
}

static void _on_icon_open(GtkWidget * widget, gpointer data)
{
	IconCallback * ic = data;
	pid_t pid;

	if(ic->desktopicon->isdir == 0)
	{
		mime_action(ic->desktopicon->desktop->mime, "open", ic->path);
		return;
	}
	if((pid = fork()) == -1)
	{
		desktop_error(ic->desktopicon->desktop, strerror(errno), 0);
		return;
	}
	if(pid != 0)
		return;
	execlp("browser", "browser", ic->desktopicon->path, NULL);
	fprintf(stderr, "%s%s\n", "desktop: browser: ", strerror(errno));
	exit(2);
}

static void _on_icon_edit(GtkWidget * widget, gpointer data)
{
	IconCallback * ic = data;

	mime_action(ic->desktopicon->desktop->mime, "edit", ic->path);
}

static void _on_icon_open_with(GtkWidget * widget, gpointer data)
{
	IconCallback * ic = data;
	GtkWidget * dialog;
	char * filename = NULL;
	pid_t pid;

	dialog = gtk_file_chooser_dialog_new("Open with...",
			GTK_WINDOW(ic->desktopicon->window),
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
		desktop_error(ic->desktopicon->desktop, "fork", 0);
	else if(pid == 0)
	{
		execlp(filename, filename, ic->path, NULL);
		desktop_error(NULL, filename, 0);
		exit(2);
	}
	g_free(filename);
}


/* desktopicon_delete */
void desktopicon_delete(DesktopIcon * desktopicon)
{
	free(desktopicon->path);
	gtk_widget_destroy(desktopicon->window);
	free(desktopicon);
}


/* desktopicon_move */
void desktopicon_move(DesktopIcon * desktopicon, int x, int y)
{
	gtk_window_move(GTK_WINDOW(desktopicon->window), x, y);
}


/* Desktop */
/* functions */
/* desktop_new */
/* FIXME implement desktop resizing callback */
Desktop * desktop_new(void)
{
	Desktop * desktop;
	char * home;

	if((desktop = malloc(sizeof(*desktop))) == NULL)
		return NULL;
	desktop->icon = NULL;
	desktop->icon_cnt = 0;
	if((desktop->mime = mime_new()) == NULL
			|| (desktop->icon = malloc(sizeof(*(desktop->icon))
					* desktop->icon_cnt)) == NULL)
	{
		desktop_delete(desktop);
		return NULL;
	}
	desktop->theme = gtk_icon_theme_new();
	gtk_icon_theme_set_custom_theme(desktop->theme, "gnome");
	if((home = getenv("HOME")) == NULL)
	{
		desktop_error(desktop, "HOME", 0);
		return desktop;
	}
	desktop_icons_add(desktop, desktopicon_new(desktop, "Home", home));
	desktop_refresh(desktop);
	return desktop;
}
/*	mask = gdk_drawable_get_image(GDK_DRAWABLE(pixbuf), 0, 0, 48, 48); */
/*	gdk_window_shape_combine_mask(GTK_WINDOW(window)->window, mask, 0, 0); */


/* desktop_delete */
void desktop_delete(Desktop * desktop)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
		desktopicon_delete(desktop->icon[i]);
	free(desktop->icon);
	mime_delete(desktop->mime);
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
			GTK_BUTTONS_CLOSE, "%s", message);
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


void desktop_refresh(Desktop * desktop)
{
	char * home;
	size_t off;
	size_t cnt;
	char * path;
	char * p;
	DIR * dir;
	struct dirent * de;
	DesktopIcon * desktopicon;

	if((home = getenv("HOME")) == NULL)
	{
		desktop_error(desktop, "HOME", 0);
		return;
	}
	off = strlen(home) + strlen("/" DESKTOP);
	cnt = off + 1;
	if((path = malloc(cnt)) == NULL)
	{
		desktop_error(desktop, "malloc", 0);
		return;
	}
	sprintf(path, "%s%s", home, "/" DESKTOP);
	if((dir = opendir(path)) == NULL && (mkdir(path, 0700) != 0
				|| (dir = opendir(path)) == NULL))
	{
		desktop_error(desktop, path, 0);
		free(path);
		return;
	}
	while((de = readdir(dir)) != NULL)
	{
		if(de->d_name[0] == '.')
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
		if((p = realloc(path, cnt + strlen(de->d_name) + 1)) == NULL)
		{
			desktop_error(NULL, "realloc", 0);
			continue;
		} /* FIXME avoid calling realloc() at every pass */
		path = p;
		sprintf(&path[off], "/%s", de->d_name);
		if((desktopicon = desktopicon_new(desktop, de->d_name, path))
				!= NULL)
			desktop_icons_add(desktop, desktopicon);
	}
	closedir(dir);
	free(path);
}


/* desktop_icons_align */
void desktop_icons_align(Desktop * desktop)
{
	GdkScreen * screen;
	int height = INT_MAX;
	size_t i;
	int x = 0;
	int y = 0;

	if((screen = gdk_screen_get_default()) != NULL)
		height = gdk_screen_get_height(screen);
	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(y + 100 > height)
		{
			x += 100;
			y = 0;
		}
		desktopicon_move(desktop->icon[i], x, y);
		y += 100;
	}
}


/* desktop_icons_add */
void desktop_icons_add(Desktop * desktop, DesktopIcon * icon)
{
	DesktopIcon ** p;

	if((p = realloc(desktop->icon, sizeof(*p) * (desktop->icon_cnt + 1)))
			== NULL)
	{
		desktop_error(desktop, "Adding an icon failed", 0);
		return;
	}
	desktop->icon = p;
	desktop->icon[desktop->icon_cnt++] = icon;
	desktop_icons_align(desktop);
}


/* desktop_icons_remove */
void desktop_icons_remove(Desktop * desktop, DesktopIcon * icon)
{
	size_t i;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktop->icon[i] != icon)
			continue;
		desktopicon_delete(icon);
		desktop->icon_cnt--;
		for(; i < desktop->icon_cnt; i++)
			desktop->icon[i] = desktop->icon[i+1];
		desktop_icons_align(desktop);
	}
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
	if(optind < argc-1)
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
