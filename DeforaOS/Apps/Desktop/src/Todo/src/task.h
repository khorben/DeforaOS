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



#ifndef TODO_TASK_H
# define TODO_TASK_H

# include <time.h>


/* Task */
/* types */
typedef struct _Task Task;


/* functions */
Task * task_new(void);
Task * task_new_from_file(char const * filename);
void task_delete(Task * task);


/* accessors */
char const * task_get_description(Task * task);
int task_get_done(Task * task);
time_t task_get_end(Task * task);
char const * task_get_filename(Task * task);
char const * task_get_priority(Task * task);
time_t task_get_start(Task * task);
char const * task_get_title(Task * task);

int task_set_description(Task * task, char const * description);
int task_set_done(Task * task, int done);
int task_set_end(Task * task, time_t end);
int task_set_filename(Task * task, char const * filename);
int task_set_priority(Task * task, char const * priority);
int task_set_start(Task * task, time_t start);
int task_set_title(Task * task, char const * title);


/* useful */
int task_load(Task * task);
int task_save(Task * task);
int task_unlink(Task * task);

#endif /* !TODO_TASK_H */
