/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
/* Init is not free software; you can redistribute it and/or modify it
 * under the terms of the Creative Commons Attribution-NonCommercial-ShareAlike
 * 3.0 Unported as published by the Creative Commons organization.
 *
 * Init is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the Creative Commons Attribution-NonCommercial-
 * ShareAlike 3.0 Unported license for more details.
 *
 * You should have received a copy of the Creative Commons Attribution-
 * NonCommercial-ShareAlike 3.0 along with Init; if not, browse to
 * http://creativecommons.org/licenses/by-nc-sa/3.0/ */



#include <System.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "service.h"


/* Service */
/* protected */
struct _Service
{
	String * name;
	int verbose;

	/* configuration */
	/* FIXME add interface name... */
	int enabled;
	ServiceType type;
	char * command;

	/* system */
	/* FIXME add pid_t pid; ... */
	pid_t pid;
};


/* public */
/* functions */
/* service_new */
Service * service_new(char const * name)
{
	Service * service;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if((service = object_new(sizeof(*service))) == NULL)
		return NULL;
	service->name = string_new(name);
	service->verbose = 1;
	service->enabled = 0;
	service->type = ST_NULL;
	service->command = NULL;
	service->pid = -1;
	if(service->name == NULL)
	{
		service_delete(service);
		return NULL;
	}
	return service;
}


/* service_new_from_config */
Service * service_new_from_config(Config * config)
{
	Service * service;
	char const * p;

	if((p = config_get(config, "", "name")) == NULL)
		return NULL;
	if((service = service_new(p)) == NULL)
		return NULL;
	if((p = config_get(config, "", "enabled")) != NULL
			&& strcmp(p, "yes") == 0)
		service->enabled = 1;
	if((p = config_get(config, "", "type")) != NULL)
	{
		/* FIXME use a pre-defined table instead */
		if(strcmp(p, "command") == 0)
			service->type = ST_COMMAND;
		else if(strcmp(p, "daemon") == 0)
			service->type = ST_DAEMON;
		/* FIXME otherwise bail out */
	}
	/* FIXME else bail out */
	/* FIXME ST_COMMAND and ST_DAEMON must have it, otherwise ignore */
	if((p = config_get(config, "", "command")) != NULL)
		if((service->command = strdup(p)) == NULL)
		{
			error_set_code(1, "%s", strerror(errno));
			service_delete(service);
			return NULL;
		}
	return service;
}


/* service_delete */
void service_delete(Service * service)
{
	string_delete(service->name);
	object_delete(service);
}


/* accessors */
/* service_get_enabled */
int service_get_enabled(Service * service)
{
	return service->enabled;
}


/* service_get_name */
char const * service_get_name(Service * service)
{
	return service->name;
}


/* service_get_pid */
pid_t service_get_pid(Service * service)
{
	return service->pid;
}


/* service_set_command */
int service_set_command(Service * service, char const * command)
{
	free(service->command);
	if((service->command = strdup(command)) == NULL)
		return error_set_code(1, "%s", strerror(errno));
	return 0;
}


/* useful */
/* service_restart */
int service_restart(Service * service)
{
	int ret;

	ret = service_stop(service);
	ret |= service_start(service);
	return ret;
}


/* service_start */
static int _start_command(Service * service);
static int _start_daemon(Service * service);

int service_start(Service * service)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, service->name);
#endif
	switch(service->type)
	{
		case ST_NULL:
			return 0;
		case ST_COMMAND:
			return _start_command(service);
		case ST_DAEMON:
			return _start_daemon(service);
	}
	/* FIXME implement */
	return -error_set_code(1, "%s", strerror(ENOSYS));
}

static int _start_command(Service * service)
{
	pid_t pid;
	int status;

	if(service->verbose)
		fprintf(stderr, "%s: %s\n", "Starting command", service->name);
	if((pid = fork()) == -1)
		return error_set_code(1, "%s: %s", "fork", strerror(errno));
	if(pid == 0) /* child */
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(\"%s\")\n", "execl",
				service->command);
#endif
		execl("/bin/sh", "sh", "-c", service->command, NULL);
		exit(127);
	}
	/* FIXME this should feature a timeout and be handled like a daemon */
	if(waitpid(pid, &status, 0) != pid)
		return error_set_code(1, "%s", strerror(errno));
	if(WIFEXITED(status))
	{
		if(WEXITSTATUS(status) == 0)
			return 0;
		return error_set_code(1, "%s%d", "Child exited with code ",
				WEXITSTATUS(status));
	}
	if(WIFSIGNALED(status))
		/* FIXME factorize this and handle SEGV/BUS/ILL/FPE... */
		return error_set_code(1, "%s%d", "Child exited with signal ",
				WTERMSIG(status));
	if(WIFSTOPPED(status))
		return error_set_code(1, "%s", "Child stopped");
	return error_set_code(1, "%s", strerror(ENOSYS));
}

static int _start_daemon(Service * service)
{
	if(service->pid != -1)
		return error_set_code(1, "%s", "Already running");
	if(service->verbose)
		fprintf(stderr, "%s: %s\n", "Starting daemon", service->name);
	if((service->pid = fork()) == -1)
		return error_set_code(1, "%s: %s", "fork", strerror(errno));
	if(service->pid == 0) /* child */
	{
		/* FIXME factorize this code */
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(\"%s\")\n", "execl",
				service->command);
#endif
		execl("/bin/sh", "sh", "-c", service->command, NULL);
		exit(127);
	}
	/* we handle child termination, etc in the event loop */
	return 0;
}


/* service_stop */
static int _stop_daemon(Service * service);

int service_stop(Service * service)
{
	switch(service->type)
	{
		case ST_NULL:
			return 0;
		case ST_COMMAND:
			/* a command never runs in the background */
			return 0;
		case ST_DAEMON:
			return _stop_daemon(service);
	}
	/* FIXME implement */
	return error_set_code(1, "%s", strerror(ENOSYS));
}

static int _stop_daemon(Service * service)
{
	if(service->verbose)
		fprintf(stderr, "%s: %s\n", "Stopping daemon", service->name);
	/* FIXME implement:
	 * - send a "nice" signal
	 * - after a timeout send a less polite signal
	 * - if not successful alert the operator */
	return error_set_code(1, "%s", strerror(ENOSYS));
}
