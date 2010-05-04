/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Unix sh */
/* sh is not free software; you can redistribute it and/or modify it under the
 * terms of the Creative Commons Attribution-NonCommercial-ShareAlike 3.0
 * Unported as published by the Creative Commons organization.
 *
 * sh is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with sh; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "sh.h"
#include "job.h"


/* types */
typedef struct _Job
{
	char * command;
	pid_t pid;
	JobStatus status;
	int error;
} Job;


/* variables */
static Job * jobs = NULL;
static unsigned int jobs_cnt = 0;


/* job_add */
static int _add_wait(unsigned int id);
static void _add_wait_all(void);

int job_add(char * command, pid_t pid, JobStatus status)
{
	int ret = 0;
	Job * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, jobs_cnt + 1);
#endif
	if((command = strdup(command)) == NULL)
		return sh_error("malloc", 1);
	if((p = realloc(jobs, sizeof(*p) * (jobs_cnt + 1))) == NULL)
	{
		free(command);
		return sh_error("malloc", 1);
	}
	jobs = p;
	p = &jobs[jobs_cnt++];
	p->command = command;
	p->pid = pid;
	p->status = status;
	/* FIXME depending on further C-z handling we could do this earlier */
	if(status == JS_WAIT)
		ret = _add_wait(jobs_cnt);
	_add_wait_all();
	return ret;
}

static int _job_remove(unsigned int id);
static int _add_wait(unsigned int id)
{
	int status;

	for(;;)
		if(waitpid(jobs[id - 1].pid, &status, 0) == -1)
			return sh_error("waitpid", -1);
		else if(WIFEXITED(status) || WIFSIGNALED(status))
			break;
	_job_remove(id);
	return WEXITSTATUS(status);
}

static void _job_print(unsigned int id, char c, char * state);
static void _add_wait_all(void)
{
	pid_t pid = 0;
	int status;
	unsigned int i;

	while(jobs_cnt > 0 && (pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		for(i = 0; i < jobs_cnt && jobs[i].pid != pid; i++);
		if(i == jobs_cnt)
			continue;
		_job_print(i + 1, 'X', "FIXME");
		_job_remove(i + 1);
	}
	if(pid == -1)
		sh_error("waitpid", 0);
}


/* job_kill_status */
int job_kill_status(int signum, JobStatus status)
{
	int ret = 0;
	unsigned int i;

	for(i = 0; i < jobs_cnt; i++)
		if(jobs[i].status == status)
			ret |= kill(jobs[i].pid, signum);
	return ret == 0 ? 0 : 1;
}


/* job_remove */
static int _job_remove(unsigned int id)
{
	Job * p;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%u)\n", __func__, id);
#endif
	if(id > jobs_cnt)
		return 1;
	if(id > 1)
	{
		free(jobs[id - 1].command);
		memmove(&jobs[id - 1], &jobs[id], (jobs_cnt - id) * sizeof(*p));
		if((p = realloc(jobs, sizeof(*p) * --jobs_cnt)) == NULL
				&& jobs_cnt != 0)
			return sh_error("malloc", 1);
		jobs = p;
	}
	else /* FIXME check this code */
		jobs_cnt--;
	return 0;
}


/* job_list */
int job_list(int argc, char * argv[])
{
	int i;
	unsigned int j;
	unsigned int id;
	char * p;

	if(argc == 0)
	{
		for(j = 0; j < jobs_cnt; j++)
			_job_print(j+1, 'X', "FIXME");
		return 0;
	}
	for(i = 1; i < argc; i++)
	{
		id = strtol(argv[i], &p, 10);
		if(*(argv[i]) == '\0' || *p != '\0' || id < 1 || id >= jobs_cnt)
			continue;
		_job_print(id+1, 'X', "FIXME");
	}
	return 0;
}


/* job_pgids */
int job_pgids(int argc, char * argv[])
{
	/* FIXME implement */
	return 1;
}


/* job_print */
static void _job_print(unsigned int id, char c, char * state)
{
	printf("[%u] %c %s %s\n", id, c, state, jobs[id-1].command);
}


/* job_status */
int job_status(int argc, char * argv[])
{
	/* FIXME ? */
	job_list(1, NULL);
	return 1;
}
