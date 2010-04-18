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



#ifndef GEDI_PROJECT_H
# define GEDI_PROJECT_H

# include <System.h>
# include <gtk/gtk.h>


/* types */
typedef struct _Project
{
	Config * config;

	/* widgets */
	/* properties window */
	GtkWidget * pr_window;
} Project;


/* functions */
Project * project_new(void);
void project_delete(Project * project);

/* accessors */
char const * project_get_package(Project * package);

/* useful */
int project_load(Project * project, char const * filename);

void project_properties(Project * project);

#endif /* !GEDI_PROJECT_H */
