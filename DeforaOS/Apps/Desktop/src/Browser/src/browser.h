/* $Id$ */
/* Copyright (c) 2006 The DeforaOS Project */



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
	BV_LIST
} BrowserView;
#  define BV_LAST BV_LIST
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
#if GTK_CHECK_VERSION(2, 6, 0)
	GdkPixbuf * pb_file_48;
	GdkPixbuf * pb_folder_48;
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
