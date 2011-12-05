/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include "Desktop.h"


/* Accel */
/* desktop_accel_create */
void desktop_accel_create(DesktopAccel const * accel, gpointer data,
		GtkAccelGroup * group)
{
	size_t i;
	GClosure * cc;

	if(group == NULL)
		return;
	for(i = 0; accel[i].callback != NULL; i++)
	{
		cc = g_cclosure_new_swap(accel[i].callback, data, NULL);
		gtk_accel_group_connect(group, accel[i].accel,
				accel[i].modifier, GTK_ACCEL_VISIBLE, cc);
	}
}
