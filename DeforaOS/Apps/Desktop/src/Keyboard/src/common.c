/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Keyboard */
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
#include "common.h"


/* public */
/* functions */
/* keysym_is_modifier */
int keysym_is_modifier(unsigned int keysym)
{
	if(keysym == XK_Alt_L || keysym == XK_Alt_R)
		return 1;
	if(keysym == XK_Control_L || keysym == XK_Control_R)
		return 1;
	if(keysym == XK_Num_Lock)
		return 1;
	if(keysym == XK_Shift_L || keysym == XK_Shift_R)
		return 1;
	return 0;
}
