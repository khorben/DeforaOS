/* surfer.h */



#ifndef SURFER_SURFER_H
# define SURFER_SURFER_H

# include <gtk/gtk.h>
# include <gtkmozembed.h>


/* Surfer */
/* constants */
# define SURFER_DEFAULT_MINIMUM_FONT_SIZE	8.0
# define SURFER_DEFAULT_FONT_SIZE		12.0
# define SURFER_DEFAULT_FIXED_FONT_SIZE		12.0
# define SURFER_DEFAULT_ENCODING		"ISO-8859-1"
# define SURFER_DEFAULT_SERIF_FONT		"Serif"
# define SURFER_DEFAULT_SANS_FONT		"Sans"
# define SURFER_DEFAULT_STANDARD_FONT		SURFER_DEFAULT_SANS_FONT
# define SURFER_DEFAULT_FIXED_FONT		"Monospace"
# define SURFER_DEFAULT_FANTASY_FONT		"Comic Sans MS"

# define SURFER_GTKMOZEMBED_COMPPATH		"/usr/pkg/lib/firefox"

/* types */
typedef struct _Surfer
{
	/* widgets */
	/* main window */
	GtkWidget * window;
	GtkWidget * menubar;
	GtkToolItem * tb_back;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_refresh;
	GtkWidget * tb_path;
	GtkWidget * view;
	GtkWidget * statusbar;
	guint statusbar_id;
} Surfer;


/* functions */
Surfer * surfer_new(char const * url);
void surfer_delete(Surfer * surfer);


/* useful */
int surfer_error(Surfer * surfer, char const * message, int ret);

#endif /* !SURFER_SURFER_H */
