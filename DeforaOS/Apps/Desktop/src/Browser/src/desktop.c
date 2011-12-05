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
/* TODO:
 * - let the user define the desktop folder (possibly default to FDO's)
 * - set the font for the icons instantly
 * - track multiple selection on delete/properties */



#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <System.h>
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
	DesktopPrefs prefs;
	PangoFontDescription * font;

	/* workarea */
	GdkRectangle window;
	GdkRectangle workarea;

	/* icons */
	DesktopIcon ** icon;
	size_t icon_cnt;

	/* common */
	char * path;
	size_t path_cnt;
	DIR * refresh_dir;
	time_t refresh_mti;
	guint refresh_source;
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
	GtkWidget * pr_font;
	GtkWidget * pr_color;
	GtkWidget * pr_background;
	GtkWidget * pr_background_how;
	GtkWidget * pr_background_extend;
	GtkWidget * pr_monitors;
	GtkWidget * pr_monitors_res;
	GtkWidget * pr_monitors_size;

	/* internal */
	GdkScreen * screen;
	GdkDisplay * display;
	GdkWindow * root;
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

typedef enum _DesktopHows
{
	DESKTOP_HOW_CENTERED = 0,
	DESKTOP_HOW_SCALED,
	DESKTOP_HOW_SCALED_RATIO,
	DESKTOP_HOW_TILED
} DesktopHows;
#define DESKTOP_HOW_LAST	DESKTOP_HOW_TILED
#define DESKTOP_HOW_COUNT	(DESKTOP_HOW_LAST + 1)


/* constants */
#define DESKTOP ".desktop"
#define DESKTOPRC ".desktoprc"

static const char * _desktop_hows[DESKTOP_HOW_COUNT] =
{
	"centered",
	"scaled",
	"scaled_ratio",
	"tiled"
};

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
	{ FALSE, "Video;",	"Video",	"video"			}
};
static const size_t _desktop_categories_cnt = sizeof(_desktop_categories)
	/ sizeof(*_desktop_categories);


/* prototypes */
static int _desktop_error(Desktop * desktop, char const * message,
		char const * error, int ret);
static int _desktop_serror(Desktop * desktop, char const * message, int ret);

/* accessors */
static Config * _desktop_get_config(Desktop * desktop);
static int _desktop_get_workarea(Desktop * desktop);

/* useful */
static void _desktop_draw_background(Desktop * desktop, GdkColor * color,
		char const * filename, DesktopHows how, gboolean extend);

static int _desktop_icon_add(Desktop * desktop, DesktopIcon * icon);
static int _desktop_icon_remove(Desktop * desktop, DesktopIcon * icon);

static void _desktop_show_preferences(Desktop * desktop);


/* public */
/* functions */
/* desktop_new */
/* callbacks */
static gboolean _new_idle(gpointer data);
static void _idle_background(Desktop * desktop, Config * config);
static GdkFilterReturn _on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data);

Desktop * desktop_new(DesktopPrefs * prefs)
{
	Desktop * desktop;
	gint depth;

	if((desktop = object_new(sizeof(*desktop))) == NULL)
		return NULL;
	memset(desktop, 0, sizeof(*desktop));
	desktop->prefs.alignment = DESKTOP_ALIGNMENT_VERTICAL;
	desktop->prefs.layout = DESKTOP_LAYOUT_FILES;
	desktop->prefs.monitor = -1;
	if(prefs != NULL)
		memcpy(&desktop->prefs, prefs, sizeof(*prefs));
	desktop->font = NULL;
	/* workarea */
	desktop->screen = gdk_screen_get_default();
	desktop->display = gdk_screen_get_display(desktop->screen);
	desktop->root = gdk_screen_get_root_window(desktop->screen);
	desktop->theme = gtk_icon_theme_get_default();
	desktop->menu = NULL;
	if((desktop->home = getenv("HOME")) == NULL
			&& (desktop->home = g_get_home_dir()) == NULL)
		desktop->home = "/";
	/* manage root window events */
	gdk_add_client_message_filter(gdk_atom_intern(DESKTOP_CLIENT_MESSAGE,
				FALSE), _on_root_event, desktop);
	gdk_window_get_geometry(desktop->root, &desktop->window.x,
			&desktop->window.y, &desktop->window.width,
			&desktop->window.height, &depth);
	gdk_window_set_events(desktop->root, gdk_window_get_events(
				desktop->root) | GDK_BUTTON_PRESS_MASK
			| GDK_PROPERTY_CHANGE_MASK);
	gdk_window_add_filter(desktop->root, _on_root_event, desktop);
	/* draw the icons and background when idle */
	g_idle_add(_new_idle, desktop);
	return desktop;
}

static gboolean _new_idle(gpointer data)
{
	Desktop * desktop = data;
	Config * config;
	char const * p;
	size_t i;
	char * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((config = _desktop_get_config(desktop)) == NULL)
		return FALSE;
	_idle_background(desktop, config);
	if((p = config_get(config, NULL, "font")) != NULL)
	{
		desktop->font = pango_font_description_from_string(p);
		for(i = 0; i < desktop->icon_cnt; i++)
			desktopicon_set_font(desktop->icon[i], desktop->font);
	}
	if(desktop->prefs.monitor < 0 && (p = config_get(config, NULL,
					"monitor")) != NULL)
	{
		desktop->prefs.monitor = strtol(p, &q, 10);
		if(p[0] == '\0' || *q != '\0')
			desktop->prefs.monitor = -1;
	}
	config_delete(config);
	_desktop_get_workarea(desktop);
	desktop_set_layout(desktop, desktop->prefs.layout);
	return FALSE;
}

