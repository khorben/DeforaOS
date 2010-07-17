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



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <System.h>
#include "task.h"


/* Task */
/* private */
/* types */
struct _Task
{
	Config * config;

	/* internal */
	char * filename;
};


/* prototype */
static int _task_config_get_boolean(Task * task, char const * section,
		char const * variable);


/* public */
/* functions */
/* task_new */
Task * task_new(void)
{
	Task * task;

	if((task = malloc(sizeof(*task))) == NULL)
		return NULL;
	task->config = config_new();
	task->filename = NULL;
	if(task->config == NULL)
	{
		task_delete(task);
		return NULL;
	}
	task_set_start(task, time(NULL));
	return task;
}


/* task_new_from_file */
Task * task_new_from_file(char const * filename)
{
	Task * task;

	if((task = task_new()) == NULL)
		return NULL;
	if(task_set_filename(task, filename) != 0
			|| task_load(task) != 0)
	{
		task_delete(task);
		return NULL;
	}
	return task;
}


/* task_delete */
void task_delete(Task * task)
{
	free(task->filename);
	if(task->config != NULL)
		config_delete(task->config);
	free(task);
}


/* accessors */
/* task_get_done */
int task_get_done(Task * task)
{
	return _task_config_get_boolean(task, NULL, "done");
}


/* task_get_filename */
char const * task_get_filename(Task * task)
{
	return task->filename;
}


/* task_get_priority */
char const * task_get_priority(Task * task)
{
	return config_get(task->config, NULL, "priority");
}


/* task_get_start */
time_t task_get_start(Task * task)
{
	char const * start;

	if((start = config_get(task->config, NULL, "start")) == NULL)
		return 0;
	return atoi(start);
}


/* task_get_title */
char const * task_get_title(Task * task)
{
	return config_get(task->config, NULL, "title");
}


/* task_set_done */
int task_set_done(Task * task, int done)
{
	return config_set(task->config, NULL, "done", done ? "1" : "0");
}


/* task_set_filename */
int task_set_filename(Task * task, char const * filename)
{
	char * p;

	if((p = strdup(filename)) == NULL)
		return -1; /* XXX set error */
	free(task->filename);
	task->filename = p;
	return 0;
}


/* task_set_priority */
int task_set_priority(Task * task, char const * priority)
{
	return config_set(task->config, NULL, "priority", priority);
}


/* task_set_start */
int task_set_start(Task * task, time_t start)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%u", start);
	return config_set(task->config, NULL, "start", buf);
}


/* task_set_title */
int task_set_title(Task * task, char const * title)
{
	return config_set(task->config, NULL, "title", title);
}


/* useful */
/* task_load */
int task_load(Task * task)
{
	config_reset(task->config);
	return config_load(task->config, task->filename);
}


/* task_save */
int task_save(Task * task)
{
	if(task->filename == NULL)
		return -1; /* XXX set error */
	return config_save(task->config, task->filename);
}


/* task_unlink */
int task_unlink(Task * task)
{
	if(task->filename == NULL)
		return -1; /* XXX set error */
	return unlink(task->filename);
}


/* private */
/* functions */
/* task_config_get_boolean */
static int _task_config_get_boolean(Task * task, char const * section,
		char const * variable)
{
	int ret;
	char const * string;
	char * p;

	if((string = config_get(task->config, section, variable)) == NULL)
		return -1;
	ret = strtol(string, &p, 10);
	if(string[0] == '\0' || *p != '\0')
		return -1;
	return ret ? 1 : 0;
}
