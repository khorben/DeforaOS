/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
static gboolean _on_properties_closex(gpointer data);

/* project_new */
Project * project_new(void)
{
	Project * p;

	if((p = malloc(sizeof(*p))) == NULL)
		return NULL;
	p->config = config_new();
	p->pr_window = NULL;
	if(p->config == NULL)
	{
		project_delete(p);
		return NULL;
	}
	return p;
}


/* project_delete */
void project_delete(Project * project)
{
	if(project->config != NULL)
		config_delete(project->config);
	free(project);
}


/* accessors */
/* project_get_package */
char const * project_get_package(Project * package)
{
	return config_get(package->config, "", "package");
}


/* useful */
/* project_load */
int project_load(Project * project, char const * filename)
{
	char const * package;
	char const * version;

	config_reset(project->config);
	if(config_load(project->config, filename) != 0)
		return 1;
	package = config_get(project->config, "", "package");
	version = config_get(project->config, "", "version");
	if(package == NULL || version == NULL)
		return error_set_code(1, "%s", "Project file is missing"
				" package name and version");
	return 0;
}


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
	g_signal_connect_swapped(G_OBJECT(p->pr_window), "delete-event",
			G_CALLBACK(_on_properties_closex), p);
	/* FIXME */
}


/* callbacks */
static gboolean _on_properties_closex(gpointer data)
{
	Project * p = data;

	gtk_widget_hide(p->pr_window);
	return TRUE;
}
