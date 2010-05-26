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
#include <locale.h>
#include <libintl.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <System.h>
#include "mime.h"
#include "desktop.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) string

#define COMMON_SYMLINK
#include "common.c"


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* Desktop */
/* private */
/* types */
typedef struct _DesktopCategory DesktopCategory;

struct _Desktop
{
	/* workarea */
	gint x;
	gint y;
	gint width;
	gint height;

	/* icons */
	DesktopIcon ** icon;
	size_t icon_cnt;

	/* layout */
	DesktopLayout layout;
	/* common */
	char * path;
	size_t path_cnt;
	DIR * refresh_dir;
	time_t refresh_mti;
	/* files */
	Mime * mime;
	char const * home;
	GdkPixbuf * file;
	GdkPixbuf * folder;
	/* applications */
	DesktopCategory * category;
	/* categories */
	GSList * apps;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_background;

	/* internal */
	GdkDisplay * display;
	GdkWindow * root;
	GdkPixbuf * background;
	GtkIconTheme * theme;
	GtkWidget * menu;
};

struct _DesktopCategory
{
	gboolean show;
	char const * category;
	char const * name;
	char const * icon;
};


/* constants */
#define DESKTOP ".desktop"
#define DESKTOPRC ".desktoprc"

static DesktopCategory _desktop_categories[] =
{
	{ FALSE, "Audio;",	"Audio",	"gnome-mime-audio",	},
	{ FALSE, "Development;","Development",	"applications-development"},
	{ FALSE, "Education;",	"Education",	"applications-science"	},
	{ FALSE, "Game;",	"Games",	"applications-games"	},
	{ FALSE, "Graphics;",	"Graphics",	"applications-graphics"	},
	{ FALSE, "AudioVideo;",	"Multimedia",	"applications-multimedia"},
	{ FALSE, "Network;",	"Network",	"applications-internet" },
	{ FALSE, "Office;",	"Office",	"applications-office"	},
	{ FALSE, "Settings;",	"Settings",	"gnome-settings"	},
	{ FALSE, "System;",	"System",	"applications-system"	},
	{ FALSE, "Utility;",	"Utilities",	"applications-utilities"},
	{ FALSE, "Video;",	"Video",	"video"			},
	{ FALSE, NULL,		NULL,		NULL,			}
};


/* prototypes */
static int _desktop_error(Desktop * desktop, char const * message,
		char const * error, int ret);
static int _desktop_serror(Desktop * desktop, char const * message, int ret);

static Config * _desktop_get_config(Desktop * desktop);
static int _desktop_get_workarea(Desktop * desktop);


/* public */
/* functions */
/* desktop_new */
/* callbacks */
static gboolean _new_idle(gpointer data);
static GdkFilterReturn _new_on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Desktop * desktop_new(DesktopLayout layout)
{
	Desktop * desktop;
	GdkScreen * screen;
	gint x;
	gint y;
	gint depth;

	if((desktop = malloc(sizeof(*desktop))) == NULL)
		return NULL;
	memset(desktop, 0, sizeof(*desktop));
	/* workarea */
	screen = gdk_screen_get_default();
	desktop->display = gdk_screen_get_display(screen);
	desktop->root = gdk_screen_get_root_window(screen);
	_desktop_get_workarea(desktop);
	/* layout */
	desktop->theme = gtk_icon_theme_get_default();
	if((desktop->home = getenv("HOME")) == NULL
			&& (desktop->home = g_get_home_dir()) == NULL)
		desktop->home = "/";
	desktop_set_layout(desktop, layout);
	/* manage root window events */
	gdk_window_get_geometry(desktop->root, &x, &y, &desktop->width,
			&desktop->height, &depth);
	gdk_window_set_events(desktop->root, gdk_window_get_events(
				desktop->root) | GDK_BUTTON_PRESS_MASK);
	gdk_window_add_filter(desktop->root, _new_on_root_event, desktop);
	/* draw background when idle */
	g_idle_add(_new_idle, desktop);
	return desktop;
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
static GdkFilterReturn _event_configure(XConfigureEvent * xevent,
		Desktop * desktop);
static GdkFilterReturn _event_property(XPropertyEvent * xevent,
		Desktop * desktop);
static void _on_popup_new_folder(gpointer data);
static void _on_popup_new_text_file(gpointer data);
static void _on_popup_paste(gpointer data);
static void _on_popup_preferences(gpointer data);
static void _on_popup_symlink(gpointer data);

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
	else if(xev->type == PropertyNotify)
		return _event_property(xevent, desktop);
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
	menuitem = gtk_image_menu_item_new_with_label(_("Folder"));
	image = gtk_image_new_from_icon_name("folder-new", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_new_folder), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label(_("Symbolic link..."));
	g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(
				_on_popup_symlink), desktop);
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), menuitem);
	menuitem = gtk_image_menu_item_new_with_label(_("Text file"));
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

