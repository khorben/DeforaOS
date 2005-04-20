/* gedi.h */



#ifndef __GEDI_H
# define __GEDI_H

# include <gtk/gtk.h>


/* types */
typedef struct _GEDI
{
	/* widgets */
	/* toolbar */
	GtkWidget * window_toolbar;
} GEDI;


/* functions */
GEDI * gedi_new(void);
void gedi_delete(GEDI * gedi);

#endif /* !__GEDI_H */
