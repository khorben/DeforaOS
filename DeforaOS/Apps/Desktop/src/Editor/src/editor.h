/* editor.h */



#ifndef EDITOR_EDITOR_H
# define EDITOR_EDITOR_H

# include <gtk/gtk.h>


/* types */
typedef struct _Editor
{
	char * filename;
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

/* useful */
gboolean editor_close(Editor * editor);
void editor_open(Editor * editor, char const * filename);
void editor_open_dialog(Editor * editor);

#endif /* !EDITOR_EDITOR_H */
