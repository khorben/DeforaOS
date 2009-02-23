/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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



#ifndef SURFER_GHTML_H
# define SURFER_GHTML_H

# include <gtk/gtk.h>
# include "surfer.h"


/* GHTML */
/* functions */
GtkWidget * ghtml_new(Surfer * surfer);

/* accessors */
gboolean ghtml_can_go_back(GtkWidget * ghtml);
gboolean ghtml_can_go_forward(GtkWidget * ghtml);

char const * ghtml_get_link_message(GtkWidget * ghtml);
char const * ghtml_get_location(GtkWidget * ghtml);
char const * ghtml_get_title(GtkWidget * ghtml);

/* useful */
gboolean ghtml_go_back(GtkWidget * ghtml);
gboolean ghtml_go_forward(GtkWidget * ghtml);

void ghtml_load_url(GtkWidget * ghtml, char const * url);

void ghtml_refresh(GtkWidget * ghtml);
void ghtml_reload(GtkWidget * ghtml);
void ghtml_stop(GtkWidget * ghtml);

void ghtml_select_all(GtkWidget * ghtml);
void ghtml_unselect_all(GtkWidget * ghtml);

void ghtml_zoom_in(GtkWidget * ghtml);
void ghtml_zoom_out(GtkWidget * ghtml);
void ghtml_zoom_reset(GtkWidget * ghtml);

#endif /* !SURFER_GHTML_H */