static GdkFilterReturn _event_configure(XConfigureEvent * xevent,
		Desktop * desktop)
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

static GdkFilterReturn _event_property(XPropertyEvent * xevent,
		Desktop * desktop)
{
	Atom atom;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	atom = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
	if(xevent->atom != atom)
		return GDK_FILTER_CONTINUE;
	_desktop_get_workarea(desktop);
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
	static char const newtext[] = N_("New text file.txt");
	Desktop * desktop = data;
	String * path;
	int fd;

	gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
	if((path = string_new_append(desktop->path, "/", _(newtext), NULL))
			== NULL)
	{
		_desktop_serror(desktop, _(newtext), 0);
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
	GtkWidget * label;
	GtkSizeGroup * group;
	PangoFontDescription * desc;

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
			_("Desktop preferences"));
	g_signal_connect(G_OBJECT(desktop->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), desktop);
	hbox = gtk_hbox_new(FALSE, 0);
	vbox = gtk_vbox_new(FALSE, 0);
	/* notebook */
	desc = pango_font_description_new();
	pango_font_description_set_weight(desc, PANGO_WEIGHT_BOLD);
	widget = gtk_notebook_new();
	vbox2 = gtk_vbox_new(FALSE, 0);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox2 = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Background: "));
	gtk_widget_modify_font(label, desc);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE, TRUE, 0);
	desktop->pr_background = gtk_file_chooser_button_new(_("Background"),
			GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_box_pack_start(GTK_BOX(hbox2), desktop->pr_background, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox2, FALSE, TRUE, 4);
	gtk_notebook_append_page(GTK_NOTEBOOK(widget), vbox2, gtk_label_new(
				_("Appearance")));
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

static void _on_popup_symlink(gpointer data)
{
	Desktop * desktop = data;

	if(_common_symlink(NULL, desktop->path) != 0)
		desktop_error(desktop, "symlink", 0);
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


/* desktop_set_layout */
static void _layout_delete(Desktop * desktop);
static int _layout_applications(Desktop * desktop);
static void _layout_applications_open(Desktop * desktop, gpointer data);
static int _layout_categories(Desktop * desktop);
static int _layout_files(Desktop * desktop);
static void _layout_files_add_home(Desktop * desktop);
static int _layout_homescreen(Desktop * desktop);
static void _layout_homescreen_open(Desktop * desktop, gpointer data);

void desktop_set_layout(Desktop * desktop, DesktopLayout layout)
{
	_layout_delete(desktop);
	desktop->layout = layout;
	switch(desktop->layout)
	{
		case DL_APPLICATIONS:
			_layout_applications(desktop);
			break;
		case DL_CATEGORIES:
			_layout_categories(desktop);
			break;
		case DL_FILES:
			_layout_files(desktop);
			break;
		case DL_HOMESCREEN:
			_layout_homescreen(desktop);
			break;
	}
	desktop_refresh(desktop);
}

static void _layout_delete(Desktop * desktop)
{
	size_t i;

	if(desktop->path != NULL)
		free(desktop->path);
	desktop->path = NULL;
	desktop->path_cnt = 0;
	for(i = 0; i < desktop->icon_cnt; i++)
	{
		desktopicon_set_immutable(desktop->icon[i], FALSE);
		desktopicon_set_updated(desktop->icon[i], FALSE);
	}
	for(i = 0; _desktop_categories[i].name != NULL; i++)
		_desktop_categories[i].show = FALSE;
}

static int _layout_applications(Desktop * desktop)
{
	const char path[] = DATADIR "/applications";
	struct stat st;
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	if((desktop->path = strdup(path)) == NULL)
		return desktop_error(NULL, strerror(errno), 1);
	desktop->path_cnt = sizeof(path);
	if(stat(desktop->path, &st) == 0)
		if(!S_ISDIR(st.st_mode))
			return desktop_error(NULL, strerror(ENOTDIR), 1);
	if(desktop->category != NULL)
	{
		desktopicon = desktopicon_new(desktop, _("Back"), NULL);
		desktopicon_set_callback(desktopicon, _layout_applications_open,
				NULL);
		icon = gtk_icon_theme_load_icon(desktop->theme, "back",
				DESKTOPICON_ICON_SIZE, 0, NULL);
		if(icon != NULL)
			desktopicon_set_icon(desktopicon, icon);
		desktop_icon_add(desktop, desktopicon);
	}
	return 0;
}

static void _layout_applications_open(Desktop * desktop, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	desktop_set_layout(desktop, DL_CATEGORIES);
}

static int _layout_categories(Desktop * desktop)
{
	desktop->category = NULL;
	return _layout_applications(desktop);
}

static int _layout_files(Desktop * desktop)
{
	const char path[] = "/" DESKTOP;
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
	char const ** p;
	struct stat st;

	if(desktop->mime == NULL)
		desktop->mime = mime_new();
	if(desktop->file == NULL)
		for(p = file; *p != NULL && desktop->file == NULL; p++)
			desktop->file = gtk_icon_theme_load_icon(desktop->theme,
					*p, DESKTOPICON_ICON_SIZE, 0, NULL);
	if(desktop->folder == NULL)
		for(p = folder; *p != NULL && desktop->folder == NULL; p++)
			desktop->folder = gtk_icon_theme_load_icon(
					desktop->theme, *p,
					DESKTOPICON_ICON_SIZE, 0, NULL);
	_layout_files_add_home(desktop);
	desktop->path_cnt = strlen(desktop->home) + 1 + sizeof(path);
	if((desktop->path = malloc(desktop->path_cnt)) == NULL)
		return desktop_error(NULL, strerror(ENOTDIR), 1);
	snprintf(desktop->path, desktop->path_cnt, "%s/%s", desktop->home,
			path);
	if(stat(desktop->path, &st) == 0)
	{
		if(!S_ISDIR(st.st_mode))
			return desktop_error(NULL, strerror(ENOTDIR), 1);
	}
	else if(errno != ENOENT || mkdir(desktop->path, 0777) != 0)
		return desktop_error(NULL, strerror(errno), 1);
	return 0;
}

static void _layout_files_add_home(Desktop * desktop)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	if((desktopicon = desktopicon_new(desktop, _("Home"), desktop->home))
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

static int _layout_homescreen(Desktop * desktop)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	if((desktopicon = desktopicon_new(desktop, _("Applications"), NULL))
			== NULL)
		return desktop_error(NULL, error_get(), 1);
	desktopicon_set_callback(desktopicon, _layout_homescreen_open, NULL);
	desktopicon_set_immutable(desktopicon, TRUE);
	desktop_icon_add(desktop, desktopicon);
	icon = gtk_icon_theme_load_icon(desktop->theme, "gnome-applications",
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon != NULL)
		desktopicon_set_icon(desktopicon, icon);
	return 0;
}

static void _layout_homescreen_open(Desktop * desktop, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	desktop_set_layout(desktop, DL_CATEGORIES);
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
static gint _categories_apps_compare(gconstpointer a, gconstpointer b);
static int _current_loop_categories(Desktop * desktop);
static int _current_loop_files(Desktop * desktop);
static gboolean _current_idle(gpointer data);
static gboolean _current_done(Desktop * desktop);
static void _done_categories(Desktop * desktop);
static void _done_categories_open(Desktop * desktop, gpointer data);

static int _loop_lookup(Desktop * desktop, char const * name);

static gboolean _done_timeout(gpointer data);

void desktop_refresh(Desktop * desktop)
{
	int fd;
	struct stat st;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
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
		case DL_CATEGORIES:
			return _current_loop_categories(desktop);
		case DL_FILES:
			return _current_loop_files(desktop);
		case DL_HOMESCREEN:
			break; /* nothing to do */
	}
	return 1;
}

static int _current_loop_applications(Desktop * desktop)
{
	struct dirent * de;
	size_t len;
	const char ext[] = ".desktop";
	char * path = NULL;
	char * p;
	Config * config;
	char const * q;
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
		if((p = realloc(path, desktop->path_cnt + len + 1)) == NULL)
		{
			error_set_print("desktop", 1, "%s: %s", "realloc",
					strerror(errno));
			continue;
		}
		path = p;
		snprintf(path, desktop->path_cnt + len + 1, "%s/%s",
				desktop->path, de->d_name);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, path);
#endif
		/* XXX not very elegant */
		if(desktop->category != NULL && (config = config_new()) != NULL)
		{
			if(config_load(config, path) != 0
					|| (q = config_get(config,
							"Desktop Entry",
							"Categories")) == NULL
					|| string_find(q,
						desktop->category->name)
					== NULL)
			{
				config_delete(config);
				continue;
			}
			config_delete(config);
		}
		if((icon = desktopicon_new_application(desktop, path)) == NULL)
			continue;
		desktop_icon_add(desktop, icon);
		free(path);
		return 0;
	}
	free(path);
	return 1;
}

