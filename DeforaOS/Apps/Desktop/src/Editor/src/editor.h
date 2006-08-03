/* editor.h */



#ifndef EDITOR_EDITOR_H
# define EDITOR_EDITOR_H

# include <gtk/gtk.h>


/* types */
typedef struct _Editor
{
	int saved;
	/* widgets */
	GtkWidget * window;
	GtkWidget * view;
	GtkWidget * statusbar;
	/* preferences */
	GtkWidget * pr_window;
} Editor;


/* functions */
Editor * editor_new(void);
void editor_delete(Editor * editor);

#endif /* !EDITOR_EDITOR_H */
