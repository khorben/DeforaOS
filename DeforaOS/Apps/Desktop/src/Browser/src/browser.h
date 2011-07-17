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
enum
{
	BR_COL_UPDATED = 0,
	BR_COL_PATH,
	BR_COL_DISPLAY_NAME,
	BR_COL_PIXBUF_24,
# if GTK_CHECK_VERSION(2, 6, 0)
	BR_COL_PIXBUF_48,
	BR_COL_PIXBUF_96,
# endif
	BR_COL_INODE,
	BR_COL_IS_DIRECTORY,
	BR_COL_IS_EXECUTABLE,
	BR_COL_IS_MOUNT_POINT,
	BR_COL_SIZE,
	BR_COL_DISPLAY_SIZE,
	BR_COL_OWNER,
	BR_COL_GROUP,
	BR_COL_DATE,
	BR_COL_DISPLAY_DATE,
	BR_COL_MIME_TYPE
};
# define BR_COL_LAST BR_COL_MIME_TYPE
# define BR_COL_COUNT (BR_COL_LAST + 1)

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
	GtkToolItem * tb_back;
	GtkToolItem * tb_updir;
	GtkToolItem * tb_forward;
	GtkWidget * tb_path;
	GtkWidget * scrolled;
	GtkWidget * detailview;
#if GTK_CHECK_VERSION(2, 6, 0)
	GtkWidget * iconview;
	BrowserView iconview_as;
#endif
	GtkListStore * store;
	GtkWidget * statusbar;
	guint statusbar_id;
	/* plugins */
	GtkListStore * pl_store;
	GtkWidget * pl_combo;
	GtkWidget * pl_view;
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
void browser_set_location(Browser * browser, char const * path);
void browser_set_view(Browser * browser, BrowserView view);

/* useful */
void browser_about(Browser * browser);

int browser_error(Browser * browser, char const * message, int ret);

int browser_config_load(Browser * browser);
int browser_config_save(Browser * browser);

void browser_focus_location(Browser * browser);

void browser_go_home(Browser * browser);

int browser_load(Browser * browser, char const * plugin);

void browser_open(Browser * browser, char const * path);
void browser_open_with(Browser * browser, char const * path);

void browser_refresh(Browser * browser);

void browser_select_all(Browser * browser);
void browser_unselect_all(Browser * browser);

void browser_view_preferences(Browser * browser);

#endif /* !BROWSER_BROWSER_H */