static int _current_loop_categories(Desktop * desktop)
{
	struct dirent * de;
	size_t len;
	const char ext[] = ".desktop";
	const char section[] = "Desktop Entry";
	char * path = NULL;
	char * p;
	Config * config = NULL;
	char const * q;
	char const * r;

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
		if((p = realloc(path, desktop->path_cnt + len + 1)) == NULL)
		{
			error_set_print("desktop", 1, "%s: %s", "realloc",
					strerror(errno));
			continue;
		}
		path = p;
		snprintf(path, desktop->path_cnt + len + 1, "%s/%s",
				desktop->path, de->d_name);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, path);
#endif
		if(config == NULL && (config = config_new()) == NULL)
			continue; /* XXX report error */
		config_reset(config);
		if(config_load(config, path) != 0)
		{
			error_set_print("desktop", 1, "%s: %s", path,
					error_get());
			continue;
		}
		q = config_get(config, section, "Name");
		r = config_get(config, section, "Exec");
		if(q == NULL || r == NULL)
			continue;
		config_set(config, "", "path", path);
		desktop->apps = g_slist_insert_sorted(desktop->apps, config,
				_categories_apps_compare);
		free(path);
		config = NULL;
		return 0;
	}
	free(path);
	return 1;
}

static gint _categories_apps_compare(gconstpointer a, gconstpointer b)
{
	Config * ca = (Config *)a;
	Config * cb = (Config *)b;
	char const * cap;
	char const * cbp;
	const char section[] = "Desktop Entry";
	const char variable[] = "Name";

	/* these should not fail */
	cap = config_get(ca, section, variable);
	cbp = config_get(cb, section, variable);
	return string_compare(cap, cbp);
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
	size_t i = 0;

	switch(desktop->layout)
	{
		case DL_CATEGORIES:
			_done_categories(desktop);
			break;
		default:
			break;
	}
	while(i < desktop->icon_cnt)
		if(desktopicon_get_immutable(desktop->icon[i]) == TRUE)
			i++;
		else if(desktopicon_get_updated(desktop->icon[i]) != TRUE)
			desktop_icon_remove(desktop, desktop->icon[i]);
		else
			desktopicon_set_updated(desktop->icon[i++], FALSE);
	closedir(desktop->refresh_dir);
	g_timeout_add(1000, _done_timeout, desktop);
	return FALSE;
}

