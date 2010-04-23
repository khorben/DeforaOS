/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
/* Surfer is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * Surfer is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Surfer; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#ifndef SURFER_COMMON_H
# define SURFER_COMMON_H

# include <System.h>
# include <gtk/gtk.h>


/* Surfer */
/* types */
struct _Surfer
{
	Config * config;

	/* preferences */
	char * homepage;

	/* widgets */
	/* main window */
	GtkWidget * window;
#ifndef EMBEDDED
	GtkWidget * menubar;
#endif
	GtkWidget * toolbar;
	GtkToolItem * tb_back;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_refresh;
	GtkToolItem * tb_fullscreen;
	GtkWidget * locationbar;
	GtkWidget * lb_path;
	GtkWidget * notebook;
	GtkWidget * progress;
	GtkWidget * statusbox;
	GtkWidget * statusbar;
	guint statusbar_id;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_homepage;

	/* find */
	GtkWidget * fi_dialog;
	GtkWidget * fi_text;
	GtkWidget * fi_case;
	GtkWidget * fi_wrap;

	/* console */
	GtkWidget * co_window;
	GtkWidget * co_entry;
	GtkListStore * co_store;

	/* about */
	GtkWidget * ab_dialog;
};

#endif /* !SURFER_COMMON_H */
