/* browser.h */



#ifndef BROWSER_BROWSER_H
# define BROWSER_BROWSER_H

# include <gtk/gtk.h>


/* Browser */
/* types */
enum
{
	BR_COL_PATH = 0,
	BR_COL_DISPLAY_NAME,
	BR_COL_PIXBUF,
	BR_COL_IS_DIRECTORY,
	BR_NUM_COLS
};
# define BR_LAST BR_NUM_COLS

typedef struct _Browser
{
	/* config */
/*	Config * config; */

	/* history */
	GList * history;
	GList * current;

	/* widgets */
	GdkPixbuf * pb_file;
	GdkPixbuf * pb_folder;
	GtkWidget * window;
	GtkToolItem * tb_back;
	GtkToolItem * tb_updir;
	GtkToolItem * tb_forward;
	GtkWidget * tb_path;
	GtkWidget * iconview; /* FIXME fix selection and probably remove */
	GtkListStore * store;
	GtkWidget * statusbar;
	guint statusbar_id;
} Browser;


/* functions */
Browser * browser_new(char const * directory);
void browser_delete(Browser * browser);

#endif /* !BROWSER_BROWSER_H */