static void _done_categories(Desktop * desktop)
{
	GSList * p;
	Config * config;
	const char section[] = "Desktop Entry";
	char const * q;
	size_t i;
	DesktopCategory * dc;
	char const * path;
	DesktopIcon * icon;

	for(p = desktop->apps; p != NULL; p = p->next)
	{
		config = p->data;
		path = config_get(config, NULL, "path");
		if((q = config_get(config, section, "Categories")) == NULL)
		{
			icon = desktopicon_new_application(desktop, path);
			desktop_icon_add(desktop, icon);
			continue;
		}
		for(i = 0; (dc = &_desktop_categories[i]) != NULL &&
				dc->category != NULL
				&& string_find(q, dc->category) == NULL; i++);
		if(dc->category == NULL)
		{
			icon = desktopicon_new_application(desktop, path);
			desktop_icon_add(desktop, icon);
			continue;
		}
		if(dc->show != FALSE)
			continue;
		dc->show = TRUE;
		icon = desktopicon_new_category(desktop, dc->name, dc->icon);
		desktopicon_set_callback(icon, _done_categories_open, dc);
		desktop_icon_add(desktop, icon);
	}
}

static void _done_categories_open(Desktop * desktop, gpointer data)
{
	DesktopCategory * dc = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, dc->name);
