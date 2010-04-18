/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



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
