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
/* FIXME
 * - use _NET_WORKAREA to determine where to place the icons
 * - track multiple selection on delete/properties... */



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
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <System.h>
#include "mime.h"
#include "desktop.h"
#include "../config.h"

#ifdef PACKAGE
# undef PACKAGE
#endif
#define PACKAGE	"desktop"

#ifndef PREFIX
# define PREFIX	"/usr/local"
#endif


/* Desktop */
/* private */
/* constants */
#define DESKTOP ".desktop"
#define DESKTOPRC ".desktoprc"


/* types */
struct _Desktop
{
	DesktopLayout layout;
	DesktopIcon ** icon;
	size_t icon_cnt;
	Mime * mime;
	char const * home;
	char * path;
	size_t path_cnt;
	DIR * refresh_dir;
	time_t refresh_mti;
	/* applications */
	GSList * apps;

	GdkWindow * root;
	GdkPixbuf * background;
	GtkIconTheme * theme;
	GdkPixbuf * file;
	GdkPixbuf * folder;
	GtkWidget * menu;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_background;

	gint width;
	gint height;
};


/* prototypes */
static int _desktop_error(Desktop * desktop, char const * message,
		char const * error, int ret);
static int _desktop_serror(Desktop * desktop, char const * message, int ret);
static Config * _desktop_get_config(Desktop * desktop);


/* public */
/* functions */
/* desktop_new */
static Desktop * _new_error(Desktop * desktop, char const * message);
static int _new_create_desktop(Desktop * desktop);
static int _new_create_desktop_applications(Desktop * desktop);
static int _new_create_desktop_files(Desktop * desktop);
static void _new_add_home(Desktop * desktop);

/* callbacks */
static gboolean _new_idle(gpointer data);
static GdkFilterReturn _new_on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Desktop * desktop_new(DesktopLayout layout)
{
	Desktop * desktop;
	const char * file[] = { "gnome-fs-regular",
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_FILE,
#endif
		GTK_STOCK_MISSING_IMAGE, NULL };
	const char * folder[] = { "gnome-fs-directory",
#if GTK_CHECK_VERSION(2, 6, 0)
		GTK_STOCK_DIRECTORY,
#endif
		GTK_STOCK_MISSING_IMAGE, NULL };
	const char ** p;
	gint x;
	gint y;
	gint depth;

	if((desktop = malloc(sizeof(*desktop))) == NULL)
		return NULL;
	desktop->layout = layout;
	desktop->icon = NULL;
	desktop->icon_cnt = 0;
	desktop->path = NULL;
	desktop->apps = NULL;
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
	if((desktop->home = getenv("HOME")) == NULL
			&& (desktop->home = g_get_home_dir()) == NULL)
		desktop->home = "/";
	if(_new_create_desktop(desktop) != 0)
		return _new_error(desktop, "Creating desktop");
	desktop_refresh(desktop);
	/* manage root window events */
	desktop->menu = NULL;
	desktop->root = gdk_screen_get_root_window(
			gdk_display_get_default_screen(
				gdk_display_get_default()));
	desktop->background = NULL;
	gdk_window_get_geometry(desktop->root, &x, &y, &desktop->width,
			&desktop->height, &depth);
	gdk_window_set_events(desktop->root, gdk_window_get_events(
				desktop->root) | GDK_BUTTON_PRESS_MASK);
	gdk_window_add_filter(desktop->root, _new_on_root_event, desktop);
	/* preferences */
	desktop->pr_window = NULL;
	/* draw background when idle */
	g_idle_add(_new_idle, desktop);
	return desktop;
}

static Desktop * _new_error(Desktop * desktop, char const * message)
{
	desktop_error(desktop, message, -1);
	desktop_delete(desktop);
	return NULL;
}

static int _new_create_desktop(Desktop * desktop)
{
	switch(desktop->layout)
	{
		case DL_APPLICATIONS:
			return _new_create_desktop_applications(desktop);
		case DL_FILES:
		default:
			return _new_create_desktop_files(desktop);
	}
}

static int _new_create_desktop_applications(Desktop * desktop)
{
	const char path[] = PREFIX "/share/applications";
	struct stat st;

	if((desktop->path = strdup(path)) == NULL)
		return 1;
	desktop->path_cnt = sizeof(path);
	if(stat(desktop->path, &st) == 0)
		if(!S_ISDIR(st.st_mode))
		{
			errno = ENOTDIR;
			return 1;
		}
	return 0;
}

