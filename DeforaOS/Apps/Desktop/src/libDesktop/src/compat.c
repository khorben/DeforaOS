/* $Id$ */
/* Copyright (c) 2012 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop libDesktop */
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



#include <gdk/gdkx.h>
#include "Desktop.h"


/* Compat */
#if GTK_CHECK_VERSION(3, 0, 0)
/* gdk_window_clear */
void gdk_window_clear(GdkWindow * window)
{
	Display * display;
	Window wid;

	display = gdk_x11_get_default_xdisplay();
	wid = gdk_x11_window_get_xid(window);
	XClearWindow(display, wid);
}
#endif