static void _idle_background(Desktop * desktop, Config * config)
{
	GdkColor color = { 0, 0, 0, 0 };
	char const * filename;
	DesktopHows how = DESKTOP_HOW_SCALED;
	gboolean extend = FALSE;
	size_t i;
	char const * p;

	if((p = config_get(config, NULL, "background_color")) != NULL)
		gdk_color_parse(p, &color);
	filename = config_get(config, NULL, "background");
	if((p = config_get(config, NULL, "background_how")) != NULL)
		for(i = 0; i < DESKTOP_HOW_COUNT; i++)
			if(strcmp(_desktop_hows[i], p) == 0)
				how = i;
	if((p = config_get(config, NULL, "background_extend")) != NULL)
		extend = strtol(p, NULL, 10) ? TRUE : FALSE;
	_desktop_draw_background(desktop, &color, filename, how, extend);
}

static GdkFilterReturn _event_button_press(XButtonEvent * xbev,
		Desktop * desktop);
static GdkFilterReturn _event_client_message(XClientMessageEvent * xevent,
		Desktop * desktop);
static GdkFilterReturn _event_configure(XConfigureEvent * xevent,
		Desktop * desktop);
static GdkFilterReturn _event_property(XPropertyEvent * xevent,
		Desktop * desktop);
static void _on_popup_new_folder(gpointer data);
static void _on_popup_new_text_file(gpointer data);
static void _on_popup_paste(gpointer data);
static void _on_popup_preferences(gpointer data);
static void _on_popup_symlink(gpointer data);

