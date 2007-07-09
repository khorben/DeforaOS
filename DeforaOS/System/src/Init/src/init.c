/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
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
#include <signal.h>
#include <stdio.h>
#include <string.h>


/* Init */
static int _init_error(char * message, int ret);
static int _init_init(void);
static int _init(void)
{
	Event * event;
	AppServer * appserver;

	if(_init_init() != 0)
		return 1;
	if((event = event_new()) == NULL)
		return _init_error("Event", 1);
	if((appserver = appserver_new_event("Session", ASO_LOCAL, event))
			== NULL)
	{
		_init_error("AppServer", 1);
		event_delete(event);
		return 1;
	}
	event_loop(event);
	appserver_delete(appserver);
	event_delete(event);
	return 0;
}

static int _init_error(char * message, int ret)
	/* FIXME should use other means to report errors */
{
	fputs("Init: ", stderr);
	perror(message);
	return ret;
}

static void _init_sighandler(int signum);
static int _init_init(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _init_sighandler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1
			|| sigaction(SIGHUP, &sa, NULL) == -1)
		return _init_error("sigaction", 1);
	return 0;
}

static void _init_sighandler(int signum)
{
	switch(signum)
	{
		case SIGCHLD:
			if(waitpid(-1, NULL, WNOHANG) == -1)
				_init_error("waitpid", 0);
			break;
		case SIGHUP:
			/* FIXME reload configuration? */
			break;
	}
}


/* port */
int port(char * app)
{
	/* FIXME */
	if(strcmp(app, "Probe") == 0)
		return 4243;
	if(strcmp(app, "VFS") == 0)
		return 4245;
	return -1;
}


/* list */
int list(void)
{
	return 0;
}


/* start */
int start(char * app)
{
	return 0;
}


/* stop */
int stop(char * app)
{
	return 0;
}


/* usage */
static int _init_usage(void)
{
	fputs("Usage: Init\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _init_usage();
	return _init() == 0 ? 0 : 2;
}
