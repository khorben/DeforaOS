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

/* useful */
void project_properties(Project * project);

#endif /* !GEDI_PROJECT_H */
