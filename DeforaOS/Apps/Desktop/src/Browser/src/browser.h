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



#ifndef BROWSER_BROWSER_H
# define BROWSER_BROWSER_H

# include <dirent.h>
# include <gtk/gtk.h>
# include <System.h>
# include <Desktop.h>
# include "Browser.h"


/* Browser */
/* defaults */
# define BROWSER_CONFIG_FILE		".browser"
# define BROWSER_ICON_WRAP_WIDTH	96
# define BROWSER_LIST_WRAP_WIDTH	118
# define BROWSER_THUMBNAIL_WRAP_WIDTH	112


/* types */
typedef enum _BrowserColumn
{
	BC_UPDATED = 0,
	BC_PATH,
	BC_DISPLAY_NAME,
	BC_PIXBUF_24,
# if GTK_CHECK_VERSION(2, 6, 0)
	BC_PIXBUF_48,
	BC_PIXBUF_96,
# endif
	BC_INODE,
	BC_IS_DIRECTORY,
	BC_IS_EXECUTABLE,
	BC_IS_MOUNT_POINT,
	BC_SIZE,
	BC_DISPLAY_SIZE,
	BC_OWNER,
	BC_GROUP,
	BC_DATE,
	BC_DISPLAY_DATE,
	BC_MIME_TYPE
} BrowserColumn;
# define BC_LAST BC_MIME_TYPE
# define BC_COUNT (BC_LAST + 1)

typedef enum _BrowserView
{
	BV_DETAILS = 0,
# if GTK_CHECK_VERSION(2, 6, 0)
	BV_ICONS,
	BV_LIST,
	BV_THUMBNAILS
} BrowserView;
#  define BV_FIRST BV_DETAILS
#  define BV_LAST BV_THUMBNAILS
# else
} BrowserView;
#  define BV_FIRST BV_DETAILS
#  define BV_LAST BV_DETAILS
# endif
# define BV_COUNT (BV_LAST + 1)

typedef struct _BrowserPrefs
{
# if GTK_CHECK_VERSION(2, 6, 0)
	int default_view;
# endif
	gboolean alternate_rows;
	gboolean confirm_before_delete;
	gboolean sort_folders_first;
	gboolean show_hidden_files;
} BrowserPrefs;

struct _Browser
{
	/* config */
	Config * config;
	BrowserPrefs prefs;

	/* mime */
	Mime * mime;

	/* history */
	GList * history;
	GList * current;

	/* refresh */
	guint refresh_id;
	DIR * refresh_dir;
	dev_t refresh_dev;
	ino_t refresh_ino;
	time_t refresh_mti;
	unsigned int refresh_cnt;
	unsigned int refresh_hid;
	GtkTreeIter refresh_iter;

	/* selection */
	GList * selection;
	gboolean selection_cut;

	/* helper */
	BrowserPluginHelper pl_helper;

	/* widgets */
	GtkIconTheme * theme;
	GdkPixbuf * pb_file_24;
	GdkPixbuf * pb_folder_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * pb_file_48;
	GdkPixbuf * pb_folder_48;
	GdkPixbuf * pb_file_96;
	GdkPixbuf * pb_folder_96;
#endif
	GtkWidget * window;
#if GTK_CHECK_VERSION(2, 18, 0)
	GtkWidget * infobar;
	GtkWidget * infobar_label;
#endif
	GtkToolItem * tb_back;
	GtkToolItem * tb_updir;
	GtkToolItem * tb_forward;
	GtkWidget * tb_path;
	GtkWidget * scrolled;
	GtkWidget * detailview;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * iconview;
	BrowserView view;
#endif
	GtkListStore * store;
	GtkWidget * statusbar;
	guint statusbar_id;
	/* plug-ins */
	GtkWidget * pl_view;
	GtkListStore * pl_store;
	GtkWidget * pl_combo;
	GtkWidget * pl_box;
	/* preferences */
	GtkWidget * pr_window;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * pr_view;
#endif
	GtkWidget * pr_alternate;
	GtkWidget * pr_confirm;
	GtkWidget * pr_sort;
	GtkWidget * pr_hidden;
	GtkListStore * pr_mime_store;
	GtkWidget * pr_mime_view;
	GtkListStore * pr_plugin_store;
	GtkWidget * pr_plugin_view;
	/* about */
	GtkWidget * ab_window;
};


/* variables */
extern unsigned int browser_cnt;


/* functions */
Browser * browser_new(char const * directory);
Browser * browser_new_copy(Browser * browser);
void browser_delete(Browser * browser);

/* accessors */
char const * browser_get_location(Browser * browser);
BrowserView browser_get_view(Browser * browser);

int browser_set_location(Browser * browser, char const * path);
void browser_set_view(Browser * browser, BrowserView view);

/* useful */
void browser_about(Browser * browser);

int browser_error(Browser * browser, char const * message, int ret);

int browser_config_load(Browser * browser);
int browser_config_save(Browser * browser);

void browser_focus_location(Browser * browser);

void browser_go_home(Browser * browser);

/* plug-ins */
int browser_load(Browser * browser, char const * plugin);
int browser_unload(Browser * browser, char const * plugin);

void browser_open(Browser * browser, char const * path);
void browser_open_with(Browser * browser, char const * path);

void browser_refresh(Browser * browser);

/* selection */
void browser_select_all(Browser * browser);
void browser_unselect_all(Browser * browser);

/* interface */
void browser_show_preferences(Browser * browser);

#endif /* !BROWSER_BROWSER_H */
