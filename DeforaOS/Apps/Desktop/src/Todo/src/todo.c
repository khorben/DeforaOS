/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Todo */
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



#include <stdlib.h>
#include <gtk/gtk.h>
#include "todo.h"


/* Todo */
/* private */
/* types */
struct _Todo
{
	GtkWidget * window;
};


/* public */
/* functions */
/* todo_new */
Todo * todo_new(void)
{
	Todo * todo;

	if((todo = malloc(sizeof(*todo))) == NULL)
		return NULL;
	todo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_show_all(todo->window);
	return todo;
}


/* todo_delete */
void todo_delete(Todo * todo)
{
	free(todo);
}
