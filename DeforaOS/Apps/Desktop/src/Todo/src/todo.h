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


/* Todo */
/* types */
typedef struct _Todo Todo;


/* functions */
Todo * todo_new(void);
void todo_delete(Todo * todo);

/* useful */
void todo_delete_selection(Todo * todo);
void todo_select_all(Todo * todo);

#endif /* !TODO_TODO_H */