static int _new_create_desktop_files(Desktop * desktop)
{
	struct stat st;

	desktop->path_cnt = strlen(desktop->home) + strlen("/" DESKTOP) + 1;
	if((desktop->path = malloc(desktop->path_cnt)) == NULL)
		return 1;
	snprintf(desktop->path, desktop->path_cnt, "%s%s", desktop->home,
			"/" DESKTOP);
	if(stat(desktop->path, &st) == 0)
	{
		if(!S_ISDIR(st.st_mode))
		{
			errno = ENOTDIR;
			return 1;
		}
	}
	else if(mkdir(desktop->path, 0777) != 0)
		return 1;
	_new_add_home(desktop);
	return 0;
}

static void _new_add_home(Desktop * desktop)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	if((desktopicon = desktopicon_new(desktop, "Home", desktop->home))
			== NULL)
		return;
	desktopicon_set_first(desktopicon, TRUE);
	desktopicon_set_immutable(desktopicon, TRUE);
	desktop_icon_add(desktop, desktopicon);
	icon = gtk_icon_theme_load_icon(desktop->theme, "gnome-home",
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon == NULL)
		icon = gtk_icon_theme_load_icon(desktop->theme, "gnome-fs-home",
				DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon != NULL)
		desktopicon_set_icon(desktopicon, icon);
}

static gboolean _new_idle(gpointer data)
{
	Desktop * desktop = data;
	Config * config;
	char const * p;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((config = _desktop_get_config(desktop)) == NULL
			|| (p = config_get(config, "", "background")) == NULL)
	{
		if(config != NULL)
			config_delete(config);
		return FALSE;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() background=\"%s\"\n", __func__, p);
#endif
	g_free(desktop->background);
	desktop->background = gdk_pixbuf_new_from_file_at_scale(p,
			desktop->width, desktop->height, FALSE, &error);
	config_delete(config);
	if(desktop->background == NULL)
	{
		desktop_error(desktop, error->message, 0);
		return FALSE;
	}
	gdk_draw_pixbuf(desktop->root, NULL, desktop->background, 0, 0, 0, 0,
			-1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
	gdk_window_set_events(desktop->root, gdk_window_get_events(
				desktop->root) | GDK_BUTTON_PRESS_MASK
			| GDK_EXPOSURE_MASK);
	return FALSE;
}

static GdkFilterReturn _event_button_press(XButtonEvent * xbev,
		Desktop * desktop);
static GdkFilterReturn _event_expose(XExposeEvent * xevent, Desktop * desktop);
static GdkFilterReturn _event_configure(XConfigureEvent * xevent, Desktop * desktop);
static void _on_popup_new_folder(gpointer data);
static void _on_popup_new_text_file(gpointer data);
static void _on_popup_paste(gpointer data);
static void _on_popup_preferences(gpointer data);

static GdkFilterReturn _new_on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Desktop * desktop = data;
	XEvent * xev = xevent;

	if(xev->type == ButtonPress)
		return _event_button_press(xevent, desktop);
	else if(xev->type == Expose)
		return _event_expose(xevent, desktop);
	else if(xev->type == ConfigureNotify)
		return _event_configure(xevent, desktop);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %d\n", __func__, xev->type);
#endif
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _event_button_press(XButtonEvent * xbev,
		Desktop * desktop)
{
	GtkWidget * menuitem;
	GtkWidget * submenu;
	GtkWidget * image;

	if(xbev->button != 3 || desktop->menu != NULL)
	{
		if(desktop->menu != NULL)
		{
			gtk_widget_destroy(desktop->menu);
			desktop->menu = NULL;
		}
		return GDK_FILTER_CONTINUE;
	}
	desktop->menu = gtk_menu_new();
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(desktop->menu), menuitem);
	/* submenu for new documents */
	menuitem = gtk_image_menu_item_new_with_label("Folder");
	image = gtk_image_new_from_icon_name("folder-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_folder), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label("Text file");
	image = gtk_image_new_from_icon_name("stock_new-text",
			GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_text_file), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	/* edition */
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(desktop->menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_paste), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(desktop->menu), menuitem);
	/* preferences */
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(desktop->menu), menuitem);
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES,
			NULL);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_preferences), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(desktop->menu), menuitem);
	gtk_widget_show_all(desktop->menu);
	gtk_menu_popup(GTK_MENU(desktop->menu), NULL, NULL, NULL, NULL, 3,
			xbev->time);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _event_expose(XExposeEvent * xevent, Desktop * desktop)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %d %d, %d %d\n", __func__,
			xevent->x, xevent->y, xevent->width, xevent->height);
