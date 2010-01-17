/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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


/* Panel */
/* types */
typedef struct _Panel Panel;

typedef struct _PanelPrefs
{
	int monitor;
} PanelPrefs;


/* constants */
#define PANEL_BORDER_WIDTH	4
#define PANEL_ICON_SIZE		GTK_ICON_SIZE_LARGE_TOOLBAR


/* functions */
Panel * panel_new(PanelPrefs * prefs);
void panel_delete(Panel * panel);

/* useful */
int panel_error(Panel * panel, char const * message, int ret);
int panel_load(Panel * panel, char const * applet);

# endif /* !PANEL_COMMON_H */
