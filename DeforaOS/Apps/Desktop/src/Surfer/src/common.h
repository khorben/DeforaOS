/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Surfer */
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
	SurferProxyType proxy_type;
	char * proxy_http;
	unsigned int proxy_http_port;

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
	GtkWidget * statusbox;
	GtkWidget * security;
	GtkWidget * progress;
	GtkWidget * statusbar;
	guint statusbar_id;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_homepage;
	GtkWidget * pr_focus_tabs;
	GtkWidget * pr_proxy_radio_direct;
	GtkWidget * pr_proxy_radio_http;
	GtkWidget * pr_proxy_http;
	GtkWidget * pr_proxy_http_port;

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
