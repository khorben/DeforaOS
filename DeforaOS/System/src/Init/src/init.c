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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "service.h"
#include "../config.h"


/* Init */
/* private */
/* types */
typedef struct _Init
{
	Event * event;
	AppServer * appserver;
	/* FIXME add a hash table? */
	Service ** services;
	/* FIXME determine how to handle the Session service (per user?) */
	/* User ** users; */
} Init;


/* variables */
static Init init; /* XXX rename? */


/* prototypes */
static int _init(void);

/* services */
static Service ** _init_services_get(void);

static void _init_sighandler(int signum);
static int _usage(void);


/* public */
/* prototypes */
/* Session AppInterface */
int list(void);
int port(char const * app);
int start(char const * app);
int stop(char const * app);


/* private */
/* functions */
/* init */
static int _init_init(Init * init);
static void _init_destroy(Init * init);
static int _init_timeout(void * data);

static int _init(void)
{
	Service ** s;

	if(_init_init(&init) != 0)
		return error_print(PACKAGE);
	for(s = init.services; *s != NULL; s++)
		if(service_get_enabled(*s) == 1)
			service_start(*s);
	/* FIXME add a condition for reboot/shutdown */
	while(event_loop(init.event) != 0)
	{
		error_print(PACKAGE);
		sleep(1); /* spare resources if it keeps failing */
	}
	for(s = init.services; *s != NULL; s++)
		service_stop(*s);
	_init_destroy(&init);
	return 0;
}

static int _init_init(Init * init)
{
	struct timeval tv = { 1, 0 };
	struct sigaction sa;
	int signals[] = { SIGCHLD, SIGHUP, SIGINT, SIGTERM, SIGUSR1, SIGUSR2 };
	size_t i;

	init->appserver = NULL;
	if((init->event = event_new()) == NULL)
		return 1;
	if((init->appserver = appserver_new_event("Session", ASO_LOCAL,
					init->event)) == NULL
			|| (event_register_timeout(init->event, &tv,
					_init_timeout, init) != 0))
	{
		_init_destroy(init);
		return 1;
	}
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _init_sighandler;
	sigfillset(&sa.sa_mask);
	for(i = 0; i < (sizeof(signals) / sizeof(*signals)); i++)
		if(sigaction(signals[i], &sa, NULL) == -1)
			error_set_print(PACKAGE, 0, "%d: %s", signals[i],
					strerror(errno));
	if((init->services = _init_services_get()) == NULL)
		return 1;
	return 0;
}

static void _init_destroy(Init * init)
{
	if(init->appserver != NULL)
		appserver_delete(init->appserver);
	if(init->event != NULL)
		event_delete(init->event);
	/* FIXME free the rest, too */
}

static int _init_timeout(void * data)
{
	Init * init = data;
	pid_t pid;
	int status;
	Service ** s;

	/* FIXME loop until all pending childs are processed instead */
	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		if(errno != ECHILD)
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
		return 0;
	}
	/* FIXME lookup which daemon(/command?) this was, analyze, react */
	for(s = init->services; *s != NULL; s++)
		if(service_get_pid(*s) == pid)
			break;
	if(s == NULL)
		return error_set_print(PACKAGE, 0, "%d: %s", pid,
				"Unknown child exited");
	return 0;
}


/* init_services_get */
static int _services_get_load(Config * config, char const * pathname);

static Service ** _init_services_get(void)
{
	Service ** services = NULL;
	static char const rep[] = PREFIX "/etc/Services";
	static char const ext[] = ".service";
	size_t services_cnt = 0;
	size_t len;
	Service ** s;
	char * pathname = NULL;
	char * p;
	DIR * dir;
	struct dirent * de;
	Config * config;

	if((config = config_new()) == NULL)
		return NULL;
	/* FIXME give up this loop and instantiate a default service, that will
	 * load the necessary others by itself (requires/provides...) */
	if((dir = opendir(rep)) == NULL)
	{
		error_set_code(1, "%s: %s", rep, strerror(errno));
		config_delete(config);
		return NULL;
	}
	while((de = readdir(dir)) != NULL)
	{
		if((len = strlen(de->d_name) + 1) <= sizeof(ext)
				|| strcmp(&de->d_name[len - sizeof(ext)], ext)
				!= 0)
			continue;
		if((p = realloc(pathname, sizeof(rep) + strlen(de->d_name) + 2))
				== NULL)
		{
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
			continue;
		}
		pathname = p;
		sprintf(pathname, "%s/%s", rep, de->d_name);
		if(_services_get_load(config, pathname) != 0)
		{
			error_print(PACKAGE);
			continue;
		}
		if((s = realloc(services, sizeof(*services)
						* (services_cnt + 2))) == NULL)
		{
			error_set_print(PACKAGE, 1, "%s", strerror(errno));
			continue;
		}
		services = s;
		if((s[services_cnt] = service_new_from_config(config)) == NULL)
		{
			error_print(PACKAGE);
			continue;
		}
		services[++services_cnt] = NULL;
	}
	free(pathname);
	closedir(dir);
	config_delete(config);
	return services;
}

static int _services_get_load(Config * config, char const * pathname)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, \"%s\")\n", __func__, config, pathname);
#endif
	config_reset(config);
	return config_load(config, pathname);
}


/* init_sighandler */
static void _init_sighandler(int signum)
{
	switch(signum)
	{
		case SIGCHLD:
			if(waitpid(-1, NULL, WNOHANG) == -1)
				error_set_print(PACKAGE, 1, "%s", strerror(
							errno));
			break;
		case SIGHUP:
			/* FIXME reload configuration? */
			break;
		case SIGINT: /* ignore these */
		case SIGTERM:
		case SIGUSR1:
		case SIGUSR2:
			break;
	}
}


/* usage */
static int _usage(void)
{
	fputs("Usage: Init\n", stderr);
	return 1;
}


/* public */
/* Session AppInterface */
/* list */
int list(void)
{
	/* FIXME implement properly */
	Service ** s;

	for(s = init.services; *s != NULL; s++)
		printf("Service: %s\n", service_get_name(*s));
	return 0;
}


/* port */
int port(char const * app)
{
	/* FIXME lookup the service and query/return port if applicable */
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%s)\n", __func__, app);
#endif
	if(strcmp(app, "Init") == 0)
		return 0;
	if(strcmp(app, "Probe") == 0)
		return 4243;
	if(strcmp(app, "VFS") == 0)
		return 4245;
	if(strcmp(app, "Directory") == 0)
		return 4247;
	return -1;
}


/* start */
int start(char const * app)
{
	Service ** s;

	/* FIXME check permissions */
	for(s = init.services; *s != NULL; s++)
		if(strcmp(service_get_name(*s), app) == 0)
			break;
	if(*s == NULL)
		return 1;
	return service_start(*s);
}


/* stop */
int stop(char const * app)
{
	Service ** s;

	/* FIXME check permissions */
	for(s = init.services; *s != NULL; s++)
		if(strcmp(service_get_name(*s), app) == 0)
			break;
	if(*s == NULL)
		return 1;
	return service_stop(*s);
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return _init() == 0 ? 0 : 2;
}
