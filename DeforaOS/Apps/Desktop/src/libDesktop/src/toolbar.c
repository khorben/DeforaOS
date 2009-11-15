/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
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


/* Toolbar */
GtkWidget * desktop_toolbar_create(DesktopToolbar * toolbar, gpointer data,
		GtkAccelGroup * accel)
{
	GtkWidget * ret;
	size_t i;
	DesktopToolbar * p;

	ret = gtk_toolbar_new();
	for(i = 0; toolbar[i].name != NULL; i++)
	{
		p = &toolbar[i];
		if(p->name[0] == '\0')
		{
			p->widget = gtk_separator_tool_item_new();
			gtk_toolbar_insert(GTK_TOOLBAR(ret), p->widget, -1);
			continue;
		}
		p->widget = gtk_tool_button_new_from_stock(p->name);
		g_signal_connect_swapped(G_OBJECT(p->widget), "clicked",
				G_CALLBACK(p->callback), data);
		if(accel != NULL && p->accel != 0)
			gtk_widget_add_accelerator(GTK_WIDGET(p->widget),
					"clicked", accel, p->accel,
					GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
		gtk_toolbar_insert(GTK_TOOLBAR(ret), p->widget, -1);
	}
	return ret;
}
