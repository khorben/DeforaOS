/* service.c */



#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include "inetd.h"
#include "service.h"


int _service_port(char * name, char * proto);
Service * service_new(char * name, ServiceSocket socket, ServiceProtocol proto,
		ServiceWait wait, ServiceId id, char ** program)
{
	int port;
	Service * s;

	if((port = _service_port(name, proto == SP_TCP ? "tcp" : "udp")) == -1)
		return NULL; /* FIXME */
	if((s = malloc(sizeof(Service))) == NULL)
	{
		inetd_error("malloc", 0);
		return NULL;
	}
	s->name = strdup(name);
	s->socket = socket;
	s->proto = proto;
	s->wait = wait;
	s->id = id;
	s->program = program;
	s->fd = -1;
	s->port = port;
	s->pid = -1;
	return s;
}

int _service_port(char * name, char * proto)
{
	struct servent * se;
	char * p;
	int port;

	if((se = getservbyname(name, proto)) != NULL)
		return se->s_port;
	port = strtol(name, &p, 10);
	if(*name == '\0' || *p != '\0')
		return -1;
	return htons(port);
}


void service_delete(Service * s)
{
	char ** p;

	free(s->name);
	if(s->program != NULL)
	{
		for(p = s->program; *p != NULL; p++)
			free(*p);
		free(s->program);
	}
	free(s);
}


/* useful */
int service_listen(Service * s)
{
	struct sockaddr_in sa;

	if((s->fd = socket(AF_INET, s->socket == SS_STREAM ? SOCK_STREAM
					: SOCK_DGRAM, 0)) == -1)
		return inetd_error("socket", 1);
	sa.sin_family = AF_INET;
	sa.sin_port = s->port;
	sa.sin_addr.s_addr = INADDR_ANY;
	if(bind(s->fd, &sa, sizeof(sa)) != 0)
	{
		close(s->fd);
		s->fd = -1;
		return inetd_error("bind", 1);
	}
	if(s->socket == SS_STREAM && listen(s->fd, 5) != 0)
	{
		close(s->fd);
		s->fd = -1;
		return inetd_error("listen", 1);
	}
	if(inetd_state->debug)
		fprintf(stderr, "%s%s%s%d%s", "inetd: Service \"", s->name,
				"\" listening on port ", ntohs(s->port), "\n");
	return 0;
}


static int _exec_tcp(Service * s);
static int _exec_udp_nowait(Service * s);
static int _exec_udp_wait(Service * s);
int service_exec(Service * s)
{
	switch(s->proto)
	{
		case SP_TCP:
			return _exec_tcp(s);
		case SP_UDP:
			if(s->wait == SW_NOWAIT)
				return _exec_udp_nowait(s);
			return _exec_udp_wait(s);
		default:
			if(inetd_state->debug)
				fprintf(stderr, "%s",
						"inetd: Not implemented\n");
			return 1;
	}
}

static int _exec_tcp(Service * s)
{
	int fd;
	pid_t pid;
	struct sockaddr_in sa;
	int sa_size = sizeof(struct sockaddr_in);

	if((fd = accept(s->fd, &sa, &sa_size)) == -1)
		return inetd_error("accept", 1);
	if((pid = fork()) == -1)
		return inetd_error("fork", 1);
	else if(pid > 0)
		return close(fd);
	if(s->id.uid && setuid(s->id.uid))
		inetd_error("setuid", 0);
	if(s->id.gid && setgid(s->id.gid))
		inetd_error("setgid", 0);
	if(close(0) != 0 || close(1) != 0 || dup2(fd, 0) != 0
			|| dup2(fd, 1) != 1)
		inetd_error("dup2", 0);
	else
	{
		execv(s->program[0], &s->program[s->program[1] ? 1 : 0]);
		inetd_error(s->program[0], 0);
	}
	exit(2);
}

static int _exec_udp_nowait(Service * s)
{
	pid_t pid;

	if((pid = fork()) == -1)
		return inetd_error("fork", 1);
	else if(pid > 0)
		return 0;
	if(s->id.uid && setuid(s->id.uid))
		inetd_error("setuid", 0);
	if(s->id.gid && setgid(s->id.gid))
		inetd_error("setgid", 0);
	if(close(0) != 0 || close(1) != 0 || dup2(s->fd, 0) != 0
			|| dup2(2, 1) != 1)
		inetd_error("dup2", 0);
	execv(s->program[0], &s->program[s->program[1] ? 1 : 0]);
	/* FIXME
	 * - receive packet some time (normal and error cases) */
	inetd_error(s->program[0], 0);
	exit(2);
}

static int _exec_udp_wait(Service * s)
{
	if((s->pid = fork()) == -1)
		return inetd_error("fork", 1);
	else if(s->pid > 0)
		return 0;
	if(close(0) != 0 || close(1) != 0 || dup2(s->fd, 0) != 0
			|| dup2(2, 1) != 1)
		inetd_error("dup2", 0);
	else
	{
		execv(s->program[0], &s->program[s->program[1] ? 1 : 0]);
		inetd_error(s->program[0], 0);
	}
	exit(2);
}
