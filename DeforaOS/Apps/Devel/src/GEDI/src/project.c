/* project.c */



#include <stdlib.h>
#include "project.h"


/* Project */
/* callbacks */
static void _on_properties_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data);

/* project_new */
Project * project_new(void)
{
	Project * p;

	if((p = malloc(sizeof(Project))) == NULL)
		return NULL;
	p->pr_window = NULL;
	return p;
}


/* project_delete */
void project_delete(Project * project)
{
	free(project);
}


/* useful */
/* project_properties */
static void _properties_new(Project * p);
void project_properties(Project * project)
{
	if(project->pr_window == NULL)
		_properties_new(project);
	gtk_widget_show_all(project->pr_window);
}

static void _properties_new(Project * p)
{
	p->pr_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(p->pr_window), 4);
	gtk_window_set_title(GTK_WINDOW(p->pr_window), "Project properties");
	g_signal_connect(G_OBJECT(p->pr_window), "delete_event",
			G_CALLBACK(_on_properties_xkill), p);
	/* FIXME */
}


/* callbacks */
static void _on_properties_xkill(GtkWidget * widget, GdkEvent * event,
		gpointer data)
{
	Project * p = data;

	gtk_widget_hide(p->pr_window);
}
