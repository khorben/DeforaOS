/* callbacks.h */



#ifndef BROWSER_CALLBACKS_H
# define BROWSER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

/* file menu */
void on_file_new_window(GtkMenuItem * menuitem, gpointer data);
void on_file_close(GtkMenuItem * menuitem, gpointer data);

/* edit menu */
void on_edit_copy(GtkMenuItem * menuitem, gpointer data);
void on_edit_cut(GtkMenuItem * menuitem, gpointer data);
void on_edit_delete(GtkMenuItem * menuitem, gpointer data);
void on_edit_preferences(GtkMenuItem * menuitem, gpointer data);
void on_edit_select_all(GtkMenuItem * menuitem, gpointer data);
void on_edit_unselect_all(GtkMenuItem * menuitem,
		gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);

/* view menu */
void on_view_home(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_details(GtkWidget * widget, gpointer data);
void on_view_icons(GtkWidget * widget, gpointer data);
void on_view_list(GtkWidget * widget, gpointer data);
#endif

/* toolbar */
void on_back(GtkWidget * widget, gpointer data);
void on_forward(GtkWidget * widget, gpointer data);
void on_home(GtkWidget * widget, gpointer data);
void on_properties(GtkWidget * widget, gpointer data);
void on_refresh(GtkWidget * widget, gpointer data);
void on_updir(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_view_as(GtkWidget * widget, gpointer data);
#endif

/* address bar */
void on_path_activate(GtkWidget * widget, gpointer data);
void on_path_change(GtkWidget * widget, gpointer data);

/* view */
void on_detail_default(GtkTreeView * view,
		GtkTreePath * tree_path, GtkTreeViewColumn * column,
		gpointer data);
#if GTK_CHECK_VERSION(2, 6, 0)
void on_icon_default(GtkIconView * view, GtkTreePath *tree_path,
		gpointer data);
#endif
void on_filename_edited(GtkCellRendererText * renderer, gchar * arg1,
		gchar * arg2, gpointer data);
gboolean on_view_popup(GtkWidget * widget, GdkEventButton * event,
		gpointer data);

#endif /* !BROWSER_CALLBACKS_H */
