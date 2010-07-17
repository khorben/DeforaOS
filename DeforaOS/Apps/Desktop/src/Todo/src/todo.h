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



#ifndef TODO_TODO_H
# define TODO_TODO_H

# include "task.h"
# include <gtk/gtk.h>


/* Todo */
/* types */
typedef struct _Todo Todo;

typedef enum _TodoPriority
{
	TODO_PRIORITY_UNKNOWN,
	TODO_PRIORITY_LOW,
	TODO_PRIORITY_MEDIUM,
	TODO_PRIORITY_HIGH,
	TODO_PRIORITY_URGENT
} TodoPriority;


/* functions */
Todo * todo_new(void);
void todo_delete(Todo * todo);

/* useful */
void todo_about(Todo * todo);
int todo_error(Todo * todo, char const * message, int ret);

/* tasks */
Task * todo_task_add(Todo * todo, Task * task);
void todo_task_delete_selected(Todo * todo);
void todo_task_remove_all(Todo * todo);

/* accessors */
void todo_task_set_priority(Todo * todo, GtkTreePath * path,
		char const * priority);
void todo_task_set_title(Todo * todo, GtkTreePath * path, char const * title);

void todo_task_edit(Todo * todo);
int todo_task_reload_all(Todo * todo);
void todo_task_save_all(Todo * todo);
void todo_task_select_all(Todo * todo);
void todo_task_toggle_done(Todo * todo, GtkTreePath * path);

#endif /* !TODO_TODO_H */
