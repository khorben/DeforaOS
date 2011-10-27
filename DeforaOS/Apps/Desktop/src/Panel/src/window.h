/* $Id$ */
/* Copyright (c) 2011 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Panel */
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



#ifndef PANEL_WINDOW_H
# define PANEL_WINDOW_H

# include "common.h"


/* PanelWindow */
/* types */
typedef struct _PanelWindow PanelWindow;


/* functions */
PanelWindow * panel_window_new(PanelPosition position,
		PanelAppletHelper * helper, GdkRectangle * root);
void panel_window_delete(PanelWindow * panel);

/* accessors */
int panel_window_get_height(PanelWindow * panel);

/* useful */
void panel_window_append(PanelWindow * panel, GtkWidget * widget,
		gboolean expand, gboolean fill);
void panel_window_reset(PanelWindow * panel, PanelPosition position,
		GdkRectangle * root);

#endif /* !PANEL_WINDOW_H */
