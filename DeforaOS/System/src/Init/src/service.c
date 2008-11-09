/* $Id$ */
/* Copyright (c) 2008 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System Init */
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "service.h"


/* Service */
/* types */
typedef enum _ServiceType { ST_COMMAND = 0, ST_DAEMON } ServiceType;
#define ST_LAST ST_DAEMON
#define ST_COUNT (ST_LAST + 1)
static const char * _service_types[ST_COUNT] = { "command", "daemon" };

struct _Service
{
	String * name;

	/* FIXME add status: starting, running, stopping... */

	Config * config;
	int enabled;
	ServiceType type;

	/* command and daemon */
	pid_t pid;
	String * command;
};


/* private */
/* prototypes */
static String * _service_get_filename(Service * service);


/* public */
/* functions */
/* service_new */
Service * service_new(char const * name)
{
	Service * service;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, name);
#endif
	if(name == NULL)
		/* FIXME report error (EINVAL) */
		return NULL;
	/* FIXME really implement */
	if((service = object_new(sizeof(*service))) == NULL)
		return NULL;
	service->name = string_new(name);
	service->config = config_new();
	service->enabled = 0;
	service->pid = -1;
	service->command = NULL;
	/* error handling */
	if(service->name == NULL || service->config == NULL
			|| service_load(service) != 0)
	{
		service_delete(service);
		return NULL;
	}
	return service;
}


/* service_delete */
void service_delete(Service * service)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, service,
			service->name);
#endif
	/* FIXME really implement when service_new() is done */
	service_stop(service);
	string_delete(service->name);
	config_delete(service->config);
	string_delete(service->command);
	object_delete(service);
}


/* accessors */
/* service_get_name */
String const * service_get_name(Service * service)
{
	return service->name;
}


/* useful */
/* service_load */
static int _load_bool(Service * service, String const * name);
static int _load_enum(Service * service, String const * name, size_t count,
		const String ** values);
static int _load_command(Service * service);
static int _load_daemon(Service * service);

int service_load(Service * service)
	/* FIXME work on temporary data in case of failure */
{
	int ret;
	String * filename;

	config_reset(service->config);
	if((filename = _service_get_filename(service)) == NULL)
		return -1;
	ret = config_load(service->config, filename);
	string_delete(filename);
	if(ret != 0)
		return -1;
	service->enabled = _load_bool(service, "enabled");
	if((ret = _load_enum(service, "type", ST_COUNT, _service_types)) < 0)
		return -1;
	service->type = ret;
	switch(service->type)
	{
		case ST_COMMAND:
			return _load_command(service);
		case ST_DAEMON:
			return _load_daemon(service);
	}
	return 0;
}

static int _load_bool(Service * service, String const * name)
{
	String const * p;

	if((p = config_get(service->config, NULL, name)) == NULL)
		return 0;
	if(strcmp(p, "yes") == 0)
	{
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s(): %s=%s\n", __func__, name, "TRUE");
#endif
	}
		return 1;
	return 0;
}

static int _load_enum(Service * service, String const * name, size_t count,
		const String ** values)
{
	String const * p;
	size_t i;

	if((p = config_get(service->config, NULL, name)) == NULL)
		return -1;
	for(i = 0; i < count; i++)
		if(strcmp(p, values[i]) == 0)
		{
#ifdef DEBUG
			fprintf(stderr, "DEBUG: %s(): %s=%s\n", __func__, name,
					values[i]);
#endif
			return i;
		}
	return -error_set_code(1, "%s: %s \"%s\"", name, "Bad value", p);
}

static int _load_command(Service * service)
{
	String const * p;

	if((p = config_get(service->config, NULL, "command")) == NULL)
		return -1;
	string_delete(service->command);
	if((service->command = string_new(p)) == NULL)
		return -1;
	return 0;
}

static int _load_daemon(Service * service)
	/* FIXME code duplication with _load_command() */
{
	String const * p;

	if((p = config_get(service->config, NULL, "command")) == NULL)
		return -1;
	string_delete(service->command);
	if((service->command = string_new(p)) == NULL)
		return -1;
	return 0;
}


/* service_reload */
int service_reload(Service * service)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, service,
			service->name);
#endif
	/* FIXME implement */
	return -1;
}


/* service_start */
static int _start_command(Service * service);
static int _start_daemon(Service * service);
static int _start_exec(Service * service);

int service_start(Service * service)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, service,
			service->name);
#endif
	if(service->enabled != 1)
		return -error_set_code(1, "%s: %s", service->name,
				"Service is not enabled");
	switch(service->type)
	{
		case ST_COMMAND:
			return _start_command(service);
		case ST_DAEMON:
			return _start_daemon(service);
	}
	return -error_set_code(1, "%s", strerror(ENOSYS));;
}

static int _start_command(Service * service)
{
	if(_start_exec(service) != 0)
		return -1;
	/* FIXME monitor process */
	return 0;
}

static int _start_daemon(Service * service)
{
	if(_start_exec(service) != 0)
		return -1;
	/* FIXME monitor process */
	return 0;
}

static int _start_exec(Service * service)
{
	char * argv[] = { "/bin/sh", "-c", NULL, NULL };

	if(service->pid != -1)
		return -error_set_code(1, "%s: %s", service->name,
				"Already running");
	fprintf(stderr, "Starting %s:", service->name);
	if((service->pid = fork()) == -1)
	{
		error_set_code(1, "%s", strerror(errno));
		error_print(NULL);
	}
	if(service->pid != 0) /* father */
	{
		fputs(" done\n", stderr);
		return 0;
	}
	argv[2] = service->command;
	execve(argv[0], argv, NULL);
	error_set_print(PACKAGE, 1, "%s: %s", argv[0], strerror(errno));
	exit(127);
}


/* service_stop */
int service_stop(Service * service)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, service,
			service->name);
#endif
	/* FIXME implement */
	return -1;
}


/* private */
/* functions */
/* service_get_filename */
/* FIXME code duplication with _session_get_filename() */
static String * _service_get_filename(Service * service)
{
	char const * format = "%s/%s.%s";
	String * p;
	int size;

	if(service->name == NULL)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	/* FIXME implement it as string_new_format() */
	if((size = snprintf(NULL, 0, format, SERVICEDIR, service->name,
					SERVICEEXT)) <= 0)
	{
		error_set_code(1, "%s", strerror(EINVAL));
		return NULL;
	}
	if((p = object_new(++size)) == NULL)
		return NULL;
	snprintf(p, size, format, SERVICEDIR, service->name, SERVICEEXT);
	return p;
}
