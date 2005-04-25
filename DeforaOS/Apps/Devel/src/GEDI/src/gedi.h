/* gedi.h */



#ifndef __GEDI_H
# define __GEDI_H

# include <gtk/gtk.h>
# include <libutils.h>
# include "project.h"


/* types */
typedef struct _GEDI
{
	Config * config;
	Project ** projects;

	/* widgets */
	/* toolbar */
	GtkWidget * tb_window;
	GtkWidget * tb_vbox;
	GtkWidget * tb_menubar;
	GtkWidget * tb_toolbar;

	/* about */
	GtkWidget * ab_window;
} GEDI;


/* functions */
GEDI * gedi_new(void);
void gedi_delete(GEDI * gedi);

#endif /* !__GEDI_H */
