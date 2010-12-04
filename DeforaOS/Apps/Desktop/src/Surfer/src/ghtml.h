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



#ifndef SURFER_GHTML_H
# define SURFER_GHTML_H

# include <gtk/gtk.h>
# include "surfer.h"


/* GHTML */
/* functions */
GtkWidget * ghtml_new(Surfer * surfer);
void ghtml_delete(GtkWidget * ghtml);

/* accessors */
gboolean ghtml_can_go_back(GtkWidget * ghtml);
gboolean ghtml_can_go_forward(GtkWidget * ghtml);

char const * ghtml_get_link_message(GtkWidget * ghtml);
char const * ghtml_get_location(GtkWidget * ghtml);
gdouble ghtml_get_progress(GtkWidget * ghtml);
char const * ghtml_get_source(GtkWidget * ghtml);
char const * ghtml_get_status(GtkWidget * ghtml);
char const * ghtml_get_title(GtkWidget * ghtml);

int ghtml_set_proxy(GtkWidget * ghtml, char const * http);

/* useful */
gboolean ghtml_go_back(GtkWidget * ghtml);
gboolean ghtml_go_forward(GtkWidget * ghtml);

void ghtml_load_url(GtkWidget * ghtml, char const * url);

void ghtml_print(GtkWidget * ghtml);

void ghtml_refresh(GtkWidget * ghtml);
void ghtml_reload(GtkWidget * ghtml);
void ghtml_stop(GtkWidget * ghtml);

void ghtml_select_all(GtkWidget * ghtml);
void ghtml_unselect_all(GtkWidget * ghtml);

gboolean ghtml_find(GtkWidget * ghtml, char const * text, gboolean sensitive,
		gboolean wrap);

void ghtml_zoom_in(GtkWidget * ghtml);
void ghtml_zoom_out(GtkWidget * ghtml);
void ghtml_zoom_reset(GtkWidget * ghtml);

void ghtml_execute(GtkWidget * ghtml, char const * code);

#endif /* !SURFER_GHTML_H */
