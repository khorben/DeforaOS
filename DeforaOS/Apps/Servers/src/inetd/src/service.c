/* service.c */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "inetd.h"
#include "service.h"


Service * service_new(char * name, ServiceSocket socket, ServiceProtocol proto,
		ServiceWait wait, ServiceId id, char ** program)
{
	Service * s;

	if((s = malloc(sizeof(Service))) == NULL)
		return NULL;
	s->name = strdup(name);
	s->socket = socket;
	s->proto = proto;
	s->wait = wait;
	s->id = id;
	s->program = program;
	return s;
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
		return 1;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(s->port);
	sa.sin_addr.s_addr = INADDR_ANY;
	if(bind(s->fd, &sa, sizeof(sa)) != 0)
		return 1;
	if(listen(s->fd, 5) != 0)
		return 1;
	return 0;
}

void service_exec(Service * s, int fd)
{
	if(close(0) != 0 || close(1) != 0 || dup2(fd, 0) != 0
			|| dup2(fd, 1) != 1)
	{
		inetd_error("dup2", 0);
		return;
	}
	execv(s->name, s->program);
	exit(inetd_error(s->name, 2));
}
