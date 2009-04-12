/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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



#ifndef SURFER_SURFER_H
# define SURFER_SURFER_H

# include <System.h>
# include <gtk/gtk.h>


/* Surfer */
/* constants */
# define SURFER_CONFIG_FILE			".surferrc"

# define SURFER_DEFAULT_MINIMUM_FONT_SIZE	8.0
# define SURFER_DEFAULT_FONT_SIZE		12.0
# define SURFER_DEFAULT_FIXED_FONT_SIZE		12.0
# define SURFER_DEFAULT_ENCODING		"ISO-8859-1"
# define SURFER_DEFAULT_HOME			"http://www.defora.org/"
# define SURFER_DEFAULT_SERIF_FONT		"Serif"
# define SURFER_DEFAULT_SANS_FONT		"Sans"
# define SURFER_DEFAULT_STANDARD_FONT		SURFER_DEFAULT_SANS_FONT
# define SURFER_DEFAULT_FIXED_FONT		"Monospace"
# define SURFER_DEFAULT_FANTASY_FONT		"Comic Sans MS"
# define SURFER_DEFAULT_TITLE			"Web surfer"

# define SURFER_GTKMOZEMBED_COMPPATH		"/usr/pkg/lib/firefox"


/* types */
typedef struct _Surfer
{
	Config * config;
	char * url;

	/* preferences */
	char * homepage;

	/* widgets */
	/* main window */
	GtkWidget * window;
#ifndef FOR_EMBEDDED
	GtkWidget * menubar;
#endif
	GtkWidget * toolbar;
	GtkToolItem * tb_back;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_refresh;
	GtkWidget * tb_path;
	GtkWidget * view;
	GtkWidget * progress;
	GtkWidget * statusbox;
	GtkWidget * statusbar;
	guint statusbar_id;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_homepage;
} Surfer;


/* variables */
extern unsigned int surfer_cnt;


/* functions */
Surfer * surfer_new(char const * url);
Surfer * surfer_new_copy(Surfer * surfer);
void surfer_delete(Surfer * surfer);


/* accessors */
void surfer_set_fullscreen(Surfer * surfer, gboolean fullscreen);
void surfer_set_location(Surfer * surfer, char const * url);
void surfer_set_progress(Surfer * surfer, gdouble fraction);
void surfer_set_status(Surfer * surfer, char const * status);
void surfer_set_title(Surfer * surfer, char const * title);


/* useful */
int surfer_config_load(Surfer * surfer);
int surfer_config_save(Surfer * surfer);

int surfer_confirm(Surfer * surfer, char const * message);
int surfer_error(Surfer * surfer, char const * message, int ret);
void surfer_warning(Surfer * surfer, char const * message);

void surfer_open(Surfer * surfer, char const * url);
void surfer_open_dialog(Surfer * surfer);

gboolean surfer_go_back(Surfer * surfer);
gboolean surfer_go_forward(Surfer * surfer);
void surfer_go_home(Surfer * surfer);

void surfer_refresh(Surfer * surfer);
void surfer_reload(Surfer * surfer);
void surfer_stop(Surfer * surfer);

void surfer_select_all(Surfer * surfer);
void surfer_unselect_all(Surfer * surfer);

void surfer_zoom_in(Surfer * surfer);
void surfer_zoom_out(Surfer * surfer);
void surfer_zoom_reset(Surfer * surfer);

#endif /* !SURFER_SURFER_H */