#endif
	gdk_draw_pixbuf(desktop->root, NULL,
			desktop->background, xevent->x, xevent->y,
			xevent->x, xevent->y, xevent->width, xevent->height,
			GDK_RGB_DITHER_NORMAL, 0, 0);
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _event_configure(XConfigureEvent * xevent, Desktop * desktop)
{
	desktop->width = xevent->width;
	desktop->height = xevent->height;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() %dx%d\n", __func__, desktop->width,
			desktop->height);
#endif
	g_idle_add(_new_idle, desktop); /* FIXME run it directly? */
	return GDK_FILTER_CONTINUE;
}

static void _on_popup_new_folder(gpointer data)
{
	static char const newfolder[] = "New folder";
	Desktop * desktop = data;
	String * path;

	gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
	if((path = string_new_append(desktop->path, "/", newfolder, NULL))
			== NULL)
	{
		_desktop_serror(desktop, newfolder, 0);
		return;
	}
	if(mkdir(path, 0777) != 0)
		desktop_error(desktop, path, 0);
	string_delete(path);
}

static void _on_popup_new_text_file(gpointer data)
{
	static char const newtext[] = "New text file.txt";
	Desktop * desktop = data;
	String * path;
	int fd;

	gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
	if((path = string_new_append(desktop->path, "/", newtext, NULL))
			== NULL)
	{
		_desktop_serror(desktop, newtext, 0);
		return;
	}
	if((fd = creat(path, 0666)) < 0)
		desktop_error(desktop, path, 0);
	else
		close(fd);
	string_delete(path);
}

static void _on_popup_paste(gpointer data)
{
	Desktop * desktop = data;

	/* FIXME implement */
	gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
}

static void _preferences_set(Desktop * desktop);
static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data);
static void _on_preferences_ok(GtkWidget * widget, gpointer data);
static void _on_preferences_apply(GtkWidget * widget, gpointer data);
static void _on_preferences_cancel(GtkWidget * widget, gpointer data);
static void _on_popup_preferences(gpointer data)
{
	Desktop * desktop = data;
	GtkWidget * hbox;
	GtkWidget * vbox;
	GtkWidget * vbox2;
	GtkWidget * hbox2;
	GtkWidget * widget;
	GtkSizeGroup * group;

	gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
	if(desktop->pr_window != NULL)
	{
		gtk_widget_show(desktop->pr_window);
		return;
	}
	/* window */
	desktop->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(desktop->pr_window),
			"Desktop preferences");
	g_signal_connect(G_OBJECT(desktop->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), desktop);
	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	/* notebook */
	widget = gtk_notebook_new();
	vbox2 = gtk_vbox_new(FALSE, 0);
	desktop->pr_background = gtk_file_chooser_button_new("Background",
			GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_box_pack_start(GTK_BOX(vbox2), desktop->pr_background, FALSE, TRUE,
			4);
	gtk_notebook_append_page(GTK_NOTEBOOK(widget), vbox2, gtk_label_new(
				"Appearance"));
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 4);
	/* dialog */
	hbox2 = gtk_hbox_new(FALSE, 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	widget = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_ok), desktop);
	gtk_box_pack_end(GTK_BOX(hbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_apply), desktop);
	gtk_box_pack_end(GTK_BOX(hbox2), widget, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_size_group_add_widget(group, widget);
	g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(
				_on_preferences_cancel), desktop);
	gtk_box_pack_end(GTK_BOX(hbox2), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, TRUE, 4);
	/* container */
	gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(desktop->pr_window), hbox);
	_preferences_set(desktop);
	gtk_widget_show_all(desktop->pr_window);
}

static gboolean _on_preferences_closex(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	gtk_widget_hide(widget);
	_on_preferences_cancel(NULL, data);
	return TRUE;
}

static void _on_preferences_ok(GtkWidget * widget, gpointer data)
{
	Desktop * desktop = data;

	gtk_widget_hide(desktop->pr_window);
	_on_preferences_apply(widget, data);
}

static void _on_preferences_apply(GtkWidget * widget, gpointer data)
{
	Desktop * desktop = data;
	Config * config;
	char * p;

	if((config = _desktop_get_config(desktop)) == NULL)
		return;
	p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
				desktop->pr_background));
	config_set(config, "", "background", p);
	g_free(p);
	/* XXX code duplication */
	if((p = string_new_append(desktop->home, "/" DESKTOPRC, NULL)) != NULL)
	{
		config_save(config, p);
		object_delete(p);
	}
	config_delete(config);
	/* XXX not very efficient */
	g_idle_add(_new_idle, desktop);
}