static GdkFilterReturn _on_root_event(GdkXEvent * xevent, GdkEvent * event,
		gpointer data)
{
	Desktop * desktop = data;
	XEvent * xev = xevent;

	if(xev->type == ButtonPress)
		return _event_button_press(xevent, desktop);
	else if(xev->type == ClientMessage)
		return _event_client_message(xevent, desktop);
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
	/* ignore if not managing files */
	if(desktop->prefs.layout != DESKTOP_LAYOUT_FILES)
		return GDK_FILTER_CONTINUE;
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

static GdkFilterReturn _event_client_message(XClientMessageEvent * xevent,
		Desktop * desktop)
{
	DesktopMessage message;
	DesktopAlignment alignment;
	DesktopLayout layout;

	if(xevent->message_type != gdk_x11_get_xatom_by_name(
				DESKTOP_CLIENT_MESSAGE))
		return GDK_FILTER_CONTINUE;
	message = xevent->data.b[0];
	switch(message)
	{
		case DESKTOP_MESSAGE_SET_ALIGNMENT:
			alignment = xevent->data.b[1];
			desktop_set_alignment(desktop, alignment);
			break;
		case DESKTOP_MESSAGE_SET_LAYOUT:
			layout = xevent->data.b[1];
			desktop_set_layout(desktop, layout);
			break;
		case DESKTOP_MESSAGE_SHOW:
			if(xevent->data.b[1] == DESKTOP_SHOW_SETTINGS)
				_on_popup_preferences(desktop); /* XXX */
			break;
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _event_configure(XConfigureEvent * xevent,
		Desktop * desktop)
{
	desktop->window.x = xevent->x;
	desktop->window.y = xevent->y;
	desktop->window.width = xevent->width;
	desktop->window.height = xevent->height;
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%dx%d) @ (%d,%d))\n", __func__,
			desktop->window.width, desktop->window.height,
			desktop->window.x, desktop->window.y);
#endif
	g_idle_add(_new_idle, desktop); /* FIXME run it directly? */
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn _event_property(XPropertyEvent * xevent,
		Desktop * desktop)
{
	Atom atom;

	atom = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
	if(xevent->atom != atom)
		return GDK_FILTER_CONTINUE;
	_desktop_get_workarea(desktop);
	return GDK_FILTER_CONTINUE;
}

static void _on_popup_new_folder(gpointer data)
{
	static char const newfolder[] = N_("New folder");
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

static void _on_popup_preferences(gpointer data)
{
	Desktop * desktop = data;

	_desktop_show_preferences(desktop);
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

	if(desktop->refresh_source != 0)
		g_source_remove(desktop->refresh_source);
	for(i = 0; i < desktop->icon_cnt; i++)
		desktopicon_delete(desktop->icon[i]);
	free(desktop->icon);
	if(desktop->mime != NULL)
		mime_delete(desktop->mime);
	free(desktop->path);
	if(desktop->font != NULL)
		pango_font_description_free(desktop->font);
	object_delete(desktop);
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


/* desktop_set_alignment */
static void _alignment_horizontal(Desktop * desktop);
static void _alignment_vertical(Desktop * desktop);

void desktop_set_alignment(Desktop * desktop, DesktopAlignment alignment)
{
	switch(alignment)
	{
		case DESKTOP_ALIGNMENT_VERTICAL:
			_alignment_vertical(desktop);
			break;
		case DESKTOP_ALIGNMENT_HORIZONTAL:
			_alignment_horizontal(desktop);
			break;
	}
}

static void _alignment_horizontal(Desktop * desktop)
{
	size_t i;
	int x = desktop->workarea.x;
	int y = desktop->workarea.y;
	int width = x + desktop->workarea.width;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(x + DESKTOPICON_MAX_WIDTH > width)
		{
			y += DESKTOPICON_MAX_HEIGHT;
			x = desktop->workarea.x;
		}
		desktopicon_move(desktop->icon[i], x, y);
		x += DESKTOPICON_MAX_WIDTH;
	}
}

static void _alignment_vertical(Desktop * desktop)
{
	size_t i;
	int x = desktop->workarea.x;
	int y = desktop->workarea.y;
	int height = desktop->workarea.y + desktop->workarea.height;

	for(i = 0; i < desktop->icon_cnt; i++)
	{
		if(y + DESKTOPICON_MAX_HEIGHT > height)
		{
			x += DESKTOPICON_MAX_WIDTH;
			y = desktop->workarea.y;
		}
		desktopicon_move(desktop->icon[i], x, y);
		y += DESKTOPICON_MAX_HEIGHT;
	}
}


/* desktop_set_layout */
static void _layout_delete(Desktop * desktop);
static int _layout_applications(Desktop * desktop);
static int _layout_categories(Desktop * desktop);
static int _layout_files(Desktop * desktop);
static void _layout_files_add_home(Desktop * desktop);
static int _layout_homescreen(Desktop * desktop);
static void _layout_set_categories(Desktop * desktop, gpointer data);
static void _layout_set_homescreen(Desktop * desktop, gpointer data);

void desktop_set_layout(Desktop * desktop, DesktopLayout layout)
{
	_layout_delete(desktop);
	desktop->prefs.layout = layout;
	switch(layout)
	{
		case DESKTOP_LAYOUT_APPLICATIONS:
			_layout_applications(desktop);
			break;
		case DESKTOP_LAYOUT_CATEGORIES:
			_layout_categories(desktop);
			break;
		case DESKTOP_LAYOUT_FILES:
			_layout_files(desktop);
			break;
		case DESKTOP_LAYOUT_HOMESCREEN:
			_layout_homescreen(desktop);
			break;
		case DESKTOP_LAYOUT_NONE:
			/* nothing to do */
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
	for(i = 0; i < _desktop_categories_cnt; i++)
		_desktop_categories[i].show = FALSE;
}

static int _layout_applications(Desktop * desktop)
{
	const char path[] = DATADIR "/applications";
	struct stat st;
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	free(desktop->path);
	if((desktop->path = strdup(path)) == NULL)
		return desktop_error(NULL, strerror(errno), 1);
	desktop->path_cnt = sizeof(path);
	if(stat(desktop->path, &st) == 0)
		if(!S_ISDIR(st.st_mode))
			return desktop_error(NULL, strerror(ENOTDIR), 1);
	if(desktop->category != NULL)
	{
		desktopicon = desktopicon_new(desktop, _("Back"), NULL);
		desktopicon_set_callback(desktopicon, _layout_set_categories,
				NULL);
		desktopicon_set_font(desktopicon, desktop->font);
		desktopicon_set_immutable(desktopicon, TRUE);
		icon = gtk_icon_theme_load_icon(desktop->theme, "back",
				DESKTOPICON_ICON_SIZE, 0, NULL);
		if(icon != NULL)
			desktopicon_set_icon(desktopicon, icon);
		_desktop_icon_add(desktop, desktopicon);
	}
	return 0;
}

static int _layout_categories(Desktop * desktop)
{
	DesktopIcon * desktopicon;
	GdkPixbuf * icon;

	desktop->category = NULL;
	_layout_applications(desktop); /* XXX hack */
	desktopicon = desktopicon_new(desktop, _("Back"), NULL);
	desktopicon_set_callback(desktopicon, _layout_set_homescreen, NULL);
	desktopicon_set_first(desktopicon, TRUE);
	desktopicon_set_font(desktopicon, desktop->font);
	desktopicon_set_immutable(desktopicon, TRUE);
	icon = gtk_icon_theme_load_icon(desktop->theme, "back",
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon != NULL)
		desktopicon_set_icon(desktopicon, icon);
	_desktop_icon_add(desktop, desktopicon);
	return 0;
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
		desktop->mime = mime_new(NULL);
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
	desktopicon_set_font(desktopicon, desktop->font);
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
#ifdef EMBEDDED
	char const * paths[] =
	{
		DATADIR "/applications/phone-contacts.desktop",
		DATADIR "/applications/phone-dialer.desktop",
		DATADIR "/applications/phone-messages.desktop",
		NULL
	};
	char const ** p;
#endif

	if((desktopicon = desktopicon_new(desktop, _("Applications"), NULL))
			== NULL)
		return desktop_error(NULL, error_get(), 1);
	desktopicon_set_callback(desktopicon, _layout_set_categories, NULL);
	desktopicon_set_font(desktopicon, desktop->font);
	desktopicon_set_immutable(desktopicon, TRUE);
	icon = gtk_icon_theme_load_icon(desktop->theme, "gnome-applications",
			DESKTOPICON_ICON_SIZE, 0, NULL);
	if(icon != NULL)
		desktopicon_set_icon(desktopicon, icon);
	_desktop_icon_add(desktop, desktopicon);
#ifdef EMBEDDED
	for(p = paths; *p != NULL; p++)
		if(access(*p, R_OK) == 0
				&& (desktopicon = desktopicon_new_application(
						desktop, *p)) != NULL)
		{
			desktopicon_set_font(desktopicon, desktop->font);
			_desktop_icon_add(desktop, desktopicon);
		}
#endif
	return 0;
}

static void _layout_set_categories(Desktop * desktop, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	desktop_set_layout(desktop, DESKTOP_LAYOUT_CATEGORIES);
}

static void _layout_set_homescreen(Desktop * desktop, gpointer data)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	desktop_set_layout(desktop, DESKTOP_LAYOUT_HOMESCREEN);
}


/* useful */
/* desktop_error */
int desktop_error(Desktop * desktop, char const * message, int ret)
{
	return _desktop_error(desktop, message, strerror(errno), ret);
}


/* desktop_refresh */
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
	if(desktop->refresh_source != 0)
		g_source_remove(desktop->refresh_source);
	if(desktop->path == NULL)
	{
		desktop->refresh_source = g_idle_add(_current_idle, desktop);
		return;
	}
	desktop->refresh_source = 0;
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
	desktop->refresh_source = g_idle_add(_current_idle, desktop);
}

static int _current_loop(Desktop * desktop)
{
	switch(desktop->prefs.layout)
	{
		case DESKTOP_LAYOUT_APPLICATIONS:
			return _current_loop_applications(desktop);
		case DESKTOP_LAYOUT_CATEGORIES:
			return _current_loop_categories(desktop);
		case DESKTOP_LAYOUT_FILES:
			return _current_loop_files(desktop);
		case DESKTOP_LAYOUT_HOMESCREEN:
		case DESKTOP_LAYOUT_NONE:
			break; /* nothing to do */
	}
	return -1;
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

	if(desktop->category == NULL)
		return -1;
	if((config = config_new()) == NULL)
		return -1;
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
		config_reset(config);
		if(config_load(config, path) != 0)
			continue;
		if((q = config_get(config, "Desktop Entry", "Categories"))
				== NULL)
			continue;
		if(string_find(q, desktop->category->category) == NULL)
			continue;
		if((icon = desktopicon_new_application(desktop, path)) == NULL)
			continue;
		desktopicon_set_font(icon, desktop->font);
		_desktop_icon_add(desktop, icon);
		free(path);
		config_delete(config);
		return 0;
	}
	free(path);
	config_delete(config);
	return -1;
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
		config_set(config, NULL, "path", path);
		desktop->apps = g_slist_insert_sorted(desktop->apps, config,
				_categories_apps_compare);
		free(path);
		config = NULL;
		return 0;
	}
	free(path);
	return -1;
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
		return -1;
	if((p = string_new_append(desktop->path, "/", de->d_name, NULL))
			== NULL)
		return -_desktop_serror(NULL, de->d_name, 1);
	if((desktopicon = desktopicon_new(desktop, de->d_name, p)) != NULL)
	{
		desktop_icon_add(desktop, desktopicon);
		desktopicon_set_font(desktopicon, desktop->font);
	}
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

	switch(desktop->prefs.layout)
	{
		case DESKTOP_LAYOUT_CATEGORIES:
			_done_categories(desktop);
			break;
		default:
			break;
	}
	while(i < desktop->icon_cnt)
		if(desktopicon_get_immutable(desktop->icon[i]) == TRUE)
			i++;
		else if(desktopicon_get_updated(desktop->icon[i]) != TRUE)
			_desktop_icon_remove(desktop, desktop->icon[i]);
		else
			desktopicon_set_updated(desktop->icon[i++], FALSE);
	if(desktop->refresh_dir != NULL)
		closedir(desktop->refresh_dir);
	desktop->refresh_dir = NULL;
	desktop_icons_align(desktop);
	desktop->refresh_source = g_timeout_add(1000, _done_timeout, desktop);
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
			desktopicon_set_font(icon, desktop->font);
			_desktop_icon_add(desktop, icon);
			continue;
		}
		for(i = 0; i < _desktop_categories_cnt
				&& (dc = &_desktop_categories[i]) != NULL
				&& dc->category != NULL
				&& string_find(q, dc->category) == NULL; i++);
		if(dc->category == NULL)
		{
			icon = desktopicon_new_application(desktop, path);
			desktopicon_set_font(icon, desktop->font);
			_desktop_icon_add(desktop, icon);
			continue;
		}
		if(dc->show == TRUE)
			continue;
		dc->show = TRUE;
		icon = desktopicon_new_category(desktop, dc->name, dc->icon);
		desktopicon_set_font(icon, desktop->font);
		desktopicon_set_callback(icon, _done_categories_open, dc);
		_desktop_icon_add(desktop, icon);
	}
}

