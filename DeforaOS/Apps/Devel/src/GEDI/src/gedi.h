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

	/* preferences */
	GtkWidget * pr_window;

	/* about */
	GtkWidget * ab_window;
} GEDI;


/* functions */
GEDI * gedi_new(void);
void gedi_delete(GEDI * gedi);

/* useful */
void gedi_error(GEDI * gedi, char const * title, char const * message);
void gedi_file_open(GEDI * gedi, char const * file);
void gedi_project_open(GEDI * gedi, char const * file);
void gedi_project_save(GEDI * gedi);
void gedi_project_save_as(GEDI * gedi, char const * file);

#endif /* !__GEDI_H */