static void _on_preferences_cancel(GtkWidget * widget, gpointer data)
{
	Desktop * desktop = data;

	gtk_widget_hide(desktop->pr_window);
	_preferences_set(desktop);
}

static void _preferences_set(Desktop * desktop)
{
	Config * config;
	String const * p;

	if((config = _desktop_get_config(desktop)) == NULL)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
					desktop->pr_background), NULL);
		return;
	}
	p = config_get(config, "", "background");
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
				desktop->pr_background), p);
	config_delete(config);
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


/* accessors */
/* desktop_get_drag_data */
int desktop_get_drag_data(Desktop * desktop, GtkSelectionData * seldata)
{
	int ret = 0;
	size_t i;
	size_t len;
	char const * path;
	unsigned char * p;

	seldata->format = 0;
	seldata->data = NULL;
	seldata->length = 0;
	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktopicon_get_selected(desktop->icon[i]) != TRUE)
			continue;
		if((path = desktopicon_get_path(desktop->icon[i])) == NULL)
			continue;
		len = strlen(path + 1);
		if((p = realloc(seldata->data, seldata->length + len)) == NULL)
		{
			ret |= error_set_code(1, "%s", strerror(errno));
			continue;
		}
		seldata->data = p;
		memcpy(&p[seldata->length], path, len);
		seldata->length += len;
	}
	return ret;
}


/* desktop_get_file */
GdkPixbuf * desktop_get_file(Desktop * desktop)
{
	return desktop->file;
}


/* desktop_get_folder */
GdkPixbuf * desktop_get_folder(Desktop * desktop)
{
	return desktop->folder;
}


/* desktop_get_mime */
Mime * desktop_get_mime(Desktop * desktop)
{
	return desktop->mime;
}


/* desktop_get_theme */
GtkIconTheme * desktop_get_theme(Desktop * desktop)
{
	return desktop->theme;
}


/* useful */
/* desktop_error */
int desktop_error(Desktop * desktop, char const * message, int ret)
{
	return _desktop_error(desktop, message, strerror(errno), ret);
}

static int _error_text(char const * message, int ret)
{
	fputs("desktop: ", stderr);
	perror(message);
	return ret;
}


/* desktop_refresh */
static void _refresh_current(Desktop * desktop);
static int _current_loop(Desktop * desktop);
static int _current_loop_applications(Desktop * desktop);
static int _current_loop_files(Desktop * desktop);
static gboolean _current_idle(gpointer data);
static gboolean _current_done(Desktop * desktop);

static int _loop_lookup(Desktop * desktop, char const * name);

static gboolean _done_timeout(gpointer data);

void desktop_refresh(Desktop * desktop)
{
	int fd;
	struct stat st;

#ifdef __sun__
	if((fd = open(desktop->path, O_RDONLY)) < 0
			|| fstat(fd, &st) != 0
			|| (desktop->refresh_dir = fdopendir(fd)) == NULL)
	{
		desktop_error(NULL, desktop->path, 0);
		if(fd >= 0)
			close(fd);
		return;
	}
#else
	if((desktop->refresh_dir = opendir(desktop->path)) == NULL)
	{
		desktop_error(NULL, desktop->path, 0);
		return;
	}
	fd = dirfd(desktop->refresh_dir);
	if(fstat(fd, &st) != 0)
	{
		desktop_error(NULL, desktop->path, 0);
		closedir(desktop->refresh_dir);
		return;
	}
#endif
	desktop->refresh_mti = st.st_mtime;
	_refresh_current(desktop);
}

static void _refresh_current(Desktop * desktop)
{
	unsigned int i;

	for(i = 0; i < 16 && _current_loop(desktop) == 0; i++);
	if(i == 16)
		g_idle_add(_current_idle, desktop);
	else
		_current_done(desktop);
}

static int _current_loop(Desktop * desktop)
{
	switch(desktop->layout)
	{
		case DL_APPLICATIONS:
			return _current_loop_applications(desktop);
		case DL_FILES:
		default:
			return _current_loop_files(desktop);
	}
}

static int _current_loop_applications(Desktop * desktop)
{
	struct dirent * de;
	size_t len;
	const char ext[] = ".desktop";
	char * name = NULL;
	char * p;
	DesktopIcon * icon;

	while((de = readdir(desktop->refresh_dir)) != NULL)
	{
		if(de->d_name[0] == '.')
			if(de->d_name[1] == '\0' || (de->d_name[1] == '.'
						&& de->d_name[2] == '\0'))
				continue;
		len = strlen(de->d_name);
		if(len < sizeof(ext) || strncmp(&de->d_name[len - sizeof(ext)
					+ 1], ext, sizeof(ext)) != 0)
			continue;
		if((p = realloc(name, desktop->path_cnt + len + 1)) == NULL)
		{
			error_set_print(PACKAGE, 1, "%s: %s", "realloc",
					strerror(errno));
			continue;
		}
		name = p;
		sprintf(name, "%s/%s", desktop->path, de->d_name);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, name);
#endif
		if((icon = desktopicon_new_application(desktop, name)) == NULL)
			continue;
		desktop_icon_add(desktop, icon);
		free(name);
		return 0;
	}
	free(name);
	return 1;
}