static void _done_categories_open(Desktop * desktop, gpointer data)
{
	DesktopCategory * dc = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() \"%s\"\n", __func__, dc->name);
#endif
	desktop->category = dc;
	desktop_set_layout(desktop, DESKTOP_LAYOUT_APPLICATIONS);
}

static gboolean _done_timeout(gpointer data)
{
	Desktop * desktop = data;
	struct stat st;

	desktop->refresh_source = 0;
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
	if(_desktop_icon_add(desktop, icon) == 0)
		desktop_icons_align(desktop);
}


/* desktop_icon_remove */
void desktop_icon_remove(Desktop * desktop, DesktopIcon * icon)
{
	if(_desktop_icon_remove(desktop, icon) == 0)
		desktop_icons_align(desktop);
}


/* desktop_icons_align */
static int _align_compare(const void * a, const void * b);

void desktop_icons_align(Desktop * desktop)
{
	qsort(desktop->icon, desktop->icon_cnt, sizeof(void*), _align_compare);
	desktop_set_alignment(desktop, desktop->prefs.alignment);
}

static int _align_compare(const void * a, const void * b)
{
	DesktopIcon * icona = *(DesktopIcon**)a;
	DesktopIcon * iconb = *(DesktopIcon**)b;
	gboolean firsta = desktopicon_get_first(icona);
	gboolean firstb = desktopicon_get_first(iconb);
	gboolean dira;
	gboolean dirb;

	if(firsta && !firstb)
		return -1;
	else if(!firsta && firstb)
		return 1;
	dira = desktopicon_get_isdir(icona);
	dirb = desktopicon_get_isdir(iconb);
	if(dira && !dirb)
		return -1;
	else if(!dira && dirb)
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

static int _error_text(char const * message, int ret)
{
	fputs("desktop: ", stderr);
	perror(message);
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
					"/" DESKTOPRC, NULL)) == NULL)
	{
		if(config != NULL)
			config_delete(config);
		if(pathname != NULL)
			object_delete(pathname);
		_desktop_serror(NULL, _("Could not load preferences"), FALSE);
		return NULL;
	}
	config_load(config, pathname); /* XXX ignore errors */
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
	unsigned char * p = NULL;
	unsigned long * u;

	if(desktop->prefs.monitor >= 0 && desktop->prefs.monitor
			< gdk_screen_get_n_monitors(desktop->screen))
	{
		gdk_screen_get_monitor_geometry(desktop->screen,
				desktop->prefs.monitor, &desktop->workarea);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() (%d, %d) %dx%d\n", __func__,
				desktop->workarea.x, desktop->workarea.y,
				desktop->workarea.width,
				desktop->workarea.height);
