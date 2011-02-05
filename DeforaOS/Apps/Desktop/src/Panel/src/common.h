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



#ifndef PANEL_COMMON_H
# define PANEL_COMMON_H

# include <gtk/gtk.h>
# include "Panel.h"


/* Panel */
/* types */
typedef enum _PanelPosition
{
	PANEL_POSITION_BOTTOM	= 0x1,
	PANEL_POSITION_TOP	= 0x2
} PanelPosition;
# define PANEL_POSITION_BOTH	(PANEL_POSITION_BOTTOM | PANEL_POSITION_TOP)
# define PANEL_POSITION_DEFAULT PANEL_POSITION_BOTTOM

typedef struct _PanelPrefs
{
	GtkIconSize iconsize;
	int monitor;
	PanelPosition position;
} PanelPrefs;


/* constants */
#define PANEL_BORDER_WIDTH	4
#define PANEL_ICON_SIZE_DEFAULT	GTK_ICON_SIZE_LARGE_TOOLBAR
#define PANEL_ICON_SIZE_UNSET	GTK_ICON_SIZE_INVALID
#define PANEL_ICON_SIZE_SMALL	GTK_ICON_SIZE_SMALL_TOOLBAR
#define PANEL_ICON_SIZE_SMALLER	GTK_ICON_SIZE_MENU
#define PANEL_ICON_SIZE_LARGE	GTK_ICON_SIZE_LARGE_TOOLBAR


/* functions */
Panel * panel_new(PanelPrefs const * prefs);
void panel_delete(Panel * panel);

/* useful */
int panel_error(Panel * panel, char const * message, int ret);
int panel_load(Panel * panel, PanelPosition position, char const * applet);

void panel_show_preferences(Panel * panel, gboolean show);

#endif /* !PANEL_COMMON_H */
