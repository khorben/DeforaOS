/* $Id$ */
/* Copyright (c) 2009-2012 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_PANEL_H
# define DESKTOP_PANEL_H

# include <gtk/gtk.h>


/* PanelApplet */
/* types */
typedef struct _Panel Panel;

typedef struct _PanelApplet PanelApplet;

typedef struct _PanelAppletHelper
{
	Panel * panel;
	GtkIconSize icon_size;
	char const * (*config_get)(Panel * panel, char const * section,
			char const * variable);
	int (*config_set)(Panel * panel, char const * section,
			char const * variable, char const * value);
	int (*error)(Panel * panel, char const * message, int ret);
	void (*about_dialog)(Panel * panel);
	int (*lock)(Panel * panel);
	void (*logout_dialog)(Panel * panel);
	void (*position_menu)(Panel * panel, GtkMenu * menu, gint * x, gint * y,
			gboolean * push_in);
	void (*preferences_dialog)(Panel * panel);
	void (*rotate_screen)(Panel * panel);
	void (*shutdown_dialog)(Panel * panel);
	int (*suspend)(Panel * panel);
} PanelAppletHelper;

typedef struct _PanelAppletDefinition
{
	char const * name;
	char const * icon;
	char const * description;
	PanelApplet * (*init)(PanelAppletHelper * helper, GtkWidget ** widget);
	void (*destroy)(PanelApplet * applet);
	GtkWidget * (*settings)(PanelApplet * applet, gboolean apply,
			gboolean reset);
	gboolean expand;
	gboolean fill;
} PanelAppletDefinition;

typedef enum _PanelMessage
{
	PANEL_MESSAGE_SHOW = 0
} PanelMessage;

typedef enum _PanelMessageShow
{
	PANEL_MESSAGE_SHOW_PANEL_BOTTOM = 0x1,
	PANEL_MESSAGE_SHOW_PANEL_TOP = 0x2,
	PANEL_MESSAGE_SHOW_SETTINGS = 0x4
} PanelMessageShow;


/* constants */
# define PANEL_CLIENT_MESSAGE		"DEFORAOS_DESKTOP_PANEL_CLIENT"

#endif /* !DESKTOP_PANEL_H */