#endif
		return 0;
	}
	atom = gdk_x11_get_xatom_by_name("_NET_WORKAREA");
	if(XGetWindowProperty(GDK_DISPLAY_XDISPLAY(desktop->display),
				GDK_WINDOW_XWINDOW(desktop->root), atom, 0,
				G_MAXLONG, False, XA_CARDINAL, &type, &format,
				&cnt, &bytes, &p) == Success && cnt >= 4)
	{
		u = (unsigned long *)p;
		desktop->workarea.x = u[0];
		desktop->workarea.y = u[1];
		if((desktop->workarea.width = u[2]) == 0
				|| (desktop->workarea.height = u[3]) == 0)
			gdk_screen_get_monitor_geometry(desktop->screen, 0,
					&desktop->workarea);
	}
	else
		gdk_screen_get_monitor_geometry(desktop->screen, 0,
				&desktop->workarea);
	XFree(p);
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() (%d, %d) %dx%d\n", __func__,
			desktop->workarea.x, desktop->workarea.y,
			desktop->workarea.width, desktop->workarea.height);
#endif
	desktop_icons_align(desktop);
	return 0;
}


/* useful */
/* desktop_background */
static void _background_centered(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error);
static void _background_scaled(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error);
static void _background_scaled_ratio(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error);
static void _background_tiled(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error);

static void _desktop_draw_background(Desktop * desktop, GdkColor * color,
		char const * filename, DesktopHows how, gboolean extend)
{
	GdkGC * gc;
	GdkPixmap * pixmap;
	GdkRectangle window = desktop->window;
	GError * error = NULL;
	gint n = 1;
	gint i;

	/* draw default color */
	pixmap = gdk_pixmap_new(desktop->root, window.width, window.height, -1);
	gc = gdk_gc_new(pixmap);
	gdk_gc_set_rgb_fg_color(gc, color);
	gdk_draw_rectangle(pixmap, gc, TRUE, 0, 0, window.width, window.height);
	/* obtain monitor information */
	if(extend != TRUE)
		n = gdk_screen_get_n_monitors(desktop->screen);
	for(i = 0; filename != NULL && i < n; i++)
	{
		if(extend != TRUE)
			gdk_screen_get_monitor_geometry(desktop->screen, i,
					&window);
		switch(how)
		{
			case DESKTOP_HOW_CENTERED:
				_background_centered(&window, pixmap, filename,
						&error);
				break;
			case DESKTOP_HOW_SCALED_RATIO:
				_background_scaled_ratio(&window, pixmap,
						filename, &error);
				break;
			case DESKTOP_HOW_TILED:
				_background_tiled(&window, pixmap, filename,
						&error);
				break;
			case DESKTOP_HOW_SCALED:
				_background_scaled(&window, pixmap, filename,
						&error);
				break;
		}
		if(error != NULL)
		{
			desktop_error(desktop, error->message, 1);
			g_error_free(error);
		}
	}
	gdk_window_set_back_pixmap(desktop->root, pixmap, FALSE);
	gdk_window_clear(desktop->root);
	gdk_pixmap_unref(pixmap);
}

