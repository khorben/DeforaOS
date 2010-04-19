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



#ifndef GEDI_GEDI_H
# define GEDI_GEDI_H

# include <System.h>
# include <gtk/gtk.h>
# include "project.h"


/* types */
typedef struct _GEDI
{
	Config * config;
	Project ** projects;
	size_t projects_cnt;
	Project * cur;

	/* widgets */
	/* toolbar */
	GtkWidget * tb_window;

	/* preferences */
	GtkWidget * pr_window;

	/* files */
	GtkWidget * fi_window;
	GtkWidget * fi_combo;
	GtkWidget * fi_view;
} GEDI;


/* functions */
GEDI * gedi_new(void);
void gedi_delete(GEDI * gedi);

/* useful */
int gedi_error(GEDI * gedi, char const * message, int ret);

void gedi_file_open(GEDI * gedi, char const * file);
int gedi_project_open(GEDI * gedi, char const * file);
void gedi_project_save(GEDI * gedi);
void gedi_project_save_as(GEDI * gedi, char const * file);

#endif /* !GEDI_GEDI_H */
