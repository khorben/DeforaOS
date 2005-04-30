/* project.h */



#ifndef __PROJECT_H
# define __PROJECT_H

# include <gtk/gtk.h>
# include <libutils.h>


/* types */
typedef struct _Project
{
	Config * config;

	/* widgets */
	GtkWidget * pr_window;
} Project;


/* functions */
Project * project_new(void);
void project_delete(Project * project);

/* useful */
void project_properties(Project * project);

#endif /* !__PROJECT_H */
