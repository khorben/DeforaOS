/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
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
/* FIXME:
 * - make sure all pathnames point to the top directory
 * - load the configuration by appending "/project.conf" as required */



#include <stdlib.h>
#include <gtk/gtk.h>
#include <System.h>
#include "project.h"


/* Project */
/* private */
/* types */
struct _Project
{
	String * pathname;		/* project's top directory */
	Config * config;

	/* widgets */
	/* properties window */
	GtkWidget * pr_window;
};


/* public */
/* functions */
/* project_new */
Project * project_new(void)
{
	Project * p;

	if((p = object_new(sizeof(*p))) == NULL)
		return NULL;
	p->pathname = NULL;
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
	string_delete(project->pathname);
	if(project->config != NULL)
		config_delete(project->config);
	object_delete(project);
}


/* accessors */
/* project_get_package */
char const * project_get_package(Project * project)
{
	return config_get(project->config, NULL, "package");
}


/* project_get_pathname */
char const * project_get_pathname(Project * project)
{
	return project->pathname;
}


/* project_set_pathname */
int project_set_pathname(Project * project, char const * pathname)
{
	String * p;

	if((p = string_new(pathname)) == NULL)
		return -1;
	string_delete(project->pathname);
	project->pathname = p;
	return 0;
}


/* useful */
/* project_load */
int project_load(Project * project, char const * pathname)
{
	Config * config;
	String * p;
	char const * package;
	char const * version;

	config = config_new();
	p = string_new(pathname);
	if(config != NULL && p != NULL && config_load(config, p) == 0)
	{
		package = config_get(config, NULL, "package");
		version = config_get(config, NULL, "version");
		if(package != NULL && version != NULL)
		{
			string_delete(project->pathname);
			project->pathname = p;
			config_delete(project->config);
			project->config = config;
			return 0;
		}
		error_set_code(1, "%s", "Project file is missing"
					" package name and version");
	}
	return -1;
}


/* project_save */
int project_save(Project * project)
{
	if(project->pathname == NULL)
		return -error_set_code(1, "%s",
				"No path defined for the project");
	/* FIXME implement */
	return -error_set_code(1, "%s", "Not implemented yet");
}


/* project_properties */
static void _properties_new(Project * p);
static gboolean _on_properties_closex(gpointer data);

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

static gboolean _on_properties_closex(gpointer data)
{
	Project * p = data;

	gtk_widget_hide(p->pr_window);
	return TRUE;
}