static int _current_loop_files(Desktop * desktop)
{
	struct dirent * de;
	String * p;
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
	if((p = string_new_append(desktop->path, "/", de->d_name, NULL))
			== NULL)
		return _desktop_serror(NULL, de->d_name, 1);
	if((desktopicon = desktopicon_new(desktop, de->d_name, p)) != NULL)
		desktop_icon_add(desktop, desktopicon);
	string_delete(p);
	return 0;
}

static int _loop_lookup(Desktop * desktop, char const * name)
{
	size_t i;
	char const * p;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktopicon_get_updated(desktop->icon[i]) == TRUE)
			continue;
		if((p = desktopicon_get_path(desktop->icon[i])) == NULL
				|| (p = strrchr(p, '/')) == NULL)
			continue;
		if(strcmp(name, ++p) != 0)
			continue;
		desktopicon_set_updated(desktop->icon[i], TRUE);
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

static gboolean _current_done(Desktop * desktop)
{
	size_t i = 1;

	while(i < desktop->icon_cnt)
		if(desktopicon_get_updated(desktop->icon[i]) != TRUE)
			desktop_icon_remove(desktop, desktop->icon[i]);
		else
			desktopicon_set_updated(desktop->icon[i++], FALSE);
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
	DesktopIcon ** p;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(desktop->icon[i] != icon)
			continue;
		desktopicon_delete(icon);
		for(desktop->icon_cnt--; i < desktop->icon_cnt; i++)
			desktop->icon[i] = desktop->icon[i + 1];
		if((p = realloc(desktop->icon, sizeof(*p)
						* (desktop->icon_cnt))) != NULL)
			desktop->icon = p;
		desktop_icons_align(desktop);
		break;
	}
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
	gboolean firsta = desktopicon_get_first(icona);
	gboolean firstb = desktopicon_get_first(iconb);

	if(firsta)
		return firstb ? 0 : -1;
	else if(firstb)
		return 1;
	return strcmp(desktopicon_get_name(icona), desktopicon_get_name(iconb));
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


/* private */
/* functions */
/* desktop_error */
static int _error_text(char const * message, int ret);

static int _desktop_error(Desktop * desktop, char const * message,
		char const * error, int ret)
{
	GtkWidget * dialog;

	if(desktop == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(NULL, 0, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s", "Error");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message, error);
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


/* desktop_serror */
static int _desktop_serror(Desktop * desktop, char const * message, int ret)
{
	return _desktop_error(desktop, message, error_get(), ret);
}


/* desktop_get_config */
static Config * _desktop_get_config(Desktop * desktop)
{
	Config * config;
	String * pathname = NULL;

	if((config = config_new()) == NULL
			|| (pathname = string_new_append(desktop->home,
					"/" DESKTOPRC, NULL)) == NULL
			|| config_load(config, pathname) != 0)
	{
		if(config != NULL)
			config_delete(config);
		if(pathname != NULL)
			object_delete(pathname);
		_desktop_serror(desktop, "Could not load preferences", FALSE);
		return NULL;
	}
	return config;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: desktop [-A | -F]\n"
"  -A	Display the applications registered\n"
"  -F	Display contents of the desktop folder [default]\n", stderr);
	return 1;
}


/* main */
static void _main_sigchld(int signum);

int main(int argc, char * argv[])
{
	int o;
	Desktop * desktop;
	DesktopLayout layout = DL_FILES;
	struct sigaction sa;

	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "AF")) != -1)
		switch(o)
		{
			case 'A':
				layout = DL_APPLICATIONS;
				break;
			case 'F':
				layout = DL_FILES;
				break;
			default:
				return _usage();
		}
	if(optind < argc)
		return _usage();
	if((desktop = desktop_new(layout)) == NULL)
	{
		gtk_main();
		return 2;
	}
	sa.sa_handler = _main_sigchld;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		desktop_error(desktop, "sigaction", 0);
	gtk_main();
	desktop_delete(desktop);
	return 0;
}

static void _main_sigchld(int signum)
{
	wait(NULL);
}