#endif
	desktop->category = dc;
	desktop_set_layout(desktop, DL_APPLICATIONS);
}

static gboolean _done_timeout(gpointer data)
{
	Desktop * desktop = data;
	struct stat st;

	if(desktop->path == NULL)
		return FALSE;
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
						* (desktop->icon_cnt))) != NULL
				|| desktop->icon_cnt == 0)
			desktop->icon = p;
		desktop_icons_align(desktop);
		break;
	}
}


/* desktop_icons_align */
static int _align_compare(const void * a, const void * b);

void desktop_icons_align(Desktop * desktop)
{
	size_t i;
	int x = desktop->x;
	int y = desktop->y;

	qsort(desktop->icon, desktop->icon_cnt, sizeof(void*), _align_compare);
	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(y + DESKTOPICON_MAX_HEIGHT > desktop->height)
		{
			x += DESKTOPICON_MAX_WIDTH;
			y = desktop->y;
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
			GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message,
#endif
			error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
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


/* desktop_get_workarea */
static int _desktop_get_workarea(Desktop * desktop)
{
	Atom atom;
	Atom type;
	int format;
	unsigned long cnt;
	unsigned long bytes;
	unsigned char * p;
	unsigned long * u;

	atom = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
	if(XGetWindowProperty(GDK_DISPLAY_XDISPLAY(desktop->display),
				GDK_WINDOW_XWINDOW(desktop->root), atom, 0,
				G_MAXLONG, False, XA_CARDINAL, &type, &format,
				&cnt, &bytes, &p) != Success)
		return 1;
	if(cnt >= 4)
	{
		u = (unsigned long *)p;
		desktop->x = u[0];
		desktop->y = u[1];
		desktop->width = u[2];
		desktop->height = u[3];
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() (%d, %d) %dx%d\n", __func__,
				desktop->x, desktop->y, desktop->width,
				desktop->height);
#endif
	}
	XFree(p);
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: desktop [-H | -V][-a | -c | -f | -h]\n"
"  -H	Place icons horizontally\n"
"  -V	Place icons vertically\n"
"  -a	Display the applications registered\n"
"  -c	Sort the applications registered by category\n"
"  -f	Display contents of the desktop folder (default)\n"
"  -h	Display the homescreen\n"), stderr);
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

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "HVacfh")) != -1)
		switch(o)
		{
			case 'H':
			case 'V':
				/* FIXME implement */
				break;
			case 'a':
				layout = DL_APPLICATIONS;
				break;
			case 'c':
				layout = DL_CATEGORIES;
				break;
			case 'f':
				layout = DL_FILES;
				break;
			case 'h':
				layout = DL_HOMESCREEN;
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