static void _background_centered(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error)
{
	GdkPixbuf * background;
	gint w;
	gint h;

	if((background = gdk_pixbuf_new_from_file(filename, error)) == NULL)
		return;
	w = gdk_pixbuf_get_width(background);
	h = gdk_pixbuf_get_height(background);
	gdk_draw_pixbuf(pixmap, NULL, background, 0, 0,
			(window->width - w) / 2 + window->x,
			(window->height - h) / 2 + window->y, w, h,
			GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref(background);
}

static void _background_scaled(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error)
{
	GdkPixbuf * background;
	gint w;
	gint h;

#if GTK_CHECK_VERSION(2, 6, 0)
	background = gdk_pixbuf_new_from_file_at_scale(filename, window->width,
			window->height, FALSE, error);
#elif GTK_CHECK_VERSION(2, 4, 0)
	background = gdk_pixbuf_new_from_file_at_size(filename, window->width,
			window->height, error);
#else
	background = gdk_pixbuf_new_from_file(filename, &error);
#endif
	if(background == NULL)
		return;
	w = gdk_pixbuf_get_width(background);
	h = gdk_pixbuf_get_height(background);
	gdk_draw_pixbuf(pixmap, NULL, background, 0, 0,
			(window->width - w) / 2 + window->x,
			(window->height - h) / 2 + window->y, w, h,
			GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref(background);
}

static void _background_scaled_ratio(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error)
{
	GdkPixbuf * background;
	gint w;
	gint h;

#if GTK_CHECK_VERSION(2, 4, 0)
	background = gdk_pixbuf_new_from_file_at_size(filename, window->width,
			window->height, error);
	if(background == NULL)
		return; /* XXX report error */
	w = gdk_pixbuf_get_width(background);
	h = gdk_pixbuf_get_height(background);
	gdk_draw_pixbuf(pixmap, NULL, background, 0, 0,
			(window->width - w) / 2 + window->x,
			(window->height - h) / 2 + window->y, w, h,
			GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref(background);
#else
	_background_scaled(desktop, pixmap, filename);
#endif
}

static void _background_tiled(GdkRectangle * window, GdkPixmap * pixmap,
		char const * filename, GError ** error)
{
	GdkPixbuf * background;
	gint w;
	gint h;
	gint i;
	gint j;

	if((background = gdk_pixbuf_new_from_file(filename, error)) == NULL)
		return; /* XXX report error */
	w = gdk_pixbuf_get_width(background);
	h = gdk_pixbuf_get_height(background);
	for(j = 0; j < window->height; j += h)
		for(i = 0; i < window->width; i += w)
			gdk_draw_pixbuf(pixmap, NULL, background, 0, 0,
					i + window->x, j + window->y, w, h,
					GDK_RGB_DITHER_NONE, 0, 0);
	g_object_unref(background);
}


/* desktop_icon_add */
static int _desktop_icon_add(Desktop * desktop, DesktopIcon * icon)
{
	DesktopIcon ** p;

	if((p = realloc(desktop->icon, sizeof(*p) * (desktop->icon_cnt + 1)))
			== NULL)
	{
		desktop_error(desktop, desktopicon_get_name(icon), 0);
		return -1;
	}
	desktop->icon = p;
	desktop->icon[desktop->icon_cnt++] = icon;
	desktopicon_show(icon);
	return 0;
}


/* desktop_icon_remove */
static int _desktop_icon_remove(Desktop * desktop, DesktopIcon * icon)
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
			desktop->icon = p; /* we can ignore errors... */
		else if(desktop->icon_cnt == 0)
			desktop->icon = NULL; /* ...except when it's not one */
		return 0;
	}
	return 1;
}


/* desktop_show_preferences */
static void _preferences_background(Desktop * desktop, GtkWidget * notebook);
static void _preferences_monitors(Desktop * desktop, GtkWidget * notebook);
static void _preferences_theme(Desktop * desktop, GtkWidget * notebook);
static void _preferences_set(Desktop * desktop);
static gboolean _on_preferences_closex(gpointer data);
static void _on_preferences_monitors_changed(gpointer data);
static void _on_preferences_monitors_refresh(gpointer data);
static void _on_preferences_response(GtkWidget * widget, gint response,
		gpointer data);
static void _on_preferences_ok(gpointer data);
static void _on_preferences_apply(gpointer data);
static void _on_preferences_cancel(gpointer data);
static void _on_preferences_update_preview(gpointer data);

static void _desktop_show_preferences(Desktop * desktop)
{
	GtkWidget * vbox;
	GtkWidget * notebook;

	if(desktop->menu != NULL)
		gtk_widget_destroy(desktop->menu);
	desktop->menu = NULL;
	if(desktop->pr_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(desktop->pr_window));
		return;
	}
	/* window */
	desktop->pr_window = gtk_dialog_new_with_buttons(
			_("Desktop preferences"), NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(G_OBJECT(desktop->pr_window), "delete-event",
			G_CALLBACK(_on_preferences_closex), desktop);
	g_signal_connect(G_OBJECT(desktop->pr_window), "response", G_CALLBACK(
				_on_preferences_response), desktop);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(desktop->pr_window));
#else
	vbox = GTK_DIALOG(desktop->pr_window)->vbox;
#endif
	/* notebook */
	notebook = gtk_notebook_new();
	_preferences_background(desktop, notebook);
	_preferences_monitors(desktop, notebook);
	_preferences_theme(desktop, notebook);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	/* container */
	_preferences_set(desktop);
	gtk_widget_show_all(desktop->pr_window);
}

static void _preferences_background(Desktop * desktop, GtkWidget * notebook)
{
	GtkSizeGroup * group;
	GtkWidget * vbox2;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkFileFilter * filter;

	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Default color: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_color = gtk_color_button_new();
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_color, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Filename: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_background = gtk_file_chooser_button_new(_("Background"),
			GTK_FILE_CHOOSER_ACTION_OPEN);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Picture files"));
	gtk_file_filter_add_mime_type(filter, "image/bmp");
	gtk_file_filter_add_mime_type(filter, "image/gif");
	gtk_file_filter_add_mime_type(filter, "image/jpeg");
	gtk_file_filter_add_mime_type(filter, "image/pbm");
	gtk_file_filter_add_mime_type(filter, "image/png");
	gtk_file_filter_add_mime_type(filter, "image/svg+xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(desktop->pr_background),
			filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(desktop->pr_background),
			filter);
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(
				desktop->pr_background), gtk_image_new());
	g_signal_connect_swapped(desktop->pr_background, "update-preview",
			G_CALLBACK(_on_preferences_update_preview), desktop);
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_background, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Position: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_background_how = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_background_how),
			_("Centered"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_background_how),
			_("Scaled"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_background_how),
			_("Scaled (keep ratio)"));
	gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_background_how),
			_("Tiled"));
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_background_how, TRUE,
			TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	desktop->pr_background_extend = gtk_check_button_new_with_mnemonic(
			_("E_xtend background to all monitors"));
	gtk_box_pack_start(GTK_BOX(vbox2), desktop->pr_background_extend, FALSE,
			TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new(
				_("Background")));
}

