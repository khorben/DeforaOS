/* $Id$ */
/* Copyright (c) 2007 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Servers inetd */
/* inetd is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * inetd is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * inetd; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA */



#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "parser.h"
#include "inetd.h"


/* variables */
InetdState * inetd_state;


/* inetd_error */
int inetd_error(char const * message, int ret)
{
	fputs("inetd: ", stderr);
	perror(message);
	return ret;
}


/* inetd */
static int _inetd_init(InetdState * state, char const * filename);
static int _inetd_do(InetdState * state);

static int _inetd(int debug, int queue, char const * filename)
{
	InetdState state;
	int ret;

	inetd_state = &state;
	state.debug = debug;
	state.queue = queue;
	state.filename = filename;
	if(_inetd_init(&state, filename))
		return 2;
	ret = _inetd_do(&state);
	config_delete(state.config);
	return ret ? 2 : 0;
}

static void _inetd_sighandler(int signum);
static int _inetd_daemonize(InetdState * state);
static int _inetd_setup(InetdState * state);
static int _inetd_init(InetdState * state, char const * filename)
{
	struct sigaction sa;

	sa.sa_handler = _inetd_sighandler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1
			|| sigaction(SIGHUP, &sa, NULL) == -1)
		return inetd_error("sigaction", 1);
	if((state->config = parser(filename)) == NULL
			|| _inetd_daemonize(state)
			|| _inetd_setup(state))
		return 1;
	return 0;
}

static void _inetd_sigchld(void);
static void _inetd_sighup(void);
static void _inetd_sighandler(int signum)
{
	switch(signum)
	{
		case SIGCHLD:
			_inetd_sigchld();
			break;
		case SIGHUP:
			_inetd_sighup();
			break;
	}
}

static void _inetd_sigchld(void)
{
	pid_t pid;
	int status;
	unsigned int i;

	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		inetd_error("waitpid", 0);
		return;
	}
	for(i = 0; i < inetd_state->config->services_nb; i++)
		if(inetd_state->config->services[i]->pid == pid)
		{
			inetd_state->config->services[i]->pid = -1;
			FD_SET(inetd_state->config->services[i]->fd,
					&inetd_state->rfds);
			break;
		}
	if(inetd_state->debug)
		fprintf(stderr, "%s%u%s%s%d%s", "inetd: Child ", (unsigned)pid,
				WIFEXITED(status) ? " exited"
				: " was terminated", " with error code ",
				WEXITSTATUS(status), "\n");
}

static void _inetd_sighup(void)
{
	Config * config;
	unsigned int i;
	unsigned int j;
	Service * sold;
	Service * snew;

	if((config = parser(inetd_state->filename)) == NULL)
	{
		if(inetd_state->debug)
			fprintf(stderr, "%s%s%s", "inetd: ",
					inetd_state->filename,
					"Ignoring reconfiguration request\n");
		return;
	}
	for(i = 0; i < inetd_state->config->services_nb; i++)
	{
		sold = inetd_state->config->services[i];
		for(j = 0; j < config->services_nb; j++)
		{
			snew = config->services[i];
			if(snew->socket != sold->socket
					|| snew->proto != sold->proto
					|| snew->wait != sold->wait
					|| snew->port != sold->port)
				continue;
			snew->fd = sold->fd;
			sold->fd = -1;
			if(inetd_state->debug)
				fprintf(stderr, "%s%s%s", "inetd: Service \"",
						sold->name, "\" kept\n");
			break;
		}
	}
	for(i = 0; i < inetd_state->config->services_nb; i++)
		if(inetd_state->config->services[i]->fd != -1)
			close(inetd_state->config->services[i]->fd);
	config_delete(inetd_state->config);
	inetd_state->config = config;
	_inetd_setup(inetd_state);
}

static int _inetd_daemonize(InetdState * state)
{
	pid_t pid;
	int fd;
	int i;

	if(state->debug)
	{
		fprintf(stderr, "%s", "inetd: Entered debugging mode\n");
		return 0;
	}
	if((pid = fork()) == -1)
		return inetd_error("fork", 1);
	if(pid != 0)
		exit(0);
	if((fd = open("/dev/null", O_RDWR, 0)) == -1)
		return inetd_error("/dev/null", 0);
	for(i = 0; i <= 2; i++)
	{
		close(i);
		dup2(fd, i);
	}
	return 0;
}

static int _inetd_setup(InetdState * state)
{
	unsigned int i;

	state->fdmax = -1;
	FD_ZERO(&state->rfds);
	for(i = 0; i < state->config->services_nb; i++)
	{
		if(state->config->services[i]->pid != -1)
			continue;
		if(state->config->services[i]->fd == -1
				&& service_listen(state->config->services[i]))
			continue;
		FD_SET(state->config->services[i]->fd, &state->rfds);
		if(state->fdmax < state->config->services[i]->fd)
			state->fdmax = state->config->services[i]->fd;
	}
	return 0;
}

static int _inetd_do(InetdState * state)
{
	fd_set rfdstmp;
	sigset_t sigset;
	unsigned int i;
	Service * s;
	int fd;

	sigfillset(&sigset);
	for(rfdstmp = state->rfds;; rfdstmp = state->rfds)
	{
		if(select(state->fdmax+1, &rfdstmp, NULL, NULL, NULL) == -1)
		{
			if(errno != EINTR)
				return inetd_error("select", 2);
			continue;
		}
		sigprocmask(SIG_SETMASK, &sigset, NULL);
		for(i = 0; i < state->config->services_nb; i++)
		{
			s = state->config->services[i];
			if(FD_ISSET(s->fd, &rfdstmp))
			{
				fd = s->fd;
				service_exec(s);
				if(s->pid != -1)
					FD_CLR(s->fd, &state->rfds);
			}
		}
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	}
	return 0;
}


/* usage */
static int _usage(void)
{
	fputs("Usage: inetd [-d][-q len] [config file]\n\
  -d	Debugging mode\n\
  -q	Queue length\n", stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	int debug = 0;
	int queue = 128;
	char const * filename = "/etc/inetd.conf";
	char * p;

	while((o = getopt(argc, argv, "dq")) != -1)
	{
		if(o == 'd')
			debug = 1;
		else if(o == 'q')
		{
			queue = strtol(optarg, &p, 10);
			if(*optarg == '\0' || *p != '\0')
				return _usage();
		}
		else
			return _usage();
	}
	if(argc - optind == 1)
		filename = argv[optind];
	else if(argc != optind)
		return _usage();
	return _inetd(debug, queue, filename) ? 2 : 0;
}
