/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#ifndef TODO_TASKEDIT_H
# define TODO_TASKEDIT_H

# include "task.h"
# include "todo.h"


/* TaskEdit */
/* types */
typedef struct _TaskEdit TaskEdit;


/* functions */
TaskEdit * taskedit_new(Todo * todo, Task * task);
void taskedit_delete(TaskEdit * taskedit);

#endif /* !TODO_TASKEDIT_H */