static void _preferences_monitors(Desktop * desktop, GtkWidget * notebook)
{
	GtkSizeGroup * group;
	GtkWidget * vbox2;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * widget;

	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	/* selector */
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Monitor: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_monitors = gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_monitors, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* geometry */
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Resolution: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_monitors_res = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(desktop->pr_monitors_res), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_monitors_res, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* size */
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Size: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_monitors_size = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(desktop->pr_monitors_size), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_monitors_size, TRUE, TRUE,
			0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* refresh */
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(NULL);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				_on_preferences_monitors_refresh), desktop);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	/* updates */
	g_signal_connect_swapped(desktop->pr_monitors, "changed", G_CALLBACK(
				_on_preferences_monitors_changed), desktop);
	_on_preferences_monitors_refresh(desktop);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new(
				_("Monitors")));
}

static void _preferences_theme(Desktop * desktop, GtkWidget * notebook)
{
	GtkSizeGroup * group;
	GtkWidget * vbox2;
	GtkWidget * hbox;
	GtkWidget * label;

	vbox2 = gtk_vbox_new(FALSE, 4);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Desktop font: "));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_size_group_add_widget(group, label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	desktop->pr_font = gtk_font_button_new();
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(desktop->pr_font), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), desktop->pr_font, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, TRUE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox2, gtk_label_new(
				_("Theme")));
}

static gboolean _on_preferences_closex(gpointer data)
{
	_on_preferences_cancel(data);
	return TRUE;
}

static void _on_preferences_monitors_changed(gpointer data)
{
	Desktop * desktop = data;
	gint active;
	GdkRectangle geometry;
	gint width;
	gint height;
	char buf[64];

	active = gtk_combo_box_get_active(GTK_COMBO_BOX(desktop->pr_monitors));
	geometry.x = 0;
	geometry.y = 0;
	geometry.width = gdk_screen_get_width(desktop->screen);
	geometry.height = gdk_screen_get_height(desktop->screen);
	width = gdk_screen_get_width_mm(desktop->screen);
	height = gdk_screen_get_height_mm(desktop->screen);
#if GTK_CHECK_VERSION(2, 14, 0)
	if(active-- > 0)
	{
		gdk_screen_get_monitor_geometry(desktop->screen, active,
				&geometry);
		width = gdk_screen_get_monitor_width_mm(desktop->screen,
				active);
		height = gdk_screen_get_monitor_height_mm(desktop->screen,
				active);
	}
#endif
	snprintf(buf, sizeof(buf), _("%dx%d (at %d,%d)"), geometry.width,
			geometry.height, geometry.x, geometry.y);
	gtk_label_set_text(GTK_LABEL(desktop->pr_monitors_res), buf);
	snprintf(buf, sizeof(buf), _("%dx%d mm (%.0lfx%.0lf DPI)"), width,
			height, geometry.width * 25.4 / width,
			geometry.height * 25.4 / height);
	gtk_label_set_text(GTK_LABEL(desktop->pr_monitors_size), buf);
}

static void _on_preferences_monitors_refresh(gpointer data)
{
	Desktop * desktop = data;
	GtkTreeModel * model;
#if GTK_CHECK_VERSION(2, 14, 0)
	gint n;
	gint i;
	char * name;
	char buf[32];
#endif

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(desktop->pr_monitors));
	gtk_list_store_clear(GTK_LIST_STORE(model));
	gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_monitors),
			_("Whole screen"));
#if GTK_CHECK_VERSION(2, 14, 0)
	n = gdk_screen_get_n_monitors(desktop->screen);
	for(i = 0; i < n; i++)
	{
		snprintf(buf, sizeof(buf), _("Monitor %d"), i);
		name = gdk_screen_get_monitor_plug_name(desktop->screen, i);
		gtk_combo_box_append_text(GTK_COMBO_BOX(desktop->pr_monitors),
				(name != NULL) ? name : buf);
		g_free(name);
	}
#endif
	gtk_combo_box_set_active(GTK_COMBO_BOX(desktop->pr_monitors), 0);
}

static void _on_preferences_response(GtkWidget * widget, gint response,
		gpointer data)
{
	if(response == GTK_RESPONSE_OK)
		_on_preferences_ok(data);
	else if(response == GTK_RESPONSE_APPLY)
		_on_preferences_apply(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_on_preferences_cancel(data);
}

static void _on_preferences_ok(gpointer data)
{
	Desktop * desktop = data;

	gtk_widget_hide(desktop->pr_window);
	_on_preferences_apply(data);
}

static void _on_preferences_apply(gpointer data)
{
	Desktop * desktop = data;
	Config * config;
	GdkColor color;
	char * p;
	char const * q;
	int i;

	/* XXX not very efficient */
	g_idle_add(_new_idle, desktop);
	if((config = _desktop_get_config(desktop)) == NULL)
		return;
	q = gtk_font_button_get_font_name(GTK_FONT_BUTTON(desktop->pr_font));
	config_set(config, NULL, "font", q);
	p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
				desktop->pr_background));
	config_set(config, NULL, "background", p);
	g_free(p);
	gtk_color_button_get_color(GTK_COLOR_BUTTON(desktop->pr_color), &color);
	p = gdk_color_to_string(&color);
	config_set(config, NULL, "background_color", p);
	g_free(p);
	i = gtk_combo_box_get_active(GTK_COMBO_BOX(desktop->pr_background_how));
	if(i >= 0 && i < DESKTOP_HOW_COUNT)
		config_set(config, NULL, "background_how", _desktop_hows[i]);
	p = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				desktop->pr_background_extend)) ? "1" : "0";
	config_set(config, NULL, "background_extend", p);
	/* XXX code duplication */
	if((p = string_new_append(desktop->home, "/" DESKTOPRC, NULL)) != NULL)
	{
		config_save(config, p);
		string_delete(p);
	}
	config_delete(config);
}

