/* $Id$ */
/* Copyright (c) 2007 The DeforaOS Project */
/* This file is part of Browser */
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



#ifndef BROWSER_BROWSER_H
# define BROWSER_BROWSER_H

# include <dirent.h>
# include <gtk/gtk.h>
# include "mime.h"


/* Browser */
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
	BR_COL_SIZE,
	BR_COL_OWNER,
	BR_COL_GROUP,
	BR_COL_MIME_TYPE,
	BR_NUM_COLS
};
# define BR_LAST BR_NUM_COLS

typedef enum _BrowserView
{
	BV_DETAILS = 0,
# if GTK_CHECK_VERSION(2, 6, 0)
	BV_ICONS,
	BV_LIST,
	BV_THUMBNAILS
} BrowserView;
#  define BV_LAST BV_THUMBNAILS
# else
} BrowserView;
#  define BV_LAST BV_DETAILS
# endif

typedef struct _BrowserPreferences
{
	gboolean sort_folders_first;
	gboolean show_hidden_files;
} BrowserPreferences;

typedef struct _Browser
{
	/* config */
/*	Config * config; */
	BrowserPreferences prefs;

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

	/* widgets */
	GtkIconTheme * theme;
	GdkPixbuf * pb_file_24;
	GdkPixbuf * pb_folder_24;
	GdkPixbuf * pb_executable_24;
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * pb_file_48;
	GdkPixbuf * pb_folder_48;
	GdkPixbuf * pb_executable_48;
	GdkPixbuf * pb_file_96;
	GdkPixbuf * pb_folder_96;
	GdkPixbuf * pb_executable_96;
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
#endif
	GtkListStore * store;
	GtkWidget * statusbar;
	guint statusbar_id;
	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_sort;
	GtkWidget * pr_hidden;
} Browser;


/* variables */
extern unsigned int browser_cnt;


/* functions */
Browser * browser_new(char const * directory);
void browser_delete(Browser * browser);

/* useful */
int browser_error(Browser * browser, char const * message, int ret);
void browser_refresh(Browser * browser);
void browser_select_all(Browser * browser);
void browser_set_location(Browser * browser, char const * path);
void browser_set_view(Browser * browser, BrowserView view);
void browser_unselect_all(Browser * browser);

#endif /* !BROWSER_BROWSER_H */
