/* callbacks.h */



#ifndef MAILER_CALLBACKS_H
# define MAILER_CALLBACKS_H

# include <gtk/gtk.h>


/* window */
gboolean on_closex(GtkWidget * widget, GdkEvent * event, gpointer data);

/* file menu */
void on_file_new_mail(GtkWidget * widget, gpointer data);
void on_file_quit(GtkWidget * widget, gpointer data);

/* edit menu */
void on_edit_preferences(GtkWidget * widget, gpointer data);

/* help menu */
void on_help_about(GtkWidget * widget, gpointer data);

/* compose window */
gboolean on_compose_closex(GtkWidget * widget, GdkEvent * event, gpointer data);
void on_compose_file_close(GtkWidget * widget, gpointer data);
void on_compose_help_about(GtkWidget * widget, gpointer data);
void on_compose_save(GtkWidget * widget, gpointer data);

#endif /* !MAILER_CALLBACKS_H */