static void _on_preferences_cancel(gpointer data)
{
	Desktop * desktop = data;

	gtk_widget_hide(desktop->pr_window);
	_preferences_set(desktop);
}

static void _on_preferences_update_preview(gpointer data)
{
	Desktop * desktop = data;
#if !GTK_CHECK_VERSION(2, 6, 0)
	gint ratio = desktop->window.width / desktop->window.height;
#endif
	GtkFileChooser * chooser = GTK_FILE_CHOOSER(desktop->pr_background);
	GtkWidget * widget;
	char * filename;
	GdkPixbuf * pixbuf;
	gboolean active = FALSE;
	GError * error = NULL;

	widget = gtk_file_chooser_get_preview_widget(chooser);
	if((filename = gtk_file_chooser_get_preview_filename(chooser)) != NULL)
	{
#if GTK_CHECK_VERSION(2, 6, 0)
		pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, 96, -1,
				TRUE, &error);
#else
		pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 96,
				96 / ratio, &error);
#endif
		if(pixbuf != NULL)
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
			g_object_unref(pixbuf);
			active = TRUE;
		}
		else if(error != NULL)
		{
			desktop_error(NULL, error->message, 1);
			g_error_free(error);
		}
	}
	g_free(filename);
	gtk_file_chooser_set_preview_widget_active(chooser, active);
}

static void _preferences_set(Desktop * desktop)
{
	Config * config;
	String const * p;
	String const * filename = NULL;
	GdkColor color = { 0, 0, 0, 0 };
	int how = 0;
	gboolean extend = FALSE;
	size_t i;

	if((config = _desktop_get_config(desktop)) != NULL)
	{
		if((p = config_get(config, NULL, "font")) != NULL)
			gtk_font_button_set_font_name(GTK_FONT_BUTTON(
						desktop->pr_font), p);
		filename = config_get(config, NULL, "background");
		if((p = config_get(config, NULL, "background_color")) != NULL
				&& gdk_color_parse(p, &color) == TRUE)
			gtk_color_button_set_color(GTK_COLOR_BUTTON(
						desktop->pr_color), &color);
		if((p = config_get(config, NULL, "background_how")) != NULL)
			for(i = 0; i < DESKTOP_HOW_COUNT; i++)
				if(strcmp(_desktop_hows[i], p) == 0)
					how = i;
		if((p = config_get(config, NULL, "background_extend")) != NULL)
			extend = strtol(p, NULL, 10) ? TRUE : FALSE;
		config_delete(config);
	}
	if(filename != NULL)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
					desktop->pr_background), filename);
	else
		gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(
					desktop->pr_background));
	gtk_combo_box_set_active(GTK_COMBO_BOX(desktop->pr_background_how),
			how);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
				desktop->pr_background_extend), extend);
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: desktop [-H|-V][-a|-c|-f|-h|-n][-m monitor]\n"
"  -H	Place icons horizontally\n"
"  -V	Place icons vertically\n"
"  -a	Display the applications registered\n"
"  -c	Sort the applications registered by category\n"
"  -f	Display contents of the desktop folder (default)\n"
"  -h	Display the homescreen\n"
"  -m	Monitor where to display the desktop\n"
"  -n	Do not display icons on the desktop\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	Desktop * desktop;
	DesktopPrefs prefs;
	int alignment = -1;
	char * p;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	memset(&prefs, 0, sizeof(prefs));
	prefs.layout = DESKTOP_LAYOUT_FILES;
	prefs.monitor = -1;
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "HVacfhm:n")) != -1)
		switch(o)
		{
			case 'H':
				alignment = DESKTOP_ALIGNMENT_HORIZONTAL;
				break;
			case 'V':
				alignment = DESKTOP_ALIGNMENT_VERTICAL;
				break;
			case 'a':
				prefs.layout = DESKTOP_LAYOUT_APPLICATIONS;
				break;
			case 'c':
				prefs.layout = DESKTOP_LAYOUT_CATEGORIES;
				break;
			case 'f':
				prefs.layout = DESKTOP_LAYOUT_FILES;
				break;
			case 'h':
				prefs.layout = DESKTOP_LAYOUT_HOMESCREEN;
				break;
			case 'm':
				prefs.monitor = strtol(optarg, &p, 0);
				if(optarg[0] == '\0' || *p != '\0')
					return _usage();
				break;
			case 'n':
				prefs.layout = DESKTOP_LAYOUT_NONE;
				break;
			default:
				return _usage();
		}
	if(optind < argc)
		return _usage();
	if(alignment < 0)
		alignment = (prefs.layout == DESKTOP_LAYOUT_FILES)
			? DESKTOP_ALIGNMENT_VERTICAL
			: DESKTOP_ALIGNMENT_HORIZONTAL;
	prefs.alignment = alignment;
	if((desktop = desktop_new(&prefs)) == NULL)
	{
		gtk_main();
		return 2;
	}
	gtk_main();
	desktop_delete(desktop);
	return 0;
}
