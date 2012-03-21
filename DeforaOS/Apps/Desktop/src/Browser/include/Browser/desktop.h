/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Browser */
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



#ifndef DESKTOP_BROWSER_DESKTOP_H
# define DESKTOP_BROWSER_DESKTOP_H

# include <gtk/gtk.h>
# include <Desktop.h>


/* Desktop */
/* public */
/* types */
typedef enum _DesktopAlignment
{
	DESKTOP_ALIGNMENT_VERTICAL = 0,
	DESKTOP_ALIGNMENT_HORIZONTAL
} DesktopAlignment;

typedef enum _DesktopIcons
{
	DESKTOP_ICONS_NONE = 0,
	DESKTOP_ICONS_APPLICATIONS,
	DESKTOP_ICONS_CATEGORIES,
	DESKTOP_ICONS_FILES,
	DESKTOP_ICONS_HOMESCREEN
} DesktopIcons;
# define DESKTOP_ICONS_LAST DESKTOP_ICONS_HOMESCREEN
# define DESKTOP_ICONS_COUNT (DESKTOP_ICONS_LAST + 1)

typedef enum _DesktopLayout
{
	DESKTOP_LAYOUT_NORMAL = 0,
	DESKTOP_LAYOUT_LANDSCAPE,
	DESKTOP_LAYOUT_PORTRAIT,
	DESKTOP_LAYOUT_ROTATE,
	DESKTOP_LAYOUT_TOGGLE
} DesktopLayout;

typedef enum _DesktopMessage
{
	DESKTOP_MESSAGE_SET_ALIGNMENT = 0,
	DESKTOP_MESSAGE_SET_ICONS,
	DESKTOP_MESSAGE_SET_LAYOUT,
	DESKTOP_MESSAGE_SHOW
} DesktopMessage;

typedef enum _DesktopShow
{
	DESKTOP_SHOW_SETTINGS = 0
} DesktopMessageShow;


/* constants */
# define DESKTOP_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_DESKTOP_CLIENT"

#endif /* !DESKTOP_BROWSER_DESKTOP_H */
