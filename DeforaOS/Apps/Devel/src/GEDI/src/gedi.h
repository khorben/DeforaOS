/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel GEDI */
/* GEDI is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * GEDI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * GEDI; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



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
	Project * project;

	/* widgets */
	/* toolbar */
	GtkWidget * tb_window;
	GtkWidget * tb_vbox;
	GtkWidget * tb_menubar;
	GtkWidget * tb_toolbar;

	/* preferences */
	GtkWidget * pr_window;
} GEDI;


/* functions */
GEDI * gedi_new(void);
void gedi_delete(GEDI * gedi);

/* useful */
int gedi_error(GEDI * gedi, char const * title, char const * message);

void gedi_file_open(GEDI * gedi, char const * file);
void gedi_project_open(GEDI * gedi, char const * file);
void gedi_project_save(GEDI * gedi);
void gedi_project_save_as(GEDI * gedi, char const * file);

#endif /* !GEDI_GEDI_H */
