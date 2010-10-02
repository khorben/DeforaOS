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



#include <string.h>
#include <libintl.h>
#include "Desktop.h"
#define _(string) gettext(string)


/* Toolbar */
/* desktop_toolbar_create */
GtkWidget * desktop_toolbar_create(DesktopToolbar * toolbar, gpointer data,
		GtkAccelGroup * accel)
{
	GtkWidget * ret;
	size_t i;
	DesktopToolbar * p;
	GtkWidget * widget;

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
		else if(strncmp(p->stock, "gtk-", 4) == 0) /* stock icon */
			p->widget = gtk_tool_button_new_from_stock(p->stock);
		else if(p->stock != NULL) /* icon name */
		{
			widget = gtk_image_new_from_icon_name(p->stock,
					GTK_ICON_SIZE_LARGE_TOOLBAR);
			p->widget = gtk_tool_button_new(widget, NULL);
		}
		else
			p->widget = gtk_tool_button_new(NULL, _(p->name));
		if(p->callback != NULL)
			g_signal_connect_swapped(G_OBJECT(p->widget), "clicked",
					G_CALLBACK(p->callback), data);
		else
			gtk_widget_set_sensitive(p->widget, FALSE);
		if(accel != NULL && p->accel != 0)
			gtk_widget_add_accelerator(GTK_WIDGET(p->widget),
					"clicked", accel, p->accel, p->modifier,
					GTK_ACCEL_VISIBLE);
		gtk_toolbar_insert(GTK_TOOLBAR(ret), p->widget, -1);
	}
	return ret;
}
